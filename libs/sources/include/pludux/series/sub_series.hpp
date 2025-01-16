#ifndef PLUDUX_PLUDUX_SERIES_SUB_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_SUB_SERIES_HPP

#include <cstddef>
#include <utility>

namespace pludux {

template<typename TSeries>
class SubSeries {
public:
  using ValueType = typename TSeries::ValueType;

  SubSeries(TSeries series, std::size_t offset)
  : series_{std::move(series)}
  , offset_{offset}
  {
  }

  auto operator[](std::size_t index) const -> TSeries::ValueType
  {
    return series_[index + offset_];
  }

  auto
  subseries(std::size_t offset) const noexcept -> SubSeries<const TSeries>
  {
    return SubSeries{series_, offset_ + offset};
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size() - offset_;
  }

private:
  TSeries series_;
  std::size_t offset_;
};

} // namespace pludux

#endif