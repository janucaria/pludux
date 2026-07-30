module;

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

export module pludux:series_evaluation_results;

import :hash_series_method;
import :method_key;

export namespace pludux {

class SeriesEvaluationResults {
public:
  SeriesEvaluationResults() = default;

  explicit SeriesEvaluationResults(
   std::unordered_map<MethodKey, std::vector<double>> results) noexcept
  : results_(std::move(results))
  {
  }

  auto operator==(const SeriesEvaluationResults&) const noexcept
   -> bool = default;

  auto results(this const SeriesEvaluationResults& self) noexcept
   -> const std::unordered_map<MethodKey, std::vector<double>>&
  {
    return self.results_;
  }

  void results(
   this SeriesEvaluationResults& self,
   std::unordered_map<MethodKey, std::vector<double>> new_results) noexcept
  {
    self.results_ = std::move(new_results);
  }

  auto results(this const SeriesEvaluationResults& self,
               const MethodKey& method_key) noexcept
   -> std::optional<std::reference_wrapper<const std::vector<double>>>
  {
    const auto it = self.results_.find(method_key);
    if(it != self.results_.end()) {
      return std::cref(it->second);
    }

    return std::nullopt;
  }

  auto results(this const SeriesEvaluationResults& self,
               const std::string& name) noexcept
   -> std::optional<std::reference_wrapper<const std::vector<double>>>
  {
    const auto it = self.aliases_.find(name);
    if(it == self.aliases_.end()) {
      return std::nullopt;
    }

    return self.results(it->second);
  }

  auto results(this SeriesEvaluationResults& self,
               const MethodKey& method_key) noexcept
   -> std::optional<std::reference_wrapper<std::vector<double>>>
  {
    const auto it = self.results_.find(method_key);
    if(it != self.results_.end()) {
      return std::ref(it->second);
    }

    return std::nullopt;
  }

  auto results(this SeriesEvaluationResults& self,
               const std::string& name) noexcept
   -> std::optional<std::reference_wrapper<std::vector<double>>>
  {
    const auto it = self.aliases_.find(name);
    if(it == self.aliases_.end()) {
      return std::nullopt;
    }

    return self.results(it->second);
  }

  void results(this SeriesEvaluationResults& self,
               const MethodKey& method_key,
               std::vector<double> new_results) noexcept
  {
    self.results_[method_key] = std::move(new_results);
  }

  void alias(this SeriesEvaluationResults& self,
             const std::string& name,
             const MethodKey& method_key)
  {
    self.aliases_.insert_or_assign(name, method_key);
  }

  void put(const MethodKey& method_key, double value)
  {
    results_[method_key].emplace_back(value);
  }

  void clear(this SeriesEvaluationResults& self) noexcept
  {
    self.results_.clear();
    self.aliases_.clear();
  }

  auto empty(this const SeriesEvaluationResults& self) noexcept -> bool
  {
    return self.results_.empty();
  }

private:
  std::unordered_map<MethodKey, std::vector<double>> results_;
  std::unordered_map<std::string, MethodKey> aliases_;
};

} // namespace pludux
