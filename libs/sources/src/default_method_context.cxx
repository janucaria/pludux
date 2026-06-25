module;

#include <cstddef>
#include <limits>
#include <optional>
#include <string>
#include <vector>

export module pludux:default_method_context;

import :asset_snapshot;
import :series_evaluation_results;
import :method_key;
import :methods;
import :constrained_numeric_input;
import :ordered_named_registry;

export namespace pludux {

class DefaultMethodContext {
public:
  using DispatchResultType = double;

  explicit DefaultMethodContext(
   const OrderedNamedRegistry<AnySeriesMethod>& series_methods,
   SeriesEvaluationResults& series_evaluation_results,
    const OrderedNamedRegistry<ConstrainedNumericInput>& inputs,
   std::size_t current_index = 0) noexcept
  : series_methods_{series_methods}
  , series_evaluation_results_{series_evaluation_results}
  , inputs_{inputs}
  , current_index_{current_index}
  {
  }

  auto call_series_method(this const DefaultMethodContext& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot) noexcept
   -> DispatchResultType
  {
    if(const auto method_opt = self.series_methods_.get(name);
       method_opt.has_value()) {
      return evaluate_series_method(method_opt.value(), asset_snapshot, self);
    }
    return std::numeric_limits<DispatchResultType>::quiet_NaN();
  }

  auto call_series_method(this const DefaultMethodContext& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot,
                          MethodOutput output) noexcept -> DispatchResultType
  {
    if(const auto method_opt = self.series_methods_.get(name);
       method_opt.has_value()) {
      return evaluate_series_method(
       output, method_opt.value(), asset_snapshot, self);
    }
    return std::numeric_limits<DispatchResultType>::quiet_NaN();
  }

  auto get_series_result(this const DefaultMethodContext& self,
                         const std::string& name,
                         std::size_t result_index) noexcept
   -> DispatchResultType
  {
    const auto& method_opt = self.series_methods_.get(name);
    if(!method_opt.has_value()) {
      return std::numeric_limits<DispatchResultType>::quiet_NaN();
    }

    if(const auto results_opt =
        self.series_evaluation_results_.results(name);
       results_opt.has_value()) {
      const auto& results = results_opt.value().get();
      if(result_index < results.size()) {
        return results[result_index];
      }
    }

    if(const auto results_opt =
        self.series_evaluation_results_.results(method_opt.value());
       results_opt.has_value()) {
      const auto& results = results_opt.value().get();
      if(result_index < results.size()) {
        return results[result_index];
      }
    }

    return std::numeric_limits<DispatchResultType>::quiet_NaN();
  }

  auto get_series_results(this DefaultMethodContext& self,
                          const MethodKey& method_key) noexcept
   -> std::vector<double>&
  {
    const auto results_opt =
     self.series_evaluation_results_.results(method_key);

    if(!results_opt.has_value()) {
      // If there are no results for the given method key, we initialize an
      // empty vector of results for it.
      self.series_evaluation_results_.results(method_key, {});
    }

    return self.series_evaluation_results_.results(method_key).value();
  }

  auto index(this const DefaultMethodContext& self) noexcept -> std::size_t
  {
    return self.current_index_;
  }

  auto get_input_value(this const DefaultMethodContext& self,
                       const std::string& key) noexcept -> double
  {
    if(const auto input_opt = self.inputs_.get(key); input_opt.has_value()) {
      const auto& input = input_opt.value();
      return input.resolved_value();
    }
    return std::numeric_limits<double>::quiet_NaN();
  }

private:
  const OrderedNamedRegistry<AnySeriesMethod>& series_methods_;
  SeriesEvaluationResults& series_evaluation_results_;
  const OrderedNamedRegistry<ConstrainedNumericInput>& inputs_;
  std::size_t current_index_;
};

} // namespace pludux
