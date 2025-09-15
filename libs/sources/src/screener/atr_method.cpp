#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/atr_method.hpp>
#include <pludux/screener/ohlcv_method.hpp>
#include <pludux/series/atr_series.hpp>

namespace pludux::screener {

AtrMethod::AtrMethod()
: AtrMethod{HighMethod{}, LowMethod{}, CloseMethod{}, 14, 1.0, 0}
{
}

AtrMethod::AtrMethod(ScreenerMethod high,
                     ScreenerMethod low,
                     ScreenerMethod close,
                     std::size_t period,
                     double multiplier,
                     std::size_t offset)
: period_{period}
, multiplier_{multiplier}
, offset_{offset}
, high_{std::move(high)}
, low_{std::move(low)}
, close_{std::move(close)}
{
}

auto AtrMethod::operator()(AssetSnapshot asset_data) const -> PolySeries<double>
{
  const auto high_series = high_(asset_data);
  const auto low_series = low_(asset_data);
  const auto close_series = close_(asset_data);

  const auto atr =
   AtrSeries{high_series, low_series, close_series, period_, multiplier_};

  return LookbackSeries{PolySeries<double>{atr},
                        static_cast<std::ptrdiff_t>(offset_)};
}

auto AtrMethod::operator==(const AtrMethod& other) const noexcept
 -> bool = default;

auto AtrMethod::period() const noexcept -> std::size_t
{
  return period_;
}

void AtrMethod::period(std::size_t period) noexcept
{
  period_ = period;
}

auto AtrMethod::multiplier() const noexcept -> double
{
  return multiplier_;
}

void AtrMethod::multiplier(double multiplier) noexcept
{
  multiplier_ = multiplier;
}

auto AtrMethod::offset() const noexcept -> std::size_t
{
  return offset_;
}

void AtrMethod::offset(std::size_t offset) noexcept
{
  offset_ = offset;
}

auto AtrMethod::high() const noexcept -> const ScreenerMethod&
{
  return high_;
}

void AtrMethod::high(ScreenerMethod high) noexcept
{
  high_ = std::move(high);
}

auto AtrMethod::low() const noexcept -> const ScreenerMethod&
{
  return low_;
}

void AtrMethod::low(ScreenerMethod low) noexcept
{
  low_ = std::move(low);
}

auto AtrMethod::close() const noexcept -> const ScreenerMethod&
{
  return close_;
}

void AtrMethod::close(ScreenerMethod close) noexcept
{
  close_ = std::move(close);
}

} // namespace pludux::screener