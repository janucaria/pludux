#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/atr_method.hpp>
#include <pludux/ta.hpp>

namespace pludux::screener {

AtrMethod::AtrMethod(std::size_t period, std::size_t offset)
: period_{period}
, offset_{offset}
{
}

auto AtrMethod::operator()(AssetSnapshot asset_data) const
 -> SubSeries<PolySeries<double>>
{
  const auto high_series = asset_data["high"];
  const auto low_series = asset_data["low"];
  const auto close_series = asset_data["close"];

  const auto atr = ta::atr(high_series, low_series, close_series, period_);
  return SubSeries{PolySeries<double>{atr},
                   static_cast<std::ptrdiff_t>(offset_)};
}

} // namespace pludux::screener