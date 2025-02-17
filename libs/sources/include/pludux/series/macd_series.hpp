#ifndef PLUDUX_PLUDUX_SERIES_MACD_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_MACD_SERIES_HPP

#include <cstddef>
#include <limits>
#include <utility>

#include "binary_fn_series.hpp"
#include "ema_series.hpp"
#include "ref_series.hpp"

namespace pludux {

enum class MacdOutput { macd, signal, histogram };

template<typename TSeries>
class MacdSeries {
public:
  using ValueType = typename TSeries::ValueType;

  MacdSeries(MacdOutput output,
             TSeries input,
             std::size_t short_period,
             std::size_t long_period,
             std::size_t signal_period)
  : input_{std::move(input)}
  , output_{output}
  , short_period_{short_period}
  , long_period_{long_period}
  , signal_period_{signal_period}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    const auto input_series = RefSeries{input_};
    const auto short_ema_series = EmaSeries{input_series, short_period_};
    const auto long_ema_series = EmaSeries{input_series, long_period_};
    const auto macd_series = SubtractSeries{short_ema_series, long_ema_series};

    const auto macd = macd_series[index];

    const auto signal_series = EmaSeries{macd_series, signal_period_};
    const auto signal = signal_series[index];

    switch(output_) {
    case MacdOutput::signal:
      return signal;
    case MacdOutput::histogram:
      return macd - signal;
    case MacdOutput::macd:
    default:
      return macd;
    }
  }

  auto size() const noexcept -> std::size_t
  {
    return input_.size();
  }

  auto input() const noexcept
  {
    return input_;
  }

  auto output() const noexcept -> MacdOutput
  {
    return output_;
  }

  void output(MacdOutput output) noexcept
  {
    output_ = output;
  }

private:
  TSeries input_;
  MacdOutput output_;
  std::size_t short_period_;
  std::size_t long_period_;
  std::size_t signal_period_;
};

} // namespace pludux

#endif