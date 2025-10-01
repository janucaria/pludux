module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

export module pludux:screener.rsi_method;

import :asset_snapshot;
import :screener.method_call_context;
import :screener.method_output;

import :screener.change_method;
import :screener.rma_method;
import :screener.operators_method;
import :screener.ohlcv_method;
import :screener.value_method;

export namespace pludux::screener {

template<typename TSourceMethod = CloseMethod>
class RsiMethod {
public:
  using ResultType = typename TSourceMethod::ResultType;

  RsiMethod()
  : RsiMethod{14}
  {
  }

  explicit RsiMethod(std::size_t period)
  : RsiMethod{TSourceMethod{}, period}
  {
  }

  explicit RsiMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const RsiMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    const auto changes = ChangeMethod{self.source_};

    const auto gains = PositivePartMethod{changes};
    const auto losses = NegativePartMethod{changes};

    const auto avg_gain = RmaMethod{gains, self.period_};
    const auto avg_loss = RmaMethod{losses, self.period_};

    const auto rs_method = DivideMethod{avg_gain, avg_loss};
    const auto rs = rs_method(asset_snapshot, context);
    const auto rsi = 100 - (100 / (1 + rs));

    return rsi;
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

RsiMethod() -> RsiMethod<CloseMethod>;
RsiMethod(std::size_t) -> RsiMethod<CloseMethod>;

} // namespace pludux::screener
