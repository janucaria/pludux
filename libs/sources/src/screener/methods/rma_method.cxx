module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

export module pludux:screener.rma_method;

import :asset_snapshot;
import :screener.method_call_context;
import :screener.method_output;

import :screener.sma_method;
import :screener.ohlcv_method;

export namespace pludux::screener {

template<typename TSourceMethod = CloseMethod>
class RmaMethod {
public:
  using ResultType = typename TSourceMethod::ResultType;

  RmaMethod()
  : RmaMethod{20}
  {
  }

  explicit RmaMethod(std::size_t period)
  : RmaMethod{TSourceMethod{}, period}
  {
  }

  explicit RmaMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const RmaMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    const auto asset_size = asset_snapshot.size();
    const auto alpha = 1.0 / self.period_;
    const auto sma_index = asset_size - self.period_;
    auto result = std::numeric_limits<ResultType>::quiet_NaN();
    for(auto ii = asset_size; ii > 0; --ii) {
      const auto i = ii - 1;
      if(i > sma_index) {
        continue;
      }

      if(std::isnan(result)) {
        result =
         SmaMethod{self.source_, self.period_}(asset_snapshot[i], context);
        continue;
      }

      const auto source_value = self.source_(asset_snapshot[i], context);
      result = source_value * alpha + result * (1 - alpha);
    }

    return result;
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodOutput output,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto source(this const auto& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this auto& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const auto& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this auto& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux::screener
