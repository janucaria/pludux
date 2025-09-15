#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/ohlcv_method.hpp>
#include <pludux/ta.hpp>

#include <pludux/screener/changes_method.hpp>

namespace pludux::screener {

ChangesMethod::ChangesMethod()
: ChangesMethod{CloseMethod{}, 0}
{
}

ChangesMethod::ChangesMethod(ScreenerMethod input, std::size_t offset)
: input_{input}
, offset_{offset}
{
}

auto ChangesMethod::operator()(AssetSnapshot asset_data) const
 -> PolySeries<double>
{
  const auto input_series = input_(asset_data);
  const auto result = ta::changes(input_series);
  return LookbackSeries{PolySeries<double>{result},
                        static_cast<std::ptrdiff_t>(offset_)};
}

auto ChangesMethod::operator==(const ChangesMethod& other) const noexcept
 -> bool = default;

auto ChangesMethod::offset() const noexcept -> std::size_t
{
  return offset_;
}

void ChangesMethod::offset(std::size_t offset) noexcept
{
  offset_ = offset;
}

auto ChangesMethod::input() const noexcept -> const ScreenerMethod&
{
  return input_;
}

void ChangesMethod::input(ScreenerMethod input) noexcept
{
  input_ = std::move(input);
}

} // namespace pludux::screener