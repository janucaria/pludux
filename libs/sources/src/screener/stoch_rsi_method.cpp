#include <pludux/screener/stoch_rsi_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

StochRsiMethod::StochRsiMethod(ScreenerMethod rsi_input,
                               std::size_t rsi_period,
                               std::size_t k_period,
                               std::size_t k_smooth,
                               std::size_t d_period)
: rsi_input_{rsi_input}
, rsi_period_{rsi_period}
, k_period_{k_period}
, k_smooth_{k_smooth}
, d_period_{d_period}
{
}

auto StochRsiMethod::operator()(AssetSnapshot asset_data) const
 -> PolySeries<double>
{
  const auto stoch_rsi_series = StochRsiSeries{
   rsi_input_(asset_data), rsi_period_, k_period_, k_smooth_, d_period_};
  return stoch_rsi_series;
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