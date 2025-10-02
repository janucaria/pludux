module;

#include <concepts>
#include <string>
#include <variant>

export module pludux:method_contextable;

import :asset_snapshot;
import :series_output;

export namespace pludux {

template<typename TContext>
concept MethodContextable =
 std::same_as<TContext, std::monostate> ||
 requires(TContext context,
          const std::string& name,
          AssetSnapshot asset_snapshot,
          SeriesOutput output) {
   {
     context.call_series_method(name, asset_snapshot)
   } -> std::convertible_to<double>;
   {
     context.call_series_method(name, asset_snapshot, output)
   } -> std::convertible_to<double>;
 };

} // namespace pludux