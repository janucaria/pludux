#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/series.hpp>

#include <pludux/screener/hidden_bullish_divergence_method.hpp>

namespace pludux::screener {

HiddenBullishDivergenceMethod::HiddenBullishDivergenceMethod(
 ScreenerMethod signal,
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

auto HiddenBullishDivergenceMethod::operator()(AssetSnapshot asset_data) const
 -> SubSeries<PolySeries<double>>
{
  const auto signal_series = signal_(asset_data);
  const auto reference_series = reference_(asset_data);

  const auto hidden_bullish_divergence_series = HiddenBullishDivergenceSeries{
   signal_series,
   reference_series,
   pivot_range_,
   lookback_range_,
  };

  return SubSeries{PolySeries<double>{hidden_bullish_divergence_series},
                   static_cast<std::ptrdiff_t>(offset_)};
}

auto HiddenBullishDivergenceMethod::operator==(
 const HiddenBullishDivergenceMethod& other) const noexcept -> bool = default;

auto HiddenBullishDivergenceMethod::offset() const noexcept -> std::size_t
{
  return offset_;
}

auto HiddenBullishDivergenceMethod::pivot_range() const noexcept -> std::size_t
{
  return pivot_range_;
}

auto HiddenBullishDivergenceMethod::lookback_range() const noexcept
 -> std::size_t
{
  return lookback_range_;
}

auto HiddenBullishDivergenceMethod::signal() const noexcept
 -> const ScreenerMethod&
{
  return signal_;
}

auto HiddenBullishDivergenceMethod::reference() const noexcept
 -> const ScreenerMethod&
{
  return reference_;
}

} // namespace pludux::screener