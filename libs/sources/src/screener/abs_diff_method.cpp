#include <algorithm>
#include <iterator>
#include <vector>


#include <pludux/screener/abs_diff_method.hpp>

namespace pludux::screener {

auto AbsDiffMethod::minuend() const noexcept -> const ScreenerMethod&
{
  return operand1();
}

auto AbsDiffMethod::subtrahend() const noexcept -> const ScreenerMethod&
{
  return operand2();
}

} // namespace pludux::screener