#include <pludux/screener/stoch_method.hpp>

namespace pludux::screener {

StochMethod::StochMethod(StochOutput output,
                         ScreenerMethod high,
                         ScreenerMethod low,
                         ScreenerMethod close,
                         std::size_t k_period,
                         std::size_t k_smooth,
                         std::size_t d_period,
                         std::size_t offset)
: high_{high}
, low_{low}
, close_{close}
, output_{output}
, k_period_{k_period}
, k_smooth_{k_smooth}
, d_period_{d_period}
, offset_{offset}
{
}

auto StochMethod::operator()(AssetSnapshot asset_data) const
 -> SubSeries<PolySeries<double>>
{
  const auto high_series = high_(asset_data);
  const auto low_series = low_(asset_data);
  const auto close_series = close_(asset_data);

  const auto stoch = StochSeries{output_,
                                 high_series,
                                 low_series,
                                 close_series,
                                 k_period_,
                                 k_smooth_,
                                 d_period_};

  return SubSeries{PolySeries<double>{stoch},
                   static_cast<std::ptrdiff_t>(offset_)};
}

auto StochMethod::operator==(const StochMethod& other) const noexcept -> bool = default;

auto StochMethod::high() const noexcept -> ScreenerMethod
{
  return high_;
}

auto StochMethod::low() const noexcept -> ScreenerMethod
{
  return low_;
}

auto StochMethod::close() const noexcept -> ScreenerMethod
{
  return close_;
}

auto StochMethod::offset() const noexcept -> std::size_t
{
  return offset_;
}

void StochMethod::offset(std::size_t offset) noexcept
{
  offset_ = offset;
}

auto StochMethod::output() const noexcept -> StochOutput
{
  return output_;
}

void StochMethod::output(StochOutput output) noexcept
{
  output_ = output;
}

auto StochMethod::k_period() const noexcept -> std::size_t
{
  return k_period_;
}

void StochMethod::k_period(std::size_t k_period) noexcept
{
  k_period_ = k_period;
}

auto StochMethod::k_smooth() const noexcept -> std::size_t
{
  return k_smooth_;
}

void StochMethod::k_smooth(std::size_t k_smooth) noexcept
{
  k_smooth_ = k_smooth;
}

auto StochMethod::d_period() const noexcept -> std::size_t
{
  return d_period_;
}

void StochMethod::d_period(std::size_t d_period) noexcept
{
  d_period_ = d_period;
}

} // namespace pludux::screener