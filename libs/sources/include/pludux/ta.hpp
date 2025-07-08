#ifndef PLUDUX_PLUDUX_TA_HPP
#define PLUDUX_PLUDUX_TA_HPP

import pludux.series;

#include <utility>

namespace pludux::ta {

template<typename TSeries>
auto changes(TSeries series) -> ChangeSeries<TSeries>
{
  return ChangeSeries{std::move(series)};
}

template<typename TSeries>
auto sma(TSeries series, std::size_t period) -> SmaSeries<TSeries>
{
  return SmaSeries{std::move(series), period};
}

template<typename TSeries>
auto ema(TSeries series, std::size_t period) -> EmaSeries<TSeries>
{
  return EmaSeries{std::move(series), period};
}

template<typename TSeries>
auto rma(TSeries series, std::size_t period) -> RmaSeries<TSeries>
{
  return RmaSeries{std::move(series), period};
}

template<typename TSeries>
auto wma(TSeries series, std::size_t period) -> WmaSeries<TSeries>
{
  return WmaSeries{std::move(series), period};
}

template<typename TSeries>
auto hma(TSeries series, std::size_t period) -> HmaSeries<TSeries>
{
  return HmaSeries{std::move(series), period};
}

template<typename TSeries>
auto rsi(TSeries series, std::size_t period) -> RsiSeries<TSeries>
{
  return RsiSeries{std::move(series), period};
}

template<typename TSeries>
auto tr(TSeries highs, TSeries lows, TSeries closes)
 -> TrSeries<TSeries, TSeries, TSeries>
{
  return TrSeries{std::move(highs), std::move(lows), std::move(closes)};
}

template<typename TSeries>
auto atr(TSeries highs, TSeries lows, TSeries closes, std::size_t period)
 -> AtrSeries<TSeries, TSeries, TSeries>
{
  return AtrSeries{
   std::move(highs), std::move(lows), std::move(closes), period};
}

template<typename TSeries>
auto macd(TSeries series,
          std::size_t short_period,
          std::size_t long_period,
          std::size_t signal_period) -> MacdSeries<TSeries>
{
  return MacdSeries{MacdOutput::macd,
                    std::move(series),
                    short_period,
                    long_period,
                    signal_period};
}

template<typename TSeries>
auto stoch(TSeries highs,
           TSeries lows,
           TSeries closes,
           std::size_t k_period,
           std::size_t k_smooth,
           std::size_t d_period) -> StochSeries<TSeries, TSeries, TSeries>
{
  return StochSeries{StochOutput::k,
                     std::move(highs),
                     std::move(lows),
                     std::move(closes),
                     k_period,
                     k_smooth,
                     d_period};
}

template<typename TSeries>
auto stoch_rsi(TSeries series,
               std::size_t rsi_period,
               std::size_t k_period,
               std::size_t k_smooth,
               std::size_t d_period) -> StochRsiSeries<TSeries>
{
  return StochRsiSeries{
   StochOutput::k, std::move(series), rsi_period, k_period, k_smooth, d_period};
}

template<typename TSeries>
auto bb(TSeries series, std::size_t period, double stddev)
 -> BbSeries<SmaSeries<TSeries>>
{
  return BbSeries{BbOutput::middle, std::move(series), period, stddev};
}

template<typename TSeries>
auto pivot_lows(TSeries series, std::size_t range) -> PivotLowsSeries<TSeries>
{
  return PivotLowsSeries{std::move(series), range};
}

} // namespace pludux::ta

#endif
