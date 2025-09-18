#ifndef PLUDUX_PLUDUX_SERIES_OUTPUT_BY_NAME_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_OUTPUT_BY_NAME_SERIES_HPP

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

import pludux.series.series_output;

namespace pludux {

template<typename TSeries>
class OutputByNameSeries {
public:
  using ValueType = typename TSeries::ValueType;

  OutputByNameSeries(TSeries series, OutputName output_name)
  : series_{std::move(series)}
  , output_name_{output_name}
  {
  }

  auto operator[](std::size_t index) const noexcept -> SeriesOutput<ValueType>
  {
    const auto ref = series_[index];
    return ref.get(output_name_);
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size();
  }

  auto output_name() const noexcept -> OutputName
  {
    return output_name_;
  }

  void output_name(OutputName new_named_index) noexcept
  {
    output_name_ = new_named_index;
  }

private:
  TSeries series_;
  OutputName output_name_;
};

} // namespace pludux

#endif