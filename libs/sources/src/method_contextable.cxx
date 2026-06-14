module;

#include <concepts>
#include <cstddef>
#include <string>
#include <variant>
#include <vector>

export module pludux:method_contextable;

import :asset_snapshot;
import :method_key;

import :methods.select_output_method;

export namespace pludux {

template<typename TContext>
concept MethodContextable =
 std::same_as<TContext, std::monostate> ||
 requires(TContext context,
          const std::string& name,
          AssetSnapshot asset_snapshot,
          MethodOutput output,
          std::size_t index) {
   {
     context.call_series_method(name, asset_snapshot)
   } -> std::convertible_to<double>;

   {
     context.call_series_method(name, asset_snapshot, output)
   } -> std::convertible_to<double>;

   { context.get_series_result(name, index) } -> std::convertible_to<double>;

   {
     context.get_series_results(std::declval<MethodKey>())
   } -> std::convertible_to<std::vector<double>&>;

   { context.get_input_value(name) } -> std::convertible_to<double>;

   { context.index() } -> std::convertible_to<std::size_t>;
 };

} // namespace pludux