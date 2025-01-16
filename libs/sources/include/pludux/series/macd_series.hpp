#ifndef PLUDUX_PLUDUX_SERIES_MACD_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_MACD_SERIES_HPP

#include <cstddef>
#include <limits>
#include <utility>

#include "binary_fn_series.hpp"
#include "ema_series.hpp"
#include "ref_series.hpp"

namespace pludux {

template<typename TSeries>
class MacdSeries {
public:
  using ValueType = typename TSeries::ValueType;

  MacdSeries(TSeries series,
                      std::size_t short_period,
                      std::size_t long_period)
  : series_{std::move(series)}
  , short_period_{short_period}
  , long_period_{long_period}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    const auto series = RefSeries{series_};
    const auto short_ema = EmaSeries{series, short_period_}[index];
    const auto long_ema = EmaSeries{series, long_period_}[index];

    return short_ema - long_ema;
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size();
  }

  auto signal(std::size_t signal_period) const noexcept -> EmaSeries<MacdSeries>
  {
    return EmaSeries{*this, signal_period};
  }

  auto histogram(std::size_t signal_period) const noexcept
   -> SubtractSeries<MacdSeries, EmaSeries<MacdSeries>>
  {
    return SubtractSeries{*this, signal(signal_period)};
  }

private:
  TSeries series_;
  std::size_t short_period_;
  std::size_t long_period_;
};

} // namespace pludux

#endif