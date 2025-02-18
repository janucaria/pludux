#ifndef PLUDUX_PLUDUX_QUOTE_ACCESS_DATA_HPP
#define PLUDUX_PLUDUX_QUOTE_ACCESS_DATA_HPP

#include <pludux/screener.hpp>

namespace pludux {

class QuoteAccess {
public:
  QuoteAccess();

  QuoteAccess(screener::ScreenerMethod time,
              screener::ScreenerMethod open,
              screener::ScreenerMethod high,
              screener::ScreenerMethod low,
              screener::ScreenerMethod close,
              screener::ScreenerMethod volume);

  auto time(AssetSnapshot asset_data) const noexcept
   -> SubSeries<PolySeries<double>>;

  auto time() const noexcept -> const screener::ScreenerMethod&;

  void time(screener::ScreenerMethod time) noexcept;

  auto open(AssetSnapshot asset_data) const noexcept
   -> SubSeries<PolySeries<double>>;

  auto open() const noexcept -> const screener::ScreenerMethod&;

  void open(screener::ScreenerMethod open) noexcept;

  auto high(AssetSnapshot asset_data) const noexcept
   -> SubSeries<PolySeries<double>>;

  auto high() const noexcept -> const screener::ScreenerMethod&;

  void high(screener::ScreenerMethod high) noexcept;

  auto low(AssetSnapshot asset_data) const noexcept
   -> SubSeries<PolySeries<double>>;

  auto low() const noexcept -> const screener::ScreenerMethod&;

  void low(screener::ScreenerMethod low) noexcept;

  auto close(AssetSnapshot asset_data) const noexcept
   -> SubSeries<PolySeries<double>>;

  auto close() const noexcept -> const screener::ScreenerMethod&;

  void close(screener::ScreenerMethod close) noexcept;

  auto volume(AssetSnapshot asset_data) const noexcept
   -> SubSeries<PolySeries<double>>;

  auto volume() const noexcept -> const screener::ScreenerMethod&;

  void volume(screener::ScreenerMethod volume) noexcept;

private:
  screener::ScreenerMethod time_;
  screener::ScreenerMethod open_;
  screener::ScreenerMethod high_;
  screener::ScreenerMethod low_;
  screener::ScreenerMethod close_;
  screener::ScreenerMethod volume_;
};

} // namespace pludux

#endif
