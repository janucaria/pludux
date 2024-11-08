#include <utility>

#include <pludux/series.hpp>

namespace pludux {

Series::Series() = default;

Series::Series(std::vector<double> data)
: data_{std::move(data)}
{
}

auto Series::operator[](std::size_t index) const -> double
{
  if(index >= data_.size()) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return data_[data_.size() - 1 - index];
}

auto Series::size() const -> std::size_t
{
  return data_.size();
}

auto Series::data() const -> const std::vector<double>&
{
  return data_;
}

auto Series::subseries(std::size_t offset) const -> Series
{
  if(offset >= data_.size()) {
    return Series{};
  }

  return Series{
   std::vector<double>{data_.begin(), data_.begin() + data_.size() - offset}};
}

} // namespace pludux
