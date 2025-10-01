module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:screener.rvol_method;

import :asset_snapshot;
import :screener.method_call_context;
import :screener.method_output;

import :screener.ohlcv_method;
import :screener.sma_method;
import :screener.operators_method;

export namespace pludux::screener {

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

  auto operator()(this const auto self,
                  AssetSnapshot asset_snapshot,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    const auto volume = VolumeMethod{};
    const auto sma_volume = SmaMethod{VolumeMethod{}, self.period_};
    const auto rvol = DivideMethod{volume, sma_volume};
    const auto rvol_result = rvol(asset_snapshot, context);
    return rvol_result;
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodOutput output,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto period(this auto self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this auto& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  std::size_t period_;
};

} // namespace pludux::screener
