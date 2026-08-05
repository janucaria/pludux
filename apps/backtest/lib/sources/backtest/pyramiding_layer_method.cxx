module;

#include <cstddef>
#include <functional>
#include <limits>
#include <string_view>
#include <type_traits>
#include <variant>

export module pludux.backtest:pyramiding_layer_method;

import pludux;

import :backtest_method_context;

export namespace pludux::backtest {

class PyramidingLayerMethod {
public:
  auto operator==(const PyramidingLayerMethod&) const noexcept
   -> bool = default;
};

auto hash_series_method(const PyramidingLayerMethod&) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.PyramidingLayerMethod");
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const PyramidingLayerMethod&,
                       AssetSnapshot,
                       MethodContextable auto context) noexcept -> double
{
  if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
    return std::numeric_limits<double>::quiet_NaN();
  } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                     BacktestMethodContext>) {
    return static_cast<double>(context.pyramiding_layer());
  } else if constexpr(std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                     ErasedSeriesMethodContext>) {
    const auto* backtest_context =
     series_method_context_cast<BacktestMethodContext>(context);
    return backtest_context
            ? static_cast<double>(backtest_context->pyramiding_layer())
            : std::numeric_limits<double>::quiet_NaN();
  } else {
    return std::numeric_limits<double>::quiet_NaN();
  }
}

} // namespace pludux::backtest
