module;

#include <concepts>
#include <string>
#include <variant>

export module pludux:series.method_contextable;

import :asset_snapshot;
import :series.method_output;

export namespace pludux::series {

template<typename TContext>
concept MethodContextable =
 std::same_as<TContext, std::monostate> ||
 requires(TContext context,
          const std::string& name,
          AssetSnapshot asset_snapshot,
          MethodOutput output) {
   {
     context.dispatch_call(name, asset_snapshot)
   } -> std::convertible_to<double>;
   {
     context.dispatch_call(name, asset_snapshot, output)
   } -> std::convertible_to<double>;
 };

} // namespace pludux::series