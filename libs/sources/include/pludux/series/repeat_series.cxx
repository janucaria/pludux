module;

#include <cstddef>
#include <limits>

export module pludux.series.repeat_series;

namespace pludux {

export template<typename T>
  requires std::numeric_limits<T>::has_quiet_NaN
class RepeatSeries {
public:
  using ValueType = T;

  RepeatSeries(T value, std::size_t size)
  : value_{value}
  , size_{size}
  {
  }

  /**
   * The 0 reverse_index is the latest value.
   * If the reverse_index is out of bounds, return NaN.
   */
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
