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

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const EquityMethod& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  static_cast<void>(method);
  static_cast<void>(asset_snapshot);

  if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
    return std::numeric_limits<double>::quiet_NaN();
  } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                     BacktestMethodContext>) {
    return context.equity();
  } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                     AnySeriesMethodContext>) {
    const auto* backtest_context =
     series_method_context_cast<BacktestMethodContext>(context);
    return backtest_context ? backtest_context->equity()
                            : std::numeric_limits<double>::quiet_NaN();
  } else {
    return std::numeric_limits<double>::quiet_NaN();
  }
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const EquityPercentMethod& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  static_cast<void>(method);
  static_cast<void>(asset_snapshot);

  if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
    return std::numeric_limits<double>::quiet_NaN();
  } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                     BacktestMethodContext>) {
    return context.equity_percent();
  } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                     AnySeriesMethodContext>) {
    const auto* backtest_context =
     series_method_context_cast<BacktestMethodContext>(context);
    return backtest_context ? backtest_context->equity_percent()
                            : std::numeric_limits<double>::quiet_NaN();
  } else {
    return std::numeric_limits<double>::quiet_NaN();
  }
}

} // namespace pludux::backtest
