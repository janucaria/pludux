#ifndef PLUDUX_PLUDUX_TA_HPP
#define PLUDUX_PLUDUX_TA_HPP

#include <utility>

#include "series.hpp"

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
auto tr(TSeries highs,
        TSeries lows,
        TSeries closes) -> TrSeries<TSeries, TSeries, TSeries>
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
          std::size_t long_period) -> MacdSeries<TSeries>
{
  return MacdSeries{std::move(series), short_period, long_period};
}

template<typename TSeries>
auto stoch(TSeries highs, TSeries lows, TSeries closes, std::size_t k_period)
 -> StochSeries<TSeries, TSeries, TSeries>
{
  return StochSeries{
   std::move(highs), std::move(lows), std::move(closes), k_period};
}

template<typename TSeries>
auto stoch_rsi(TSeries series,
               std::size_t rsi_period,
               std::size_t k_period) -> StochRsiSeries<TSeries>
{
  return StochRsiSeries{std::move(series), rsi_period, k_period};
}

template<typename TSeries>
auto bb(TSeries series, std::size_t period, double stddev) -> BbSeries<TSeries>
{
  return BbSeries{std::move(series), period, stddev};
}

} // namespace pludux::ta

#endif
