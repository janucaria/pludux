#ifndef PLUDUX_PLUDUX_SERIES_REF_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_REF_SERIES_HPP

#include <cstddef>


import pludux.series.series_output;

namespace pludux {

template<typename TSeries>
class RefSeries {
public:
  using ValueType = typename TSeries::ValueType;

  explicit RefSeries(TSeries& series)
  : series_{series}
  {
  }

  auto operator[](std::size_t index) const -> SeriesOutput<ValueType>
  {
    return series_[index];
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size();
  }

private:
  TSeries& series_;
};

} // namespace pludux

#endif