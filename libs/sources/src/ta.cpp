#include <algorithm>
#include <cmath>
#include <tuple>
#include <vector>

#include <pludux/ta.hpp>

namespace pludux::ta {

auto changes(const Series& series) -> Series
{
  return Series{ta::changes(series.data())};
}

auto sma(const Series& series, std::size_t period) -> Series
{
  return Series{ta::sma(series.data(), period)};
}

auto ema(const Series& series, std::size_t period) -> Series
{
  return Series{ta::ema(series.data(), period)};
}

auto rma(const Series& series, std::size_t period) -> Series
{
  return Series{ta::rma(series.data(), period)};
}

auto wma(const Series& series, std::size_t period) -> Series
{
  return Series{ta::wma(series.data(), period)};
}

auto hma(const Series& series, std::size_t period) -> Series
{
  return Series{ta::hma(series.data(), period)};
}

auto rsi(const Series& series, std::size_t period) -> Series
{
  return Series{ta::rsi(series.data(), period)};
}

auto tr(const Series& highs, const Series& lows, const Series& closes) -> Series
{
  return Series{ta::tr(highs.data(), lows.data(), closes.data())};
}

auto atr(const Series& highs,
         const Series& lows,
         const Series& closes,
         std::size_t period) -> Series
{
  return Series{ta::atr(highs.data(), lows.data(), closes.data(), period)};
}

auto macd(const Series& series,
          std::size_t short_period,
          std::size_t long_period,
          std::size_t signal_period) -> std::tuple<Series, Series, Series>
{
  auto [macd_line, signal_line, histogram] =
   ta::macd(series.data(), short_period, long_period, signal_period);

  return {Series{macd_line}, Series{signal_line}, Series{histogram}};
}

auto stoch(const Series& highs,
           const Series& lows,
           const Series& closes,
           std::size_t k_period,
           std::size_t k_smooth,
           std::size_t d_period) -> std::pair<Series, Series>
{
  auto [k, d] = ta::stoch(
   highs.data(), lows.data(), closes.data(), k_period, k_smooth, d_period);

  return {Series{k}, Series{d}};
}

auto stoch_rsi(const Series& series,
               std::size_t rsi_period,
               std::size_t k_period,
               std::size_t k_smooth,
               std::size_t d_period) -> std::pair<Series, Series>
{
  auto [k, d] =
   ta::stoch_rsi(series.data(), rsi_period, k_period, k_smooth, d_period);

  return {Series{k}, Series{d}};
}

auto bb(const Series& series,
        std::size_t period,
        double stddev) -> std::tuple<Series, Series, Series>
{
  auto [sma, upper_band, lower_band] = ta::bb(series.data(), period, stddev);

  return {Series{sma}, Series{upper_band}, Series{lower_band}};
}

} // namespace pludux::ta
