module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.macd_series;

import :series.series_output;
import :series.ref_series;
import :series.ema_series;
import :series.operators_series;

export namespace pludux {

template<typename TSeries>
class MacdSeries {
public:
  using ValueType = typename TSeries::ValueType;

  MacdSeries(TSeries input,
             std::size_t short_period,
             std::size_t long_period,
             std::size_t signal_period)
  : input_{std::move(input)}
  , short_period_{short_period}
  , long_period_{long_period}
  , signal_period_{signal_period}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    const auto input_series = RefSeries{self.input_};
    const auto short_ema_series = EmaSeries{input_series, self.short_period_};
    const auto long_ema_series = EmaSeries{input_series, self.long_period_};
    const auto macd_series = SubtractSeries{short_ema_series, long_ema_series};

    const auto macd = macd_series[lookback];

    const auto signal_series = EmaSeries{macd_series, self.signal_period_};
    const auto signal = signal_series[lookback];
    const auto histogram = macd - signal;

    return {{macd, signal, histogram},
            {
             {OutputName::MacdLine, 0},
             {OutputName::SignalLine, 1},
             {OutputName::MacdHistogram, 2},
            }};
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.input_.size();
  }

  auto input(this const auto& self) noexcept -> const TSeries&
  {
    return self.input_;
  }

private:
  TSeries input_;
  std::size_t short_period_;
  std::size_t long_period_;
  std::size_t signal_period_;
};

} // namespace pludux
