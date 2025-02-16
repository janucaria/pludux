#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/macd_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

MacdMethod::MacdMethod(ScreenerMethod input,
                       MacdOutput output,

                       std::size_t fast_period,
                       std::size_t slow_period,
                       std::size_t signal_period,
                       std::size_t offset)
: input_{input}
, output_{output}
, offset_{offset}
, fast_period_{fast_period}
, slow_period_{slow_period}
, signal_period_{signal_period}
{
}

auto MacdMethod::operator()(AssetSnapshot asset_data) const
 -> SubSeries<PolySeries<double>>
{
  const auto input_series = input_(asset_data);

  const auto macd_series = MacdSeries{
   input_series, output_, fast_period_, slow_period_, signal_period_};

  return SubSeries{PolySeries<double>{macd_series},
                   static_cast<std::ptrdiff_t>(offset_)};
}

auto MacdMethod::offset() const noexcept -> std::size_t
{
  return offset_;
}

void MacdMethod::offset(std::size_t offset) noexcept
{
  offset_ = offset;
}

auto MacdMethod::output() const noexcept -> MacdOutput
{
  return output_;
}

void MacdMethod::output(MacdOutput output) noexcept
{
  output_ = output;
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