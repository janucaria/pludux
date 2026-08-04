module;

#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

export module pludux.backtest:position_method;

import pludux;

import :backtest_method_context;

export namespace pludux::backtest {

class InitialEntryPriceMethod {
public:
  auto operator==(const InitialEntryPriceMethod&) const noexcept
   -> bool = default;
};

class LatestEntryPriceMethod {
public:
  auto operator==(const LatestEntryPriceMethod&) const noexcept
   -> bool = default;
};

class AveragePriceMethod {
public:
  auto operator==(const AveragePriceMethod&) const noexcept -> bool = default;
};

class StopTargetRefPriceMethod {
public:
  auto operator==(const StopTargetRefPriceMethod&) const noexcept
   -> bool = default;
};

class PositionDirectionMethod {
public:
  auto operator==(const PositionDirectionMethod&) const noexcept
   -> bool = default;
};

template<typename TSourceMethod = CloseMethod>
class PositionRMultipleMethod {
public:
  PositionRMultipleMethod()
  : PositionRMultipleMethod{TSourceMethod{}}
  {
  }

  explicit PositionRMultipleMethod(TSourceMethod source)
  : source_{std::move(source)}
  {
  }

  auto operator==(const PositionRMultipleMethod&) const noexcept
   -> bool = default;

  auto source(this const PositionRMultipleMethod& self) noexcept
   -> const TSourceMethod&
  {
    return self.source_;
  }

private:
  TSourceMethod source_;
};

auto hash_series_method(const InitialEntryPriceMethod&) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}(
   "pludux.backtest.InitialEntryPriceMethod");
}

auto hash_series_method(const LatestEntryPriceMethod&) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}(
   "pludux.backtest.LatestEntryPriceMethod");
}

auto hash_series_method(const AveragePriceMethod&) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.AveragePriceMethod");
}

auto hash_series_method(const StopTargetRefPriceMethod&) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}(
   "pludux.backtest.StopTargetRefPriceMethod");
}

auto hash_series_method(const PositionDirectionMethod&) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}(
   "pludux.backtest.PositionDirectionMethod");
}

template<typename TSourceMethod>
auto hash_series_method(
 const PositionRMultipleMethod<TSourceMethod>& method) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}(
          "pludux.backtest.PositionRMultipleMethod") ^
         hash_series_method(method.source());
}

namespace detail {

template<typename TValueFn>
auto position_context_value(MethodContextable auto context,
                            TValueFn value_fn) noexcept -> double
{
  if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
    return std::numeric_limits<double>::quiet_NaN();
  } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                     BacktestMethodContext>) {
    return value_fn(context);
  } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                     ErasedSeriesMethodContext>) {
    const auto* backtest_context =
     series_method_context_cast<BacktestMethodContext>(context);
    return backtest_context ? value_fn(*backtest_context)
                            : std::numeric_limits<double>::quiet_NaN();
  } else {
    return std::numeric_limits<double>::quiet_NaN();
  }
}

} // namespace detail

auto backtest_position_initial_entry_price(
 MethodContextable auto context) noexcept -> double
{
  return detail::position_context_value(context, [](const auto& value) {
    return value.position_initial_entry_price();
  });
}

auto backtest_position_latest_entry_price(
 MethodContextable auto context) noexcept -> double
{
  return detail::position_context_value(context, [](const auto& value) {
    return value.position_latest_entry_price();
  });
}

auto backtest_position_average_price(MethodContextable auto context) noexcept
 -> double
{
  return detail::position_context_value(
   context, [](const auto& value) { return value.position_average_price(); });
}

auto backtest_position_reference_price(MethodContextable auto context) noexcept
 -> double
{
  return detail::position_context_value(
   context, [](const auto& value) { return value.position_reference_price(); });
}

auto backtest_position_direction(MethodContextable auto context) noexcept
 -> double
{
  return detail::position_context_value(
   context, [](const auto& value) { return value.position_direction(); });
}

auto backtest_position_risk_distance(MethodContextable auto context) noexcept
 -> double
{
  return detail::position_context_value(
   context, [](const auto& value) { return value.position_risk_distance(); });
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const InitialEntryPriceMethod&,
                       AssetSnapshot,
                       MethodContextable auto context) noexcept -> double
{
  return backtest_position_initial_entry_price(context);
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const LatestEntryPriceMethod&,
                       AssetSnapshot,
                       MethodContextable auto context) noexcept -> double
{
  return backtest_position_latest_entry_price(context);
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const AveragePriceMethod&,
                       AssetSnapshot,
                       MethodContextable auto context) noexcept -> double
{
  return backtest_position_average_price(context);
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const StopTargetRefPriceMethod&,
                       AssetSnapshot,
                       MethodContextable auto context) noexcept -> double
{
  return backtest_position_reference_price(context);
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const PositionDirectionMethod&,
                       AssetSnapshot,
                       MethodContextable auto context) noexcept -> double
{
  return backtest_position_direction(context);
}

template<typename TSourceMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const PositionRMultipleMethod<TSourceMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto source_price =
   evaluate_series_method(method.source(), asset_snapshot, context);
  const auto reference_price = backtest_position_reference_price(context);
  const auto direction = backtest_position_direction(context);
  const auto risk_distance = backtest_position_risk_distance(context);
  if(!std::isfinite(source_price) || !std::isfinite(reference_price) ||
     !std::isfinite(direction) || direction == 0.0 ||
     !std::isfinite(risk_distance) || risk_distance <= 0.0) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return direction * (source_price - reference_price) / risk_distance;
}

} // namespace pludux::backtest
