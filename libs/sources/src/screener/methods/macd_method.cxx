module;

#include <cstddef>

export module pludux:screener.macd_method;

import :asset_snapshot;
import :screener.screener_method;

export namespace pludux::screener {

class MacdMethod {
public:
  MacdMethod(ScreenerMethod input,
             std::size_t fast_period,
             std::size_t slow_period,
             std::size_t signal_period)
  : input_{input}
  , fast_period_{fast_period}
  , slow_period_{slow_period}
  , signal_period_{signal_period}
  {
  }

  auto operator==(const MacdMethod& other) const noexcept -> bool = default;

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    const auto input_series = input_(asset_data);

    const auto macd_series =
     MacdSeries{input_series, fast_period_, slow_period_, signal_period_};

    return macd_series;
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
  ScreenerMethod input_;
  std::size_t fast_period_;
  std::size_t slow_period_;
  std::size_t signal_period_;
};

} // namespace pludux::screener