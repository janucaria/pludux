module;

#include <cstddef>
#include <functional>
#include <limits>
#include <string_view>
#include <type_traits>
#include <variant>

export module pludux.backtest:equity_method;

import pludux;

import :backtest_method_context;

export namespace pludux::backtest {

class EquityMethod {
public:
  EquityMethod() = default;

  auto operator==(const EquityMethod& other) const noexcept -> bool = default;
};

class EquityPercentMethod {
public:
  EquityPercentMethod() = default;

  auto operator==(const EquityPercentMethod& other) const noexcept
   -> bool = default;
};

auto hash_series_method(const EquityMethod&) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.EquityMethod");
}

auto hash_series_method(const EquityPercentMethod&) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.EquityPercentMethod");
}

template<typename TContext>
  requires requires(TContext context) { context.equity(); }
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const EquityMethod&,
                       AssetSnapshot,
                       TContext context) noexcept -> double
{
  return context.equity();
}

template<typename TContext>
  requires requires(TContext context) { context.equity_percent(); }
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const EquityPercentMethod&,
                       AssetSnapshot,
                       TContext context) noexcept -> double
{
  return context.equity_percent();
}

} // namespace pludux::backtest
