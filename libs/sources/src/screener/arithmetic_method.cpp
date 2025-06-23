#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/arithmetic_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

auto MultiplyMethod::multiplicand() const noexcept -> const ScreenerMethod&
{
  return operand1();
}

auto MultiplyMethod::multiplier() const noexcept -> const ScreenerMethod&
{
  return operand2();
}

auto DivideMethod::dividend() const noexcept -> const ScreenerMethod&
{
  return operand1();
}

auto DivideMethod::divisor() const noexcept -> const ScreenerMethod&
{
  return operand2();
}

auto AddMethod::augend() const noexcept -> const ScreenerMethod&
{
  return operand1();
}

auto AddMethod::addend() const noexcept -> const ScreenerMethod&
{
  return operand2();
}

auto SubtractMethod::minuend() const noexcept -> const ScreenerMethod&
{
  return operand1();
}

auto SubtractMethod::subtrahend() const noexcept -> const ScreenerMethod&
{
  return operand2();
}

auto PercentageMethod::total() const noexcept -> const ScreenerMethod&
{
  return operand1();
}

auto PercentageMethod::percent() const noexcept -> const ScreenerMethod&
{
  return operand2();
}

} // namespace pludux::screener