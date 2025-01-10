#ifndef PLUDUX_PLUDUX_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_HPP

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

namespace pludux {

class Series {
public:
  Series();

  explicit Series(std::vector<double> data);

  template<std::convertible_to<double> T>
  explicit Series(const std::vector<T>& data)
  : data_{}
  {
    data_.reserve(data.size());
    std::transform(
     data.cbegin(),
     data.cend(),
     std::back_inserter(data_),
     [](const auto& value) { return static_cast<double>(value); });
  }

  /**
   * The 0 index is the latest value.
   * If the index is out of bounds, return NaN.
   */
  auto operator[](std::size_t index) const -> double;

  auto size() const -> std::size_t;

  auto data() const noexcept -> const std::vector<double>&;
  
  void append(double value);

  auto subseries(std::size_t offset) const -> Series;

private:
  std::vector<double> data_;
};

} // namespace pludux

#endif