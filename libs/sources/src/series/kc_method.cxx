module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

export module pludux:series.kc_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;
import :series.operators_method;
import :series.tr_method;
import :series.atr_method;
import :series.adaptive_ma_method;

export namespace pludux {

enum class KcBandMethodType { Atr, Tr, RangeHighLow };

template<typename TMaSourceMethod = CloseMethod>
  requires requires { typename TMaSourceMethod::ResultType; }
class KcMethod {
public:
  using ResultType = typename TMaSourceMethod::ResultType;

  KcMethod()
  : KcMethod{MaMethodType::Ema, 10, KcBandMethodType::Atr, 14, 1.5}
  {
  }

  KcMethod(MaMethodType ma_method_type,
           std::size_t ma_period,
           KcBandMethodType band_method_type,
           std::size_t band_atr_period,
           double multiplier)
  : KcMethod{TMaSourceMethod{},
             ma_method_type,
             ma_period,
             band_method_type,
             band_atr_period,
             multiplier}
  {
  }

  KcMethod(TMaSourceMethod ma_source,
           MaMethodType ma_method_type,
           std::size_t ma_period,
           KcBandMethodType band_method_type,
           std::size_t band_atr_period,
           double multiplier)
  : band_atr_period_{band_atr_period}
  , multiplier_{multiplier}
  , ma_method_{ma_method_type, std::move(ma_source), ma_period}
  , band_method_type_{band_method_type}
  {
  }

  auto operator==(const KcMethod& other) const noexcept -> bool = default;

  auto operator()(this const KcMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return self(asset_snapshot, SeriesOutput::MiddleBand, context);
  }

  auto operator()(this const KcMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    const auto band_range = [&]() -> double {
      switch(self.band_method_type_) {
      case KcBandMethodType::Atr: {
        const auto atr_method = AtrMethod{self.band_atr_period_};
        return atr_method(asset_snapshot, context);
      }
      case KcBandMethodType::Tr: {
        const auto tr_method = TrMethod{};
        return tr_method(asset_snapshot, context);
      }
      case KcBandMethodType::RangeHighLow: {
        const auto range_method = SubtractMethod{HighMethod{}, LowMethod{}};
        return range_method(asset_snapshot, context);
      }
      default:
        return std::numeric_limits<double>::quiet_NaN();
      }
    };

    const auto middle = self.ma_method_(asset_snapshot, context);

    switch(output) {
    case SeriesOutput::MiddleBand:
      return middle;
    case SeriesOutput::UpperBand:
      return middle + (self.multiplier_ * band_range());
    case SeriesOutput::LowerBand:
      return middle - (self.multiplier_ * band_range());
    default:
      return std::numeric_limits<double>::quiet_NaN();
    }
  }

  auto multiplier(this const KcMethod& self) noexcept -> double
  {
    return self.multiplier_;
  }

  void multiplier(this KcMethod& self, double multiplier) noexcept
  {
    self.multiplier_ = multiplier;
  }

  auto ma_period(this const KcMethod& self) noexcept -> std::size_t
  {
    return self.ma_method_.period();
  }

  void ma_period(this KcMethod& self, std::size_t ma_period) noexcept
  {
    self.ma_method_.period(ma_period);
  }

  auto ma_method_type(this const KcMethod& self) noexcept -> MaMethodType
  {
    return self.ma_method_.ma_type();
  }

  void ma_method_type(this KcMethod& self, MaMethodType ma_method_type) noexcept
  {
    self.ma_method_.ma_type(ma_method_type);
  }

  auto ma_source(this const KcMethod& self) noexcept -> const TMaSourceMethod&
  {
    return self.ma_method_.source();
  }

  void ma_source(this KcMethod& self, TMaSourceMethod ma_source) noexcept
  {
    self.ma_method_.source(std::move(ma_source));
  }

  auto band_method_type(this const KcMethod& self) noexcept -> KcBandMethodType
  {
    return self.band_method_type_;
  }

  void band_method_type(this KcMethod& self,
                        KcBandMethodType band_method_type) noexcept
  {
    self.band_method_type_ = band_method_type;
  }

  auto band_atr_period(this const KcMethod& self) noexcept -> std::size_t
  {
    return self.band_atr_period_;
  }

  void band_atr_period(this KcMethod& self,
                       std::size_t band_atr_period) noexcept
  {
    self.band_atr_period_ = band_atr_period;
  }

private:
  std::size_t band_atr_period_;
  double multiplier_;

  AdaptiveMaMethod<TMaSourceMethod> ma_method_;
  KcBandMethodType band_method_type_;
};

KcMethod() -> KcMethod<CloseMethod>;

} // namespace pludux