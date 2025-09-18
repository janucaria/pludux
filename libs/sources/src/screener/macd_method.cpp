#include <pludux/screener/macd_method.hpp>

namespace pludux::screener {

MacdMethod::MacdMethod(ScreenerMethod input,
                       std::size_t fast_period,
                       std::size_t slow_period,
                       std::size_t signal_period)
: input_{input}
, fast_period_{fast_period}
, slow_period_{slow_period}
, signal_period_{signal_period}
{
}

auto MacdMethod::operator==(const MacdMethod& other) const noexcept
 -> bool = default;

auto MacdMethod::operator()(AssetSnapshot asset_data) const
 -> PolySeries<double>
{
  const auto input_series = input_(asset_data);

  const auto macd_series =
   MacdSeries{input_series, fast_period_, slow_period_, signal_period_};

  return macd_series;
}

auto MacdMethod::input() const noexcept -> ScreenerMethod
{
  return input_;
}

void MacdMethod::input(ScreenerMethod input) noexcept
{
  input_ = input;
}

auto MacdMethod::fast_period() const noexcept -> std::size_t
{
  return fast_period_;
}

void MacdMethod::fast_period(std::size_t fast_period) noexcept
{
  fast_period_ = fast_period;
}

auto MacdMethod::slow_period() const noexcept -> std::size_t
{
  return slow_period_;
}

void MacdMethod::slow_period(std::size_t slow_period) noexcept
{
  slow_period_ = slow_period;
}

auto MacdMethod::signal_period() const noexcept -> std::size_t
{
  return signal_period_;
}

void MacdMethod::signal_period(std::size_t signal_period) noexcept
{
  signal_period_ = signal_period;
}

} // namespace pludux::screener