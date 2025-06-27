#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/stoch_rsi_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

StochRsiMethod::StochRsiMethod(StochOutput output,
                               ScreenerMethod rsi_input,
                               std::size_t rsi_period,
                               std::size_t k_period,
                               std::size_t k_smooth,
                               std::size_t d_period,
                               std::size_t offset)
: rsi_input_{rsi_input}
, output_{output}
, rsi_period_{rsi_period}
, k_period_{k_period}
, k_smooth_{k_smooth}
, d_period_{d_period}
, offset_{offset}
{
}

auto StochRsiMethod::operator()(AssetSnapshot asset_data) const
 -> SubSeries<PolySeries<double>>
{
  const auto stoch_rsi_series = StochRsiSeries{output_,
                                               rsi_input_(asset_data),
                                               rsi_period_,
                                               k_period_,
                                               k_smooth_,
                                               d_period_};

  return SubSeries{PolySeries<double>{stoch_rsi_series},
                   static_cast<std::ptrdiff_t>(offset_)};
}

auto StochRsiMethod::operator==(const StochRsiMethod& other) const noexcept
 -> bool = default;

auto StochRsiMethod::rsi_input() const noexcept -> ScreenerMethod
{
  return rsi_input_;
}

auto StochRsiMethod::rsi_period() const noexcept -> std::size_t
{
  return rsi_period_;
}

auto StochRsiMethod::offset() const noexcept -> std::size_t
{
  return offset_;
}

void StochRsiMethod::offset(std::size_t offset) noexcept
{
  offset_ = offset;
}

auto StochRsiMethod::output() const noexcept -> StochOutput
{
  return output_;
}

void StochRsiMethod::output(StochOutput output) noexcept
{
  output_ = output;
}

auto StochRsiMethod::k_period() const noexcept -> std::size_t
{
  return k_period_;
}

void StochRsiMethod::k_period(std::size_t k_period) noexcept
{
  k_period_ = k_period;
}

auto StochRsiMethod::k_smooth() const noexcept -> std::size_t
{
  return k_smooth_;
}

void StochRsiMethod::k_smooth(std::size_t k_smooth) noexcept
{
  k_smooth_ = k_smooth;
}

auto StochRsiMethod::d_period() const noexcept -> std::size_t
{
  return d_period_;
}

void StochRsiMethod::d_period(std::size_t d_period) noexcept
{
  d_period_ = d_period;
}

} // namespace pludux::screener