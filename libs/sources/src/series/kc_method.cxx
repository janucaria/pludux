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
import :series.ema_method;
import :series.atr_method;

export namespace pludux {

template<typename TMaMethod = EmaMethod<>, typename TRangeMethod = AtrMethod>
class KcMethod {
public:
  using ResultType = std::common_type_t<typename TRangeMethod::ResultType,
                                        typename TMaMethod::ResultType>;

  KcMethod()
  : KcMethod{TMaMethod{}, TRangeMethod{14}, 1.5}
  {
  }

  KcMethod(TMaMethod ma, TRangeMethod range, double multiplier)
  : multiplier_{multiplier}
  , ma_{std::move(ma)}
  , range_{std::move(range)}
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
    const auto range = self.range_(asset_snapshot, context) * self.multiplier_;
    const auto middle = self.ma_(asset_snapshot, context);

    switch(output) {
    case SeriesOutput::MiddleBand:
      return middle;
    case SeriesOutput::UpperBand:
      return middle + range;
    case SeriesOutput::LowerBand:
      return middle - range;
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

  auto ma(this const KcMethod& self) noexcept -> const TMaMethod&
  {
    return self.ma_;
  }

  void ma(this KcMethod& self, TMaMethod ma) noexcept
  {
    self.ma_ = std::move(ma);
  }

  auto range(this const KcMethod& self) noexcept -> const TRangeMethod&
  {
    return self.range_;
  }

  void range(this KcMethod& self, TRangeMethod range) noexcept
  {
    self.range_ = std::move(range);
  }

private:
  double multiplier_;
  TMaMethod ma_;
  TRangeMethod range_;
};

KcMethod() -> KcMethod<EmaMethod<>, AtrMethod>;

} // namespace pludux