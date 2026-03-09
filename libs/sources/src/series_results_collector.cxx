module;

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

export module pludux:series_results_collector;

export namespace pludux {

class SeriesResultsCollector {
public:
  SeriesResultsCollector() = default;

  auto results(this const SeriesResultsCollector& self) noexcept
   -> const std::unordered_map<std::string, std::vector<double>>&
  {
    return self.results_;
  }

  void results(
   this SeriesResultsCollector& self,
   std::unordered_map<std::string, std::vector<double>> new_results) noexcept
  {
    self.results_ = std::move(new_results);
  }

  auto results(this const SeriesResultsCollector& self,
               const std::string& series_name) noexcept
   -> std::optional<std::reference_wrapper<const std::vector<double>>>
  {
    const auto it = self.results_.find(series_name);
    if(it != self.results_.end()) {
      return std::cref(it->second);
    }

    return std::nullopt;
  }

  void results(this SeriesResultsCollector& self,
               const std::string& series_name,
               std::vector<double> new_results) noexcept
  {
    self.results_[series_name] = std::move(new_results);
  }

  void collect(const std::string& series_name, double value)
  {
    results_[series_name].emplace_back(value);
  }

  void clear(this SeriesResultsCollector& self) noexcept
  {
    self.results_.clear();
  }

private:
  std::unordered_map<std::string, std::vector<double>> results_;
};

} // namespace pludux