#ifndef PLUDUX_PLUDUX_TA_HPP
#define PLUDUX_PLUDUX_TA_HPP

#include <algorithm>
#include <cmath>
#include <tuple>
#include <vector>

#include "quote.hpp"
#include "series.hpp"

namespace pludux::ta {

auto changes(const auto& series) -> std::vector<double>
{
  auto result = std::vector<double>{};
  result.reserve(series.size());

  result.push_back(std::numeric_limits<double>::quiet_NaN());

  for(std::size_t i = 1; i < series.size(); ++i) {
    const auto diff = series[i] - series[i - 1];
    result.push_back(diff);
  }

  return result;
}

auto changes(const Series& series) -> Series;

auto sma(const auto& series, std::size_t period) -> std::vector<double>
{
  auto result = std::vector<double>{};
  result.reserve(series.size());

  for(std::size_t i = 0; i < series.size(); ++i) {
    if(i < period - 1) {
      result.push_back(std::numeric_limits<double>::quiet_NaN());
      continue;
    }

    auto sum = 0.0;
    for(std::size_t j = i - period + 1; j <= i; ++j) {
      const auto value = static_cast<double>(series[j]);
      sum += series[j];
    }
    result.push_back(sum / period);
  }

  return result;
}

auto sma(const Series& series, std::size_t period) -> Series;

auto ema(const auto& series, std::size_t period) -> std::vector<double>
{
  const auto alpha = 2.0 / (period + 1);

  auto result = std::vector<double>{};
  result.reserve(series.size());

  for(std::size_t i = 0; i < series.size(); ++i) {
    if(i < period - 1) {
      result.push_back(std::numeric_limits<double>::quiet_NaN());
      continue;
    }

    if(std::isnan(result.back())) {
      auto sum = 0.0;
      for(std::size_t j = i - period + 1; j <= i; ++j) {
        const auto value = static_cast<double>(series[j]);
        sum += series[j];
      }
      result.push_back(sum / period);
      continue;
    }

    const auto value = static_cast<double>(series[i]);
    const auto prev_ema = result.back();
    const auto current_ema = alpha * value + (1 - alpha) * prev_ema;
    result.push_back(current_ema);
  }

  return result;
}

auto ema(const Series& series, std::size_t period) -> Series;

auto rma(const auto& series, std::size_t period) -> std::vector<double>
{
  const auto alpha = 1.0 / period;

  auto result = std::vector<double>{};
  result.reserve(series.size());

  for(std::size_t i = 0; i < series.size(); ++i) {
    if(i < period - 1) {
      result.push_back(std::numeric_limits<double>::quiet_NaN());
      continue;
    }

    if(std::isnan(result.back())) {
      auto sum = 0.0;
      for(std::size_t j = i - period + 1; j <= i; ++j) {
        const auto value = static_cast<double>(series[j]);
        sum += series[j];
      }
      result.push_back(sum / period);
      continue;
    }

    const auto value = static_cast<double>(series[i]);
    const auto prev_ema = result.back();
    const auto current_ema = alpha * value + (1 - alpha) * prev_ema;
    result.push_back(current_ema);
  }

  return result;
}

auto rma(const Series& series, std::size_t period) -> Series;

auto wma(const auto& series, std::size_t period) -> std::vector<double>
{
  auto result = std::vector<double>{};
  result.reserve(series.size());

  for(std::size_t i = result.size(); i < series.size(); ++i) {
    if(i < period - 1) {
      result.push_back(std::numeric_limits<double>::quiet_NaN());
      continue;
    }

    auto norm = 0.0;
    double sum = 0.0;
    for(std::size_t j = i - period + 1; j <= i; ++j) {
      const auto weight = (period - (i - j)) * period;
      const auto value = static_cast<double>(series[j]);
      norm += weight;
      sum += value * weight;
    }
    result.push_back(sum / norm);
  }

  return result;
}

auto wma(const Series& series, std::size_t period) -> Series;

auto hma(const auto& series, std::size_t period) -> std::vector<double>
{
  auto wma1 = ta::wma(series, period / 2);
  std::transform(wma1.begin(), wma1.end(), wma1.begin(), [](auto value) {
    return 2 * value;
  });

  const auto wma2 = ta::wma(series, period);

  auto diff = std::vector<double>{};
  diff.reserve(wma1.size());
  std::transform(wma1.cbegin(),
                 wma1.cend(),
                 wma2.cbegin(),
                 std::back_inserter(diff),
                 std::minus<double>{});

  const auto hma = ta::wma(diff, static_cast<std::size_t>(sqrt(period)));

  return hma;
}

auto hma(const Series& series, std::size_t period) -> Series;

auto rsi(const auto& series, std::size_t period) -> std::vector<double>
{
  auto result = std::vector<double>{};
  result.reserve(series.size());

  const auto changes = ta::changes(series);

  auto gains = std::vector<double>{};
  gains.reserve(series.size());

  auto losses = std::vector<double>{};
  losses.reserve(series.size());

  for(std::size_t i = 0; i < changes.size(); ++i) {
    const auto change = static_cast<double>(changes[i]);

    if(std::isnan(change)) {
      gains.push_back(std::numeric_limits<double>::quiet_NaN());
      losses.push_back(std::numeric_limits<double>::quiet_NaN());
    } else if(change > 0) {
      gains.push_back(change);
      losses.push_back(0.0);
    } else {
      gains.push_back(0.0);
      losses.push_back(-change);
    }
  }

  const auto avg_gain = ta::rma(gains, period);
  const auto avg_loss = ta::rma(losses, period);

  for(std::size_t i = 0; i < series.size(); ++i) {
    if(i < period) {
      result.push_back(std::numeric_limits<double>::quiet_NaN());
    } else {
      const auto rs = avg_gain[i] / avg_loss[i];
      const auto rsi = 100 - (100 / (1 + rs));
      result.push_back(rsi);
    }
  }

  return result;
}

auto rsi(const Series& series, std::size_t period) -> Series;

auto tr(const auto& quotes) -> std::vector<double>
{
  auto result = std::vector<double>{};
  result.reserve(quotes.size());

  for(std::size_t i = 0; i < quotes.size(); ++i) {
    const auto quote = quotes[i];
    const auto prev_quote = i == 0 ? quote : quotes[i - 1];

    const auto high = static_cast<double>(quote.high());
    const auto low = static_cast<double>(quote.low());
    const auto prev_close = static_cast<double>(prev_quote.close());

    const auto hl = std::abs(high - low);
    const auto hc = std::abs(high - prev_close);
    const auto lc = std::abs(low - prev_close);

    const auto true_range = std::max(std::max(hl, hc), lc);
    result.push_back(true_range);
  }

  return result;
}

auto tr(const auto& highs,
        const auto& lows,
        const auto& closes) -> std::vector<double>
{
  auto result = std::vector<double>{};
  result.reserve(highs.size());

  for(std::size_t i = 0; i < highs.size(); ++i) {
    const auto high = static_cast<double>(highs[i]);
    const auto low = static_cast<double>(lows[i]);
    const auto prev_close =
     static_cast<double>(i == 0 ? closes[i] : closes[i - 1]);

    const auto hl = std::abs(high - low);
    const auto hc = std::abs(high - prev_close);
    const auto lc = std::abs(low - prev_close);

    const auto true_range = std::max(std::max(hl, hc), lc);
    result.push_back(true_range);
  }

  return result;
}

auto tr(const Series& highs,
        const Series& lows,
        const Series& closes) -> Series;

auto atr(const auto& quotes, std::size_t period) -> std::vector<double>
{
  const auto trs = ta::tr(quotes);
  const auto atr = ta::rma(trs, period);

  return atr;
}

auto atr(const auto& highs,
         const auto& lows,
         const auto& closes,
         std::size_t period) -> std::vector<double>
{
  const auto trs = ta::tr(highs, lows, closes);
  const auto atr = ta::rma(trs, period);

  return atr;
}

auto atr(const Series& highs,
         const Series& lows,
         const Series& closes,
         std::size_t period) -> Series;

auto macd(const auto& series,
          std::size_t short_period,
          std::size_t long_period,
          std::size_t signal_period)
 -> std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
{
  const auto short_ema = ta::ema(series, short_period);
  const auto long_ema = ta::ema(series, long_period);

  auto macd_line = std::vector<double>{};
  macd_line.reserve(series.size());
  std::transform(short_ema.cbegin(),
                 short_ema.cend(),
                 long_ema.cbegin(),
                 std::back_inserter(macd_line),
                 std::minus<double>{});

  const auto signal_line = ta::ema(macd_line, signal_period);

  auto histogram = std::vector<double>{};
  histogram.reserve(series.size());
  std::transform(macd_line.cbegin(),
                 macd_line.cend(),
                 signal_line.cbegin(),
                 std::back_inserter(histogram),
                 std::minus<double>{});

  return {macd_line, signal_line, histogram};
}

auto macd(const Series& series,
          std::size_t short_period,
          std::size_t long_period,
          std::size_t signal_period) -> std::tuple<Series, Series, Series>;

auto stoch(const auto& quotes,
           std::size_t k_period,
           std::size_t k_smooth,
           std::size_t d_period)
 -> std::pair<std::vector<double>, std::vector<double>>
{
  auto stochastic = std::vector<double>{};
  stochastic.reserve(quotes.size());
  for(std::size_t i = 0; i < quotes.size(); ++i) {
    if(i < k_period - 1) {
      stochastic.push_back(std::numeric_limits<double>::quiet_NaN());
    } else {
      const auto high = static_cast<double>(quotes[i].high());
      const auto low = static_cast<double>(quotes[i].low());
      const auto close = static_cast<double>(quotes[i].close());

      auto highest_high = std::numeric_limits<double>::min();
      auto lowest_low = std::numeric_limits<double>::max();

      for(std::size_t j = i - k_period + 1; j <= i; ++j) {
        const auto quote = quotes[j];
        const auto quote_high = static_cast<double>(quote.high());
        const auto quote_low = static_cast<double>(quote.low());

        highest_high = std::max(highest_high, quote_high);
        lowest_low = std::min(lowest_low, quote_low);
      }

      const auto stoch =
       100 * (close - lowest_low) / (highest_high - lowest_low);
      stochastic.push_back(stoch);
    }
  }

  const auto k = ta::sma(stochastic, k_smooth);
  const auto d = ta::sma(k, d_period);

  return {k, d};
}

auto stoch(const auto& highs,
           const auto& lows,
           const auto& closes,
           std::size_t k_period,
           std::size_t k_smooth,
           std::size_t d_period)
 -> std::pair<std::vector<double>, std::vector<double>>
{
  auto stochastic = std::vector<double>{};
  stochastic.reserve(highs.size());
  for(std::size_t i = 0; i < highs.size(); ++i) {
    if(i < k_period - 1) {
      stochastic.push_back(std::numeric_limits<double>::quiet_NaN());
    } else {
      const auto high = static_cast<double>(highs[i]);
      const auto low = static_cast<double>(lows[i]);
      const auto close = static_cast<double>(closes[i]);

      auto highest_high = std::numeric_limits<double>::min();
      auto lowest_low = std::numeric_limits<double>::max();

      for(std::size_t j = i - k_period + 1; j <= i; ++j) {
        const auto quote_high = static_cast<double>(highs[j]);
        const auto quote_low = static_cast<double>(lows[j]);

        highest_high = std::max(highest_high, quote_high);
        lowest_low = std::min(lowest_low, quote_low);
      }

      const auto stoch =
       100 * (close - lowest_low) / (highest_high - lowest_low);
      stochastic.push_back(stoch);
    }
  }

  const auto k = ta::sma(stochastic, k_smooth);
  const auto d = ta::sma(k, d_period);

  return {k, d};
}

auto stoch(const Series& highs,
           const Series& lows,
           const Series& closes,
           std::size_t k_period,
           std::size_t k_smooth,
           std::size_t d_period) -> std::pair<Series, Series>;

auto stoch_rsi(const auto& series,
               std::size_t rsi_period,
               std::size_t k_period,
               std::size_t k_smooth,
               std::size_t d_period)
 -> std::pair<std::vector<double>, std::vector<double>>
{
  const auto rsi_values = ta::rsi(series, rsi_period);

  auto stoch_rsi = std::vector<double>{};
  stoch_rsi.reserve(series.size());

  for(std::size_t i = 0; i < series.size(); ++i) {
    if(i < k_period - 1) {
      stoch_rsi.push_back(std::numeric_limits<double>::quiet_NaN());
    } else {
      const auto close = rsi_values[i];

      auto highest_high = std::numeric_limits<double>::min();
      auto lowest_low = std::numeric_limits<double>::max();

      for(std::size_t j = i - k_period + 1; j <= i; ++j) {
        const auto close_j = rsi_values[j];

        highest_high = std::max(highest_high, close_j);
        lowest_low = std::min(lowest_low, close_j);
      }

      const auto stoch =
       100 * (close - lowest_low) / (highest_high - lowest_low);
      stoch_rsi.push_back(stoch);
    }
  }

  const auto k = ta::sma(stoch_rsi, k_smooth);
  const auto d = ta::sma(k, d_period);

  return {k, d};
}

auto stoch_rsi(const Series& series,
               std::size_t rsi_period,
               std::size_t k_period,
               std::size_t k_smooth,
               std::size_t d_period) -> std::pair<Series, Series>;

auto bb(const auto& series, std::size_t period, double stddev)
 -> std::tuple<std::vector<double>, std::vector<double>, std::vector<double>>
{
  auto sma = std::vector<double>{};
  sma.reserve(series.size());

  auto upper_band = std::vector<double>{};
  upper_band.reserve(series.size());

  auto lower_band = std::vector<double>{};
  lower_band.reserve(series.size());

  for(std::size_t i = 0; i < series.size(); ++i) {
    if(i < period - 1) {
      sma.push_back(std::numeric_limits<double>::quiet_NaN());
      upper_band.push_back(std::numeric_limits<double>::quiet_NaN());
      lower_band.push_back(std::numeric_limits<double>::quiet_NaN());
      continue;
    }

    auto sum = 0.0;
    for(std::size_t j = i - period + 1; j <= i; ++j) {
      const auto value = static_cast<double>(series[j]);
      sum += value;
    }
    const auto current_sma = sum / period;
    sma.push_back(current_sma);

    auto sum_squared_diff = 0.0;
    for(std::size_t j = i - period + 1; j <= i; ++j) {
      const auto value = static_cast<double>(series[j]);
      const auto diff = value - current_sma;
      sum_squared_diff += diff * diff;
    }
    const auto std_dev = sqrt(sum_squared_diff / period);
    upper_band.push_back(current_sma + stddev * std_dev);
    lower_band.push_back(current_sma - stddev * std_dev);
  }

  return {sma, upper_band, lower_band};
}

auto bb(const Series& series,
        std::size_t period,
        double stddev) -> std::tuple<Series, Series, Series>;

} // namespace pludux::ta

#endif
