module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.adaptive_ma_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;
import :series.sma_method;
import :series.cached_results_ema_method;
import :series.wma_method;
import :series.cached_results_rma_method;
import :series.hma_method;

export namespace pludux {

enum class MaMethodType { Sma, Ema, Wma, Rma, Hma };

template<typename TSourceMethod = CloseMethod>
class AdaptiveMaMethod {
public:
  using ResultType = typename TSourceMethod::ResultType;

  AdaptiveMaMethod()
  : AdaptiveMaMethod{20}
  {
  }

  explicit AdaptiveMaMethod(std::size_t period)
  : AdaptiveMaMethod{TSourceMethod{}, period}
  {
  }

  AdaptiveMaMethod(TSourceMethod source, std::size_t period)
  : AdaptiveMaMethod{MaMethodType::Sma, std::move(source), period}
  {
  }

  AdaptiveMaMethod(MaMethodType ma_type,
                   TSourceMethod source,
                   std::size_t period)
  : ma_type_{ma_type}
  , source_{std::move(source)}
  , period_{period}
  , ema_method_{source_, period_}
  , rma_method_{source_, period_}
  {
  }

  auto operator==(const AdaptiveMaMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const AdaptiveMaMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    switch(self.ma_type_) {
    case MaMethodType::Sma: {
      const auto sma = SmaMethod{self.source_, self.period_};
      return sma(asset_snapshot, context);
    }
    case MaMethodType::Ema: {
      return self.ema_method_(asset_snapshot, context);
    }
    case MaMethodType::Wma: {
      const auto wma = WmaMethod{self.source_, self.period_};
      return wma(asset_snapshot, context);
    }
    case MaMethodType::Rma: {
      return self.rma_method_(asset_snapshot, context);
    }
    case MaMethodType::Hma: {
      const auto hma = HmaMethod{self.source_, self.period_};
      return hma(asset_snapshot, context);
    }
    default:
      return std::numeric_limits<ResultType>::quiet_NaN();
    }
  }

  auto operator()(this const AdaptiveMaMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto ma_type(this const AdaptiveMaMethod& self) noexcept -> MaMethodType
  {
    return self.ma_type_;
  }

  void ma_type(this AdaptiveMaMethod& self, MaMethodType ma_type) noexcept
  {
    self.ma_type_ = ma_type;
  }

  auto source(this const AdaptiveMaMethod& self) noexcept
   -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this AdaptiveMaMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
    self.reset_mas_methods();
  }

  auto period(this const AdaptiveMaMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this AdaptiveMaMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
    self.reset_mas_methods();
  }

private:
  MaMethodType ma_type_;
  TSourceMethod source_;
  std::size_t period_;

  CachedResultsEmaMethod<TSourceMethod> ema_method_;
  CachedResultsRmaMethod<TSourceMethod> rma_method_;

  void reset_mas_methods(this AdaptiveMaMethod& self) noexcept
  {
    self.ema_method_ =
     CachedResultsEmaMethod<TSourceMethod>{self.source_, self.period_};
    self.rma_method_ =
     CachedResultsRmaMethod<TSourceMethod>{self.source_, self.period_};
  }
};

} // namespace pludux
