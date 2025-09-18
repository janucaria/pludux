module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux.series.macd_series;

import pludux.series.series_output;
import pludux.series.ref_series;
import pludux.series.ema_series;
import pludux.series.operators_series;

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

  auto operator[](std::size_t index) const noexcept -> SeriesOutput<ValueType>
  {
    const auto input_series = RefSeries{input_};
    const auto short_ema_series = EmaSeries{input_series, short_period_};
    const auto long_ema_series = EmaSeries{input_series, long_period_};
    const auto macd_series = SubtractSeries{short_ema_series, long_ema_series};

    const auto macd = macd_series[index];

    const auto signal_series = EmaSeries{macd_series, signal_period_};
    const auto signal = signal_series[index];
    const auto histogram = macd - signal;

    return {{macd, signal, histogram},
            {
             {OutputName::MacdLine, 0},
             {OutputName::SignalLine, 1},
             {OutputName::MacdHistogram, 2},
            }};
  }

  auto size() const noexcept -> std::size_t
  {
    return input_.size();
  }

  auto input() const noexcept
  {
    return input_;
  }

private:
  TSeries input_;
  std::size_t short_period_;
  std::size_t long_period_;
  std::size_t signal_period_;
};

} // namespace pludux
