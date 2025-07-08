module;

#include <cstddef>

#include <pludux/screener/screener_method.hpp>

export module pludux.screener.macd_method;

import pludux.asset_snapshot;
import pludux.series;

namespace pludux::screener {

export class MacdMethod {
public:
  MacdMethod(MacdOutput output,
             ScreenerMethod input,
             std::size_t fast_period,
             std::size_t slow_period,
             std::size_t signal_period,
             std::size_t offset = 0)
  : input_{input}
  , output_{output}
  , offset_{offset}
  , fast_period_{fast_period}
  , slow_period_{slow_period}
  , signal_period_{signal_period}
  {
  }

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>
  {
    const auto input_series = input_(asset_data);

    const auto macd_series = MacdSeries{
     output_, input_series, fast_period_, slow_period_, signal_period_};

    return SubSeries{PolySeries<double>{macd_series},
                     static_cast<std::ptrdiff_t>(offset_)};
  }

  auto operator==(const MacdMethod& other) const noexcept -> bool = default;

  auto offset() const noexcept -> std::size_t
  {
    return offset_;
  }

  void offset(std::size_t offset) noexcept
  {
    offset_ = offset;
  }

  auto output() const noexcept -> MacdOutput
  {
    return output_;
  }

  void output(MacdOutput output) noexcept
  {
    output_ = output;
  }

  auto input() const noexcept -> ScreenerMethod
  {
    return input_;
  }

  void input(ScreenerMethod input) noexcept
  {
    input_ = input;
  }

  auto fast_period() const noexcept -> std::size_t
  {
    return fast_period_;
  }

  void fast_period(std::size_t fast_period) noexcept
  {
    fast_period_ = fast_period;
  }

  auto slow_period() const noexcept -> std::size_t
  {
    return slow_period_;
  }

  void slow_period(std::size_t slow_period) noexcept
  {
    slow_period_ = slow_period;
  }

  auto signal_period() const noexcept -> std::size_t
  {
    return signal_period_;
  }

  void signal_period(std::size_t signal_period) noexcept
  {
    signal_period_ = signal_period;
  }

private:
  MacdOutput output_;
  ScreenerMethod input_;
  std::size_t offset_;
  std::size_t fast_period_;
  std::size_t slow_period_;
  std::size_t signal_period_;
};

} // namespace pludux::screener
