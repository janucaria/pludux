module;

#include <cstddef>
#include <limits>

export module pludux.series.repeat_series;

import pludux.series.series_output;

export namespace pludux {

template<typename T>
class RepeatSeries {
public:
  using ValueType = T;

  RepeatSeries(T value, std::size_t size)
  : value_{value}
  , size_{size}
  {
  }

  auto operator[](std::size_t index) const -> ValueType
  {
    if(index >= size()) {
      return std::numeric_limits<T>::quiet_NaN();
    }

    return value_;
  }

  auto size() const noexcept -> std::size_t
  {
    return size_;
  }

private:
  T value_;
  std::size_t size_;
};

} // namespace pludux
