module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:series.atr_method;

import :asset_snapshot;
import :series.method_call_context;
import :series.method_output;

import :series.tr_method;
import :series.rma_method;

export namespace pludux::series {

class AtrMethod {
public:
  using ResultType = typename RmaMethod<TrMethod>::ResultType;

  AtrMethod()
  : AtrMethod{14}
  {
  }

  explicit AtrMethod(std::size_t period)
  : AtrMethod{period, 1.0}
  {
  }

  explicit AtrMethod(std::size_t period, double multiplier)
  : period_{period}
  , multiplier_{multiplier}
  {
  }

  auto operator==(const AtrMethod& other) const noexcept -> bool = default;

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    const auto tr_method = TrMethod{};
    const auto rma_method = RmaMethod{tr_method, self.period_};
    const auto atr = rma_method(asset_snapshot, context) * self.multiplier_;

    return atr;
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

  void period(this auto& self, std::size_t new_period) noexcept
  {
    self.period_ = new_period;
  }

  auto multiplier(this auto self) noexcept -> double
  {
    return self.multiplier_;
  }

  void multiplier(this auto& self, double new_multiplier) noexcept
  {
    self.multiplier_ = new_multiplier;
  }

private:
  std::size_t period_;
  double multiplier_;
};

} // namespace pludux::series
