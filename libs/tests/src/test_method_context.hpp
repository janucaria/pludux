#pragma once

#include <cstddef>
#include <functional>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

import pludux;

struct StatelessMethodContext
{
  auto get_series_results(const auto&) const -> std::vector<double>&
  {
    static thread_local auto results = std::vector<double>{};
    return results;
  }
};

class NamedSeriesTestContext
{
public:
  using Method = pludux::ErasedSeriesMethod<NamedSeriesTestContext>;
  using Registry = pludux::OrderedNamedRegistry<Method>;

  NamedSeriesTestContext(const Registry& methods,
                         pludux::SeriesEvaluationResults& results,
                         std::size_t current_index)
      : methods_{methods}, results_{results}, current_index_{current_index}
  {}

  auto call_series_method(const std::string& name,
                          pludux::AssetSnapshot asset) const -> double
  {
    const auto method = methods_.get(name);
    if (!method) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return pludux::evaluate_series_method(method.value(), asset, *this);
  }

  auto call_series_method(const std::string& name,
                          pludux::AssetSnapshot asset,
                          pludux::MethodOutput output) const -> double
  {
    const auto method = methods_.get(name);
    if (!method) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return pludux::evaluate_series_method(output, method.value(), asset, *this);
  }

  auto get_series_result(const std::string& name, std::size_t index) const
   -> double
  {
    const auto values = results_.results(name);
    if (!values || index >= values->get().size()) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    return values->get()[index];
  }

  auto get_series_results(const auto& method_key)
   -> std::vector<double>&
  {
    if (!results_.results(method_key)) {
      results_.results(method_key, {});
    }
    return results_.results(method_key).value();
  }

  auto index() const -> std::size_t
  {
    return current_index_;
  }

private:
  const Registry& methods_;
  pludux::SeriesEvaluationResults& results_;
  std::size_t current_index_;
};
