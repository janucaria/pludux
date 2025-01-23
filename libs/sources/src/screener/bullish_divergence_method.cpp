#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/series.hpp>

#include <pludux/screener/bullish_divergence_method.hpp>

namespace pludux::screener {

BullishDivergenceMethod::BullishDivergenceMethod(ScreenerMethod signal,
                                                 ScreenerMethod reference,
                                                 std::size_t pivot_range,
                                                 std::size_t lookback_range,
                                                 std::size_t offset)
: offset_{offset}
, pivot_range_{pivot_range}
, lookback_range_{lookback_range}
, signal_{signal}
, reference_{reference}
{
}

auto BullishDivergenceMethod::operator()(AssetSnapshot asset_data) const
 -> SubSeries<PolySeries<double>>
{
  const auto signal_series = signal_(asset_data);
  const auto reference_series = reference_(asset_data);

  const auto bullish_divergence_series = BullishDivergenceSeries{
   signal_series,
   reference_series,
   pivot_range_,
   lookback_range_,
  };

  return SubSeries{PolySeries<double>{bullish_divergence_series},
                   static_cast<std::ptrdiff_t>(offset_)};
}

} // namespace pludux::screener