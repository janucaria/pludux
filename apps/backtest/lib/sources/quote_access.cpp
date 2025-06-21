#include <pludux/quote_access.hpp>

namespace pludux {

QuoteAccess::QuoteAccess()
: QuoteAccess{screener::DataMethod{"Date"},
              screener::DataMethod{"Open"},
              screener::DataMethod{"High"},
              screener::DataMethod{"Low"},
              screener::DataMethod{"Close"},
              screener::DataMethod{"Volume"}}
{
}

QuoteAccess::QuoteAccess(screener::ScreenerMethod time,
                         screener::ScreenerMethod open,
                         screener::ScreenerMethod high,
                         screener::ScreenerMethod low,
                         screener::ScreenerMethod close,
                         screener::ScreenerMethod volume)
: time_{time}
, open_{open}
, high_{high}
, low_{low}
, close_{close}
, volume_{volume}
{
}

auto QuoteAccess::time(AssetSnapshot asset_data) const noexcept
 -> SubSeries<PolySeries<double>>
{
  return time_(asset_data);
}

auto QuoteAccess::time() const noexcept -> const screener::ScreenerMethod&
{
  return time_;
}

void QuoteAccess::time(screener::ScreenerMethod time) noexcept
{
  time_ = time;
}

auto QuoteAccess::open(AssetSnapshot asset_data) const noexcept
 -> SubSeries<PolySeries<double>>
{
  return open_(asset_data);
}

auto QuoteAccess::open() const noexcept -> const screener::ScreenerMethod&
{
  return open_;
}

void QuoteAccess::open(screener::ScreenerMethod open) noexcept
{
  open_ = open;
}

auto QuoteAccess::high(AssetSnapshot asset_data) const noexcept
 -> SubSeries<PolySeries<double>>
{
  return high_(asset_data);
}

auto QuoteAccess::high() const noexcept -> const screener::ScreenerMethod&
{
  return high_;
}

void QuoteAccess::high(screener::ScreenerMethod high) noexcept
{
  high_ = high;
}

auto QuoteAccess::low(AssetSnapshot asset_data) const noexcept
 -> SubSeries<PolySeries<double>>
{
  return low_(asset_data);
}

auto QuoteAccess::low() const noexcept -> const screener::ScreenerMethod&
{
  return low_;
}

void QuoteAccess::low(screener::ScreenerMethod low) noexcept
{
  low_ = low;
}

auto QuoteAccess::close(AssetSnapshot asset_data) const noexcept
 -> SubSeries<PolySeries<double>>
{
  return close_(asset_data);
}

auto QuoteAccess::close() const noexcept -> const screener::ScreenerMethod&
{
  return close_;
}

void QuoteAccess::close(screener::ScreenerMethod close) noexcept
{
  close_ = close;
}

auto QuoteAccess::volume(AssetSnapshot asset_data) const noexcept
 -> SubSeries<PolySeries<double>>
{
  return volume_(asset_data);
}

auto QuoteAccess::volume() const noexcept -> const screener::ScreenerMethod&
{
  return volume_;
}

void QuoteAccess::volume(screener::ScreenerMethod volume) noexcept
{
  volume_ = volume;
}

} // namespace pludux
