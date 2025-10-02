module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

export module pludux:series.bb_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.sma_method;
import :series.ema_method;
import :series.wma_method;
import :series.rma_method;
import :series.hma_method;

import :series.ohlcv_method;

export namespace pludux {

enum class BbMaType { Sma, Ema, Wma, Rma, Hma };

template<typename TMaSourceMethod = CloseMethod>
  requires requires { typename TMaSourceMethod::ResultType; }
class BbMethod {
public:
  using ResultType = typename TMaSourceMethod::ResultType;

  BbMethod()
  : BbMethod{20, 1.5}
  {
  }

  explicit BbMethod(std::size_t period, double stddev)
  : BbMethod{BbMaType::Sma, TMaSourceMethod{}, period, stddev}
  {
  }

  explicit BbMethod(BbMaType ma_type,
                    TMaSourceMethod ma,
                    std::size_t period,
                    double stddev)
  : ma_type_{ma_type}
  , ma_source_{std::move(ma)}
  , period_{period}
  , stddev_{stddev}
  {
  }

  auto operator==(const BbMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return self(asset_snapshot, SeriesOutput::MiddleBand, context);
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    const auto middle = [&]() -> ResultType {
      switch(self.ma_type_) {
      case BbMaType::Sma: {
        const auto ma = SmaMethod{self.ma_source_, self.period_};
        return ma(asset_snapshot, context);
      }
      case BbMaType::Ema: {
        const auto ma = EmaMethod{self.ma_source_, self.period_};
        return ma(asset_snapshot, context);
      }
      case BbMaType::Wma: {
        const auto ma = WmaMethod{self.ma_source_, self.period_};
        return ma(asset_snapshot, context);
      }
      case BbMaType::Rma: {
        const auto ma = RmaMethod{self.ma_source_, self.period_};
        return ma(asset_snapshot, context);
      }
      case BbMaType::Hma: {
        const auto ma = HmaMethod{self.ma_source_, self.period_};
        return ma(asset_snapshot, context);
      }
      default:
        return std::numeric_limits<ResultType>::quiet_NaN();
      }
    }();

    auto sum_squared_diff = ResultType{0};
    for(auto i = 0uz; i < self.period_; ++i) {
      const auto diff = self.ma_source_(asset_snapshot[i], context) - middle;
      sum_squared_diff += diff * diff;
    }

    const auto std_dev = std::sqrt(sum_squared_diff / self.period_);
    const auto std_dev_scaled = std_dev * self.stddev_;

    const auto upper = middle + std_dev_scaled;
    const auto lower = middle - std_dev_scaled;

    switch(output) {
    case SeriesOutput::MiddleBand:
      return middle;
    case SeriesOutput::UpperBand:
      return upper;
    case SeriesOutput::LowerBand:
      return lower;
    default:
      return std::numeric_limits<ResultType>::quiet_NaN();
    }
  }

  auto ma_type(this const auto& self) noexcept -> BbMaType
  {
    return self.ma_type_;
  }

  auto ma_type(this auto& self, BbMaType ma_type) noexcept
  {
    self.ma_type_ = ma_type;
  }

  auto ma_source(this const auto& self) noexcept -> const TMaSourceMethod&
  {
    return self.ma_source_;
  }

  void ma_source(this auto& self, TMaSourceMethod ma_source) noexcept
  {
    self.ma_source_ = std::move(ma_source);
  }

  auto stddev(this const auto& self) noexcept -> double
  {
    return self.stddev_;
  }

  void stddev(this auto& self, double new_stddev) noexcept
  {
    self.stddev_ = new_stddev;
  }

  auto period(this const auto& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this auto& self, std::size_t new_period) noexcept
  {
    self.period_ = new_period;
  }

private:
  BbMaType ma_type_;
  TMaSourceMethod ma_source_;
  std::size_t period_{20};
  double stddev_;
};

} // namespace pludux
