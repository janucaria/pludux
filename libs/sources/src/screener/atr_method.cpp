#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/atr_method.hpp>
#include <pludux/ta.hpp>

namespace pludux::screener {

AtrMethod::AtrMethod(ScreenerMethod high,
                     ScreenerMethod low,
                     ScreenerMethod close,
                     std::size_t period,
                     std::size_t offset)
: period_{period}
, offset_{offset}
, high_{std::move(high)}
, low_{std::move(low)}
, close_{std::move(close)}
{
}

auto AtrMethod::operator()(AssetSnapshot asset_data) const
 -> SubSeries<PolySeries<double>>
{
  const auto high_series = high_(asset_data);
  const auto low_series = low_(asset_data);
  const auto close_series = close_(asset_data);

  const auto atr = ta::atr(high_series, low_series, close_series, period_);
  return SubSeries{PolySeries<double>{atr},
                   static_cast<std::ptrdiff_t>(offset_)};
}

} // namespace pludux::screener