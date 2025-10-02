module;

#include <concepts>
#include <string>
#include <variant>

export module pludux:series.method_call_context;

import :asset_snapshot;
import :series.method_output;

export namespace pludux::series {

template<typename TContext, typename TResultType>
concept MethodCallContext =
 std::same_as<TContext, std::monostate> ||
 requires(TContext context,
          const std::string& name,
          AssetSnapshot asset_snapshot,
          MethodOutput output) {
   {
     context.dispatch_call(name, asset_snapshot)
   } -> std::convertible_to<TResultType>;
   {
     context.dispatch_call(name, asset_snapshot, output)
   } -> std::convertible_to<TResultType>;
 };

} // namespace pludux::series