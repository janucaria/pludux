#ifndef PLUDUX_PLUDUX_SERIES_VALUE_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_VALUE_SERIES_HPP

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

namespace pludux {

template<typename T>
class DataSeries {
public:
  using ValueType = T;

  explicit DataSeries(std::vector<T> data)
  : data_{std::move(data)}
  {
  }

  template<std::convertible_to<ValueType> U>
  explicit DataSeries(const std::vector<U>& data)
  : DataSeries{std::vector<ValueType>{}}
  {
    data_.reserve(data.size());
    std::transform(
     data.cbegin(),
     data.cend(),
     std::back_inserter(data_),
     [](const auto& value) { return static_cast<double>(value); });
  }

  /**
   * The 0 reverse_index is the latest value.
   * If the reverse_index is out of bounds, return NaN.
   */
  auto operator[](std::size_t index) const -> ValueType
  {
    if(index >= data_.size()) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    const auto value_index = data_.size() - 1 - index;
    return data_[value_index];
  }

  auto size() const noexcept -> std::size_t
  {
    return data_.size();
  }

  auto data() const noexcept -> const std::vector<T>&
  {
    return data_;
  }

  auto data() noexcept -> std::vector<T>&
  {
    return data_;
  }

private:
  std::vector<ValueType> data_;
};

} // namespace pludux

#endif