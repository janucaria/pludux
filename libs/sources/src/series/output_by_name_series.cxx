module;

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

export module pludux:series.output_by_name_series;

import :series.series_output;

export namespace pludux {

template<typename TSeries>
class OutputByNameSeries {
public:
  using ValueType = typename TSeries::ValueType;

  OutputByNameSeries(TSeries series, OutputName output_name)
  : series_{std::move(series)}
  , output_name_{output_name}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    const auto ref = self.series_[lookback];
    return ref.get(self.output_name_);
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.series_.size();
  }

  auto output_name(this const auto& self) noexcept -> OutputName
  {
    return self.output_name_;
  }

  void output_name(this auto& self, OutputName new_named_index) noexcept
  {
    self.output_name_ = new_named_index;
  }

private:
  TSeries series_;
  OutputName output_name_;
};

} // namespace pludux
