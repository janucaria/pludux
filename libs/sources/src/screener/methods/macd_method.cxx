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

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    const auto input_series = self.input_(asset_data);

    const auto macd_series = MacdSeries{
     input_series, self.fast_period_, self.slow_period_, self.signal_period_};

    return macd_series;
  }

  auto input(this const auto& self) noexcept -> ScreenerMethod
  {
    return self.input_;
  }

  void input(this auto& self, ScreenerMethod input) noexcept
  {
    self.input_ = input;
  }

  auto fast_period(this const auto& self) noexcept -> std::size_t
  {
    return self.fast_period_;
  }

  void fast_period(this auto& self, std::size_t fast_period) noexcept
  {
    self.fast_period_ = fast_period;
  }

  auto slow_period(this const auto& self) noexcept -> std::size_t
  {
    return self.slow_period_;
  }

  void slow_period(this auto& self, std::size_t slow_period) noexcept
  {
    self.slow_period_ = slow_period;
  }

  auto signal_period(this const auto& self) noexcept -> std::size_t
  {
    return self.signal_period_;
  }

  void signal_period(this auto& self, std::size_t signal_period) noexcept
  {
    self.signal_period_ = signal_period;
  }

private:
  ScreenerMethod input_;
  std::size_t fast_period_;
  std::size_t slow_period_;
  std::size_t signal_period_;
};

} // namespace pludux::screener