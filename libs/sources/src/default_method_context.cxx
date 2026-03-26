module;

#include <cstddef>
#include <limits>
#include <optional>
#include <string>
#include <unordered_map>

export module pludux:default_method_context;

import :series_results_collector;
import :series.series_method_registry;

export namespace pludux {

class DefaultMethodContext {
public:
  using DispatchResultType = double;

  explicit DefaultMethodContext(const SeriesMethodRegistry& methods,
                                const SeriesResultsCollector& results_collector,
                                std::size_t current_index = 0) noexcept
  : methods_{methods}
  , results_collector_{results_collector}
  , current_index_{current_index}
  {
  }

  auto call_series_method(this const DefaultMethodContext& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot) noexcept
   -> DispatchResultType
  {
    if(const auto method_opt = self.methods_.get(name);
       method_opt.has_value()) {
      const auto& method = method_opt.value();
      return method(asset_snapshot, self);
    }
    return std::numeric_limits<DispatchResultType>::quiet_NaN();
  }

  auto call_series_method(this const DefaultMethodContext& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot,
                          SeriesOutput output) noexcept -> DispatchResultType
  {
    if(const auto method_opt = self.methods_.get(name);
       method_opt.has_value()) {
      const auto& method = method_opt.value();
      return method(asset_snapshot, output, self);
    }
    return std::numeric_limits<DispatchResultType>::quiet_NaN();
  }

  auto get_series_result(this const DefaultMethodContext& self,
                         const std::string& name,
                         std::size_t result_index) noexcept
   -> DispatchResultType
  {
    if(const auto results_opt = self.results_collector_.results(name);
       results_opt.has_value()) {
      const auto& results = results_opt.value().get();
      if(result_index < results.size()) {
        return results[result_index];
      }
    }
    return std::numeric_limits<DispatchResultType>::quiet_NaN();
  }

  auto index(this const DefaultMethodContext& self) noexcept -> std::size_t
  {
    return self.current_index_;
  }

private:
  const SeriesMethodRegistry& methods_{};
  const SeriesResultsCollector& results_collector_{};
  std::size_t current_index_ = 0;
};

} // namespace pludux