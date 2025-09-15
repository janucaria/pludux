#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/ohlcv_method.hpp>
#include <pludux/screener/ta_with_period_method.hpp>
#include <pludux/series.hpp>

#include <pludux/screener/bullish_divergence_method.hpp>

namespace pludux::screener {

BullishDivergenceMethod::BullishDivergenceMethod()
: BullishDivergenceMethod{RsiMethod{}, CloseMethod{}, 5, 60, 0}
{
}

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
 -> PolySeries<double>
{
  const auto signal_series = signal_(asset_data);
  const auto reference_series = reference_(asset_data);

  const auto bullish_divergence_series = BullishDivergenceSeries{
   signal_series,
   reference_series,
   pivot_range_,
   lookback_range_,
  };

  return LookbackSeries{PolySeries<double>{bullish_divergence_series},
                        static_cast<std::ptrdiff_t>(offset_)};
}

auto BullishDivergenceMethod::operator==(
 const BullishDivergenceMethod& other) const noexcept -> bool = default;

auto BullishDivergenceMethod::offset() const noexcept -> std::size_t
{
  return offset_;
}

void BullishDivergenceMethod::offset(std::size_t offset) noexcept
{
  offset_ = offset;
}

auto BullishDivergenceMethod::pivot_range() const noexcept -> std::size_t
{
  return pivot_range_;
}

void BullishDivergenceMethod::pivot_range(std::size_t pivot_range) noexcept
{
  pivot_range_ = pivot_range;
}

auto BullishDivergenceMethod::lookback_range() const noexcept -> std::size_t
{
  return lookback_range_;
}

void BullishDivergenceMethod::lookback_range(
 std::size_t lookback_range) noexcept
{
  lookback_range_ = lookback_range;
}

auto BullishDivergenceMethod::signal() const noexcept -> const ScreenerMethod&
{
  return signal_;
}

void BullishDivergenceMethod::signal(ScreenerMethod signal) noexcept
{
  signal_ = std::move(signal);
}

auto BullishDivergenceMethod::reference() const noexcept
 -> const ScreenerMethod&
{
  return reference_;
}

void BullishDivergenceMethod::reference(ScreenerMethod reference) noexcept
{
  reference_ = std::move(reference);
}

} // namespace pludux::screener