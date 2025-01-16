#ifndef PLUDUX_PLUDUX_SERIES_CHANGE_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_CHANGE_SERIES_HPP

#include <cstddef>
#include <limits>
#include <utility>

namespace pludux {

template<typename TSeries>
class ChangeSeries {
public:
  using ValueType = typename TSeries::ValueType;

  explicit ChangeSeries(TSeries series)
  : series_{std::move(series)}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    return series_[index] - series_[index + 1];
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size();
  }

private:
  TSeries series_;
};

} // namespace pludux

#endif