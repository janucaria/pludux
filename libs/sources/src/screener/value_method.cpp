#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset.hpp>

#include <pludux/screener/value_method.hpp>

namespace pludux::screener {

ValueMethod::ValueMethod(double value)
: value_{value}
{
}

auto ValueMethod::run_all(const Asset& asset) const -> Series
{
  auto values = std::vector<double>(asset.quotes().size(), value_);
  return Series{std::move(values)};
}

auto ValueMethod::run_one(const Asset& asset) const -> double
{
  return value_;
}

} // namespace pludux::screener