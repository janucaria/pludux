#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/screener/ohlcv_method.hpp>
#include <pludux/screener/ta_with_period_method.hpp>

#include <pludux/screener/hidden_bullish_divergence_method.hpp>

namespace pludux::screener {

HiddenBullishDivergenceMethod::HiddenBullishDivergenceMethod()
: HiddenBullishDivergenceMethod{RsiMethod{}, CloseMethod{}, 5, 60}
{
}

HiddenBullishDivergenceMethod::HiddenBullishDivergenceMethod(
 ScreenerMethod signal,
 ScreenerMethod reference,
 std::size_t pivot_range,
 std::size_t lookback_range)
: pivot_range_{pivot_range}
, lookback_range_{lookback_range}
, signal_{signal}
, reference_{reference}
{
}

auto HiddenBullishDivergenceMethod::operator==(
 const HiddenBullishDivergenceMethod& other) const noexcept -> bool = default;

auto HiddenBullishDivergenceMethod::operator()(AssetSnapshot asset_data) const
 -> PolySeries<double>
{
  const auto signal_series = signal_(asset_data);
  const auto reference_series = reference_(asset_data);

  const auto hidden_bullish_divergence_series = HiddenBullishDivergenceSeries{
   signal_series,
   reference_series,
   pivot_range_,
   lookback_range_,
  };

  return hidden_bullish_divergence_series;
}

auto HiddenBullishDivergenceMethod::pivot_range() const noexcept -> std::size_t
{
  return pivot_range_;
}

void HiddenBullishDivergenceMethod::pivot_range(
 std::size_t pivot_range) noexcept
{
  pivot_range_ = pivot_range;
}

auto HiddenBullishDivergenceMethod::lookback_range() const noexcept
 -> std::size_t
{
  return lookback_range_;
}

void HiddenBullishDivergenceMethod::lookback_range(
 std::size_t lookback_range) noexcept
{
  lookback_range_ = lookback_range;
}

auto HiddenBullishDivergenceMethod::signal() const noexcept
 -> const ScreenerMethod&
{
  return signal_;
}

void HiddenBullishDivergenceMethod::signal(ScreenerMethod signal) noexcept
{
  signal_ = std::move(signal);
}

auto HiddenBullishDivergenceMethod::reference() const noexcept
 -> const ScreenerMethod&
{
  return reference_;
}

void HiddenBullishDivergenceMethod::reference(ScreenerMethod reference) noexcept
{
  reference_ = std::move(reference);
}

} // namespace pludux::screener