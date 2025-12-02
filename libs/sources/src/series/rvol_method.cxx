module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:series.rvol_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;
import :series.sma_method;
import :series.operators_method;

export namespace pludux {

class RvolMethod {
public:
  using ResultType =
   std::common_type_t<typename VolumeMethod::ResultType,
                      typename SmaMethod<VolumeMethod>::ResultType>;

  explicit RvolMethod(std::size_t period = 14)
  : period_{period}
  {
  }

  auto operator==(const RvolMethod& other) const noexcept -> bool = default;

  auto operator()(this const RvolMethod self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    const auto volume = VolumeMethod{};
    const auto sma_volume = SmaMethod{VolumeMethod{}, self.period_};
    const auto rvol = DivideMethod{volume, sma_volume};
    const auto rvol_result = rvol(asset_snapshot, context);
    return rvol_result;
  }

  auto operator()(this RvolMethod self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto period(this const RvolMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this RvolMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  std::size_t period_;
};

} // namespace pludux
