module;

#include <algorithm>
#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

export module pludux:evaluate_series_method;

import :asset_snapshot;
import :series_output;
import :method_contextable;

// Import all method modules for type/template visibility
import :series.atr_method;
import :series.bb_method;
import :series.change_method;
import :series.data_method;
import :series.percentage_method;
import :series.roc_method;
import :series.rvol_method;
import :series.rsi_method;
import :series.rma_method;
import :series.ema_method;
import :series.highest_method;
import :series.hma_method;
import :series.kc_method;
import :series.lookback_method;
import :series.lowest_method;
import :series.macd_method;
import :series.ohlcv_method;
import :series.operators_method;
import :series.adaptive_ma_method;
import :series.sma_method;
import :series.wma_method;
import :series.rma_method;
import :series.select_output_method;
import :series.series_node_method;
import :series.series_value_method;
import :series.stddev_method;
import :series.stoch_method;
import :series.stoch_rsi_method;
import :series.tr_method;
import :series.value_method;
import :series.wma_method;

export namespace pludux {

template<typename TMethod>
auto evaluate_selected_output_series_or_nan(
 const TMethod& method,
 SeriesOutput output,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
  requires requires {
    {
      evaluate_series_method(method, asset_snapshot, context)
    } -> std::convertible_to<double>;
  }
{
  if constexpr(requires {
                 {
                   evaluate_series_method(
                    output, method, asset_snapshot, context)
                 } -> std::convertible_to<double>;
               }) {
    return evaluate_series_method(output, method, asset_snapshot, context);
  } else {
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// ATR
auto evaluate_series_method(const AtrMethod& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto ma_method =
   AdaptiveMaMethod{TrMethod{}, method.ma_smoothing_type(), method.period()};

  return evaluate_series_method(ma_method, std::move(asset_snapshot), context);
}

// TR
auto evaluate_series_method(const TrMethod& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto high =
   evaluate_series_method(HighMethod{}, asset_snapshot, context);
  const auto low = evaluate_series_method(LowMethod{}, asset_snapshot, context);

  const auto close_last_index = asset_snapshot.size() - 1;
  const auto prev_close =
   close_last_index == 0
    ? evaluate_series_method(CloseMethod{}, asset_snapshot, context)
    : evaluate_series_method(CloseMethod{}, asset_snapshot[1], context);

  const auto hl = std::abs(high - low);
  const auto hc = std::abs(high - prev_close);
  const auto lc = std::abs(low - prev_close);

  return std::max(std::max(hl, hc), lc);
}

// BB
template<typename TMaSourceMethod>
auto evaluate_series_method(const BbMethod<TMaSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   SeriesOutput::MiddleBand, method, std::move(asset_snapshot), context);
}

template<typename TMaSourceMethod>
auto evaluate_series_method(SeriesOutput output,
                            const BbMethod<TMaSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto ma_source = method.source();
  const auto ma_period = method.period();
  const auto ma_method =
   AdaptiveMaMethod{ma_source, method.ma_method_type(), ma_period};
  const auto middle =
   evaluate_series_method(ma_method, asset_snapshot, context);

  const auto stddev_method = StddevMethod{ma_source, ma_period};
  const auto std_dev =
   evaluate_series_method(stddev_method, asset_snapshot, context);
  const auto std_dev_scaled = std_dev * method.stddev();
  switch(output) {
  case SeriesOutput::MiddleBand:
    return middle;
  case SeriesOutput::UpperBand:
    return middle + std_dev_scaled;
  case SeriesOutput::LowerBand:
    return middle - std_dev_scaled;
  default:
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// Change
template<typename TSourceMethod>
auto evaluate_series_method(const ChangeMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto current =
   evaluate_series_method(method.source(), asset_snapshot, context);
  const auto previous =
   evaluate_series_method(method.source(), asset_snapshot[1], context);

  return current - previous;
}

// Data
auto evaluate_series_method(const DataMethod& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return asset_snapshot.data(method.field());
}

// Percentage
template<typename TMethod>
auto evaluate_series_method(const PercentageMethod<TMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto total =
   evaluate_series_method(method.base(), asset_snapshot, context);
  const auto percentage = total * (method.percent() / 100.0);
  return percentage;
}

// SMA
template<typename TSourceMethod>
auto evaluate_series_method(const SmaMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto asset_size = asset_snapshot.size();
  if(asset_size < method.period()) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto sum = 0.0;
  for(auto i = 0uz; i < method.period(); ++i) {
    sum += evaluate_series_method(method.source(), asset_snapshot[i], context);
  }

  return sum / method.period();
}

// EMA RMA
template<template<typename> typename TEmaMethod, typename TSourceMethod>
  requires(std::same_as<std::remove_cvref_t<TEmaMethod<TSourceMethod>>,
                        EmaMethod<TSourceMethod>> ||
           std::same_as<std::remove_cvref_t<TEmaMethod<TSourceMethod>>,
                        RmaMethod<TSourceMethod>>)
auto evaluate_series_method(const TEmaMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  auto empty_results = std::vector<double>{};

  auto& cached_results = [&]() mutable -> std::vector<double>& {
    if constexpr(!std::is_same_v<std::remove_cvref_t<decltype(context)>,
                                 std::monostate>) {
      return context.get_series_results(MethodKey{method});
    } else {
      return empty_results;
    }
  }();

  const auto results_size = static_cast<std::ptrdiff_t>(cached_results.size());
  const auto snapshot_size = static_cast<std::ptrdiff_t>(asset_snapshot.size());

  if(snapshot_size > 0 && snapshot_size <= results_size) {
    const auto cached_index = snapshot_size - 1;
    return cached_results[cached_index];
  }

  const auto results_to_compute = snapshot_size - results_size;
  for(auto ii = results_to_compute; ii > 1; --ii) {
    const auto i = ii - 1;
    const auto result =
     evaluate_series_method(cached_results, method, asset_snapshot[i], context);
    cached_results.push_back(result);
  }

  return evaluate_series_method(
   cached_results, method, std::move(asset_snapshot), context);
}

template<template<typename> typename TEmaMethod, typename TSourceMethod>
  requires(std::same_as<std::remove_cvref_t<TEmaMethod<TSourceMethod>>,
                        EmaMethod<TSourceMethod>> ||
           std::same_as<std::remove_cvref_t<TEmaMethod<TSourceMethod>>,
                        RmaMethod<TSourceMethod>>)
auto evaluate_series_method(std::vector<double>& cached_results,
                            const TEmaMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto& source_method = method.source();
  const auto period = method.period();

  const auto prev_result = cached_results.empty()
                            ? std::numeric_limits<double>::quiet_NaN()
                            : cached_results.back();
  if(std::isnan(prev_result)) {
    return evaluate_series_method(
     SmaMethod{source_method, period}, asset_snapshot, context);
  }

  const auto alpha =
   std::is_same_v<std::remove_cvref_t<TEmaMethod<TSourceMethod>>,
                  EmaMethod<TSourceMethod>>
    ? 2.0 / (period + 1)
    : 1.0 / period;

  const auto source =
   evaluate_series_method(source_method, asset_snapshot, context);

  const auto result = alpha * source + (1 - alpha) * prev_result;

  return result;
}

// WMA
template<typename TSourceMethod>
auto evaluate_series_method(const WmaMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto asset_size = asset_snapshot.size();
  if(asset_size < method.period()) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto norm = 0.0;
  auto sum = 0.0;
  for(auto i = 0uz; i < method.period(); ++i) {
    const auto weight = (method.period() - i) * method.period();
    sum += evaluate_series_method(method.source(), asset_snapshot[i], context) *
           weight;
    norm += weight;
  }

  return sum / norm;
}

// HMA
template<typename TSourceMethod>
auto evaluate_series_method(const HmaMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto wam1 = WmaMethod{method.source(), method.period() / 2};
  const auto scalar_2_method = ValueMethod{2.0};
  const auto times_2_wam1 = MultiplyMethod{scalar_2_method, wam1};

  const auto wam2 = WmaMethod{method.source(), method.period()};
  const auto diff = SubtractMethod{times_2_wam1, wam2};

  const auto hma =
   WmaMethod{diff, static_cast<std::size_t>(std::sqrt(method.period()))};

  return evaluate_series_method(hma, asset_snapshot, context);
}

// RSI
template<typename TSourceMethod>
auto evaluate_series_method(const RsiMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto rs_method =
   DivideMethod{RmaMethod{PositivePartMethod{ChangeMethod{method.source()}},
                          method.period()},
                RmaMethod{NegativePartMethod{ChangeMethod{method.source()}},
                          method.period()}};

  const auto rs = evaluate_series_method(rs_method, asset_snapshot, context);
  const auto rsi = 100 - (100 / (1 + rs));

  return rsi;
}

// ROC
template<typename TSourceMethod>
auto evaluate_series_method(const RocMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto source_size = asset_snapshot.size();
  if(source_size < method.period()) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  const auto current =
   evaluate_series_method(method.source(), asset_snapshot, context);
  const auto end = evaluate_series_method(
   method.source(), asset_snapshot[method.period()], context);

  return 100.0 * (current - end) / end;
}

// RVOL
auto evaluate_series_method(const RvolMethod& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto volume = VolumeMethod{};
  const auto sma_volume = SmaMethod{VolumeMethod{}, method.period()};
  const auto rvol = DivideMethod{volume, sma_volume};
  const auto rvol_result =
   evaluate_series_method(rvol, asset_snapshot, context);
  return rvol_result;
}

// Highest
template<typename TSourceMethod>
auto evaluate_series_method(const HighestMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  if(asset_snapshot.size() < method.period()) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto highest = std::numeric_limits<double>::min();
  for(auto i = 0uz; i < method.period(); ++i) {
    const auto value =
     evaluate_series_method(method.source(), asset_snapshot[i], context);
    highest = std::max(highest, value);
  }
  return highest;
}

// KC
template<typename TMaSourceMethod>
auto evaluate_series_method(const KcMethod<TMaSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   SeriesOutput::MiddleBand, method, std::move(asset_snapshot), context);
}

template<typename TMaSourceMethod>
auto evaluate_series_method(SeriesOutput output,
                            const KcMethod<TMaSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto band_range = [&]() -> double {
    switch(method.band_method_type()) {
    case KcBandMethodType::Atr: {
      const auto atr_method = AtrMethod{method.band_atr_period()};
      return evaluate_series_method(atr_method, asset_snapshot, context);
    }
    case KcBandMethodType::Tr: {
      const auto tr_method = TrMethod{};
      return evaluate_series_method(tr_method, asset_snapshot, context);
    }
    case KcBandMethodType::RangeHighLow: {
      const auto range_method = SubtractMethod{HighMethod{}, LowMethod{}};
      return evaluate_series_method(range_method, asset_snapshot, context);
    }
    default:
      return std::numeric_limits<double>::quiet_NaN();
    }
  };

  const auto ma_method =
   AdaptiveMaMethod{method.source(), method.ma_method_type(), method.period()};
  const auto middle =
   evaluate_series_method(ma_method, asset_snapshot, context);

  switch(output) {
  case SeriesOutput::MiddleBand:
    return middle;
  case SeriesOutput::UpperBand:
    return middle + (method.multiplier() * band_range());
  case SeriesOutput::LowerBand:
    return middle - (method.multiplier() * band_range());
  default:
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// Lookback

template<typename TSourceMethod>
auto evaluate_series_method(const LookbackMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   method.source(), asset_snapshot[method.period()], context);
}

// SelectOutput
template<typename TSourceMethod>
auto evaluate_series_method(const SelectOutputMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   method.output(), method, asset_snapshot, context);
}

template<typename TSourceMethod>
auto evaluate_series_method(SeriesOutput output,
                            const SelectOutputMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return evaluate_selected_output_series_or_nan(
   method.source(), output, asset_snapshot, context);
}

// Lowest

template<typename TSourceMethod>
auto evaluate_series_method(const LowestMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  if(asset_snapshot.size() < method.period()) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto lowest = std::numeric_limits<double>::max();
  for(auto i = 0uz; i < method.period(); ++i) {
    const auto value =
     evaluate_series_method(method.source(), asset_snapshot[i], context);
    lowest = std::min(lowest, value);
  }
  return lowest;
}

// MACD

template<typename TSourceMethod>
auto evaluate_series_method(const MacdMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   SeriesOutput::MacdLine, method, asset_snapshot, context);
}

template<typename TSourceMethod>
auto evaluate_series_method(SeriesOutput output,
                            const MacdMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto macd_method =
   SubtractMethod{EmaMethod{method.source(), method.short_period()},
                  EmaMethod{method.source(), method.long_period()}};
  const auto signal_ema = EmaMethod{macd_method, method.signal_period()};

  const auto macd =
   evaluate_series_method(macd_method, asset_snapshot, context);
  const auto signal =
   evaluate_series_method(signal_ema, asset_snapshot, context);
  const auto histogram = macd - signal;

  switch(output) {
  case SeriesOutput::MacdLine:
    return macd;
  case SeriesOutput::SignalLine:
    return signal;
  case SeriesOutput::Histogram:
    return histogram;
  default:
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// OhlcvMethod (Open, High, Low, Close, Volume)

auto evaluate_series_method(OpenMethod method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return asset_snapshot.open();
}

auto evaluate_series_method(HighMethod method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return asset_snapshot.high();
}

auto evaluate_series_method(LowMethod method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return asset_snapshot.low();
}

auto evaluate_series_method(CloseMethod method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return asset_snapshot.close();
}

auto evaluate_series_method(VolumeMethod method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return asset_snapshot.volume();
}

// Operators (Multiply, Divide, Add, Subtract, Negate, Abs, AbsDiff, Sqrt, Max,
// Min, PositivePart, NegativePart)

template<typename TMethodOp1, typename TMethodOp2>
auto evaluate_series_method(
 const MultiplyMethod<TMethodOp1, TMethodOp2>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto multiplicand_result =
   evaluate_series_method(method.multiplicand(), asset_snapshot, context);
  const auto multiplier_result =
   evaluate_series_method(method.multiplier(), asset_snapshot, context);

  return std::multiplies<>{}(multiplicand_result, multiplier_result);
}

template<typename TMethodOp1, typename TMethodOp2>
auto evaluate_series_method(const DivideMethod<TMethodOp1, TMethodOp2>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto dividend_result =
   evaluate_series_method(method.dividend(), asset_snapshot, context);
  const auto divisor_result =
   evaluate_series_method(method.divisor(), asset_snapshot, context);

  return std::divides<>{}(dividend_result, divisor_result);
}

template<typename TMethodOp1, typename TMethodOp2>
auto evaluate_series_method(const AddMethod<TMethodOp1, TMethodOp2>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto augend_result =
   evaluate_series_method(method.augend(), asset_snapshot, context);
  const auto addend_result =
   evaluate_series_method(method.addend(), asset_snapshot, context);

  return std::plus<>{}(augend_result, addend_result);
}

template<typename TMethodOp1, typename TMethodOp2>
auto evaluate_series_method(
 const SubtractMethod<TMethodOp1, TMethodOp2>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto minuend_result =
   evaluate_series_method(method.minuend(), asset_snapshot, context);
  const auto subtrahend_result =
   evaluate_series_method(method.subtrahend(), asset_snapshot, context);

  return std::minus<>{}(minuend_result, subtrahend_result);
}

template<typename TMethodOp>
auto evaluate_series_method(const NegateMethod<TMethodOp>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto operand_result =
   evaluate_series_method(method.operand(), asset_snapshot, context);

  return std::negate<>{}(operand_result);
}

template<typename TMethodOp>
auto evaluate_series_method(const AbsMethod<TMethodOp>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto operand_result =
   evaluate_series_method(method.operand(), asset_snapshot, context);

  return Absolute<>{}(operand_result);
}

template<typename TMethodOp1, typename TMethodOp2>
auto evaluate_series_method(const AbsDiffMethod<TMethodOp1, TMethodOp2>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto left_result =
   evaluate_series_method(method.left(), asset_snapshot, context);
  const auto right_result =
   evaluate_series_method(method.right(), asset_snapshot, context);

  return AbsoluteDifference<>{}(left_result, right_result);
}

template<typename TMethodOp>
auto evaluate_series_method(const SqrtMethod<TMethodOp>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto operand_result =
   evaluate_series_method(method.operand(), asset_snapshot, context);

  return SquareRoot<>{}(operand_result);
}

template<typename TMethodOp1, typename TMethodOp2>
auto evaluate_series_method(const MaxMethod<TMethodOp1, TMethodOp2>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto left_result =
   evaluate_series_method(method.left(), asset_snapshot, context);
  const auto right_result =
   evaluate_series_method(method.right(), asset_snapshot, context);

  return Maximum<>{}(left_result, right_result);
}

template<typename TMethodOp1, typename TMethodOp2>
auto evaluate_series_method(const MinMethod<TMethodOp1, TMethodOp2>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto left_result =
   evaluate_series_method(method.left(), asset_snapshot, context);
  const auto right_result =
   evaluate_series_method(method.right(), asset_snapshot, context);

  return Minimum<>{}(left_result, right_result);
}

template<typename TMethodOp>
auto evaluate_series_method(const PositivePartMethod<TMethodOp>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto operand_result =
   evaluate_series_method(method.operand(), asset_snapshot, context);

  return PositivePart<>{}(operand_result);
}

template<typename TMethodOp>
auto evaluate_series_method(const NegativePartMethod<TMethodOp>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto operand_result =
   evaluate_series_method(method.operand(), asset_snapshot, context);

  return NegativePart<>{}(operand_result);
}

// Adaptive MA

template<typename TSourceMethod>
auto evaluate_series_method(const AdaptiveMaMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  switch(method.ma_type()) {
  case MaMethodType::Sma: {
    const auto sma = SmaMethod{method.source(), method.period()};
    return evaluate_series_method(sma, asset_snapshot, context);
  }
  case MaMethodType::Ema: {
    const auto ema = EmaMethod{method.source(), method.period()};
    return evaluate_series_method(ema, asset_snapshot, context);
  }
  case MaMethodType::Wma: {
    const auto wma = WmaMethod{method.source(), method.period()};
    return evaluate_series_method(wma, asset_snapshot, context);
  }
  case MaMethodType::Rma: {
    const auto rma = RmaMethod{method.source(), method.period()};
    return evaluate_series_method(rma, asset_snapshot, context);
  }
  case MaMethodType::Hma: {
    const auto hma = HmaMethod{method.source(), method.period()};
    return evaluate_series_method(hma, asset_snapshot, context);
  }
  default:
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// SeriesNode

auto evaluate_series_method(const SeriesNodeMethod& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
    return std::numeric_limits<double>::quiet_NaN();
  } else {
    return context.call_series_method(method.name(), asset_snapshot);
  }
}

auto evaluate_series_method(SeriesOutput output,
                            const SeriesNodeMethod& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
    return std::numeric_limits<double>::quiet_NaN();
  } else {
    return context.call_series_method(method.name(), asset_snapshot, output);
  }
}

// SeriesValue

auto evaluate_series_method(const SeriesValueMethod& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  const auto result_index = asset_snapshot.index();
  return context.get_series_result(method.name(), result_index);
}

// Stddev

template<typename TSourceMethod>
auto evaluate_series_method(const StddevMethod<TSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto asset_size = asset_snapshot.size();
  if(asset_size < method.period()) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto sum = 0.0;
  for(auto i = 0uz; i < method.period(); ++i) {
    sum += evaluate_series_method(method.source(), asset_snapshot[i], context);
  }
  const auto mean = sum / method.period();

  auto sum_squared_diff = 0.0;
  for(auto i = 0uz; i < method.period(); ++i) {
    const auto diff =
     evaluate_series_method(method.source(), asset_snapshot[i], context) - mean;
    sum_squared_diff += diff * diff;
  }

  const auto variance = sum_squared_diff / method.period();
  const auto stddev = std::sqrt(variance);
  return stddev;
}

// Stochastic
auto evaluate_series_method(const StochMethod& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   SeriesOutput::KPercent, method, asset_snapshot, context);
}

auto evaluate_series_method(SeriesOutput output,
                            const StochMethod& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto close = CloseMethod{};
  const auto highest_high = HighestMethod{HighMethod{}, method.k_period()};
  const auto lowest_low = LowestMethod{LowMethod{}, method.k_period()};
  const auto stoch = DivideMethod{MultiplyMethod{ValueMethod{100},
                                                 SubtractMethod{
                                                  close,
                                                  lowest_low,
                                                 }},
                                  SubtractMethod{
                                   highest_high,
                                   lowest_low,
                                  }};

  const auto k_percent = SmaMethod{stoch, method.k_smooth()};

  switch(output) {
  case SeriesOutput::KPercent:
    return evaluate_series_method(k_percent, asset_snapshot, context);
  case SeriesOutput::DPercent:
    return evaluate_series_method(
     SmaMethod{k_percent, method.d_period()}, asset_snapshot, context);
  default:
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// Stochastic RSI
template<typename TRsiSourceMethod>
auto evaluate_series_method(const StochRsiMethod<TRsiSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   SeriesOutput::KPercent, method, std::move(asset_snapshot), context);
}

template<typename TRsiSourceMethod>
auto evaluate_series_method(SeriesOutput output,
                            const StochRsiMethod<TRsiSourceMethod>& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  const auto rsi = RsiMethod{method.rsi_source(), method.rsi_period()};
  const auto highest_rsi = HighestMethod{rsi, method.k_period()};
  const auto lowest_rsi = LowestMethod{rsi, method.k_period()};
  const auto stoch = DivideMethod{MultiplyMethod{ValueMethod{100},
                                                 SubtractMethod{
                                                  rsi,
                                                  lowest_rsi,
                                                 }},
                                  SubtractMethod{
                                   highest_rsi,
                                   lowest_rsi,
                                  }};

  const auto k_percent = SmaMethod{stoch, method.k_smooth()};

  switch(output) {
  case SeriesOutput::KPercent:
    return evaluate_series_method(k_percent, asset_snapshot, context);
  case SeriesOutput::DPercent:
    return evaluate_series_method(
     SmaMethod{k_percent, method.d_period()}, asset_snapshot, context);
  default:
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// Value
auto evaluate_series_method(const ValueMethod& method,
                            AssetSnapshot asset_snapshot,
                            MethodContextable auto context) noexcept -> double
{
  return method.value();
}

} // namespace pludux