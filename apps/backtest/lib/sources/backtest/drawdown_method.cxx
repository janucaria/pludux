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

template<typename TContext>
  requires requires(TContext context) { context.drawdown(); }
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const DrawdownMethod&,
                       AssetSnapshot,
                       TContext context) noexcept -> double
{
  return context.drawdown();
}

} // namespace pludux::backtest
