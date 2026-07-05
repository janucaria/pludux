module;

#include <cstddef>
#include <functional>
#include <limits>
#include <string_view>
#include <type_traits>
#include <variant>

export module pludux.backtest:drawdown_method;

import pludux;

import :backtest_method_context;

export namespace pludux::backtest {

class DrawdownMethod {
public:
  DrawdownMethod() = default;

  auto operator==(const DrawdownMethod& other) const noexcept -> bool = default;
};

auto hash_series_method(const DrawdownMethod&) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.DrawdownMethod");
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const DrawdownMethod& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  static_cast<void>(method);
  static_cast<void>(asset_snapshot);

  if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
    return std::numeric_limits<double>::quiet_NaN();
  } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                     BacktestMethodContext>) {
    return context.previous_drawdown();
  } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                     AnySeriesMethodContext>) {
    const auto* backtest_context =
     series_method_context_cast<BacktestMethodContext>(context);
    return backtest_context ? backtest_context->previous_drawdown()
                            : std::numeric_limits<double>::quiet_NaN();
  } else {
    return std::numeric_limits<double>::quiet_NaN();
  }
}

} // namespace pludux::backtest
