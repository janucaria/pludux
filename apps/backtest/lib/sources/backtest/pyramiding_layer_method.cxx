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
                       BacktestMethodContext context) noexcept -> double
{
  return static_cast<double>(context.pyramiding_layer());
}

} // namespace pludux::backtest
