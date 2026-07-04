module;

#include <algorithm>
#include <cmath>
#include <concepts>
#include <functional>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

export module pludux:evaluate_series_method;

import :asset_snapshot;
import :method_contextable;

// Import all method modules for type/template visibility
import :methods.atr_method;
import :methods.bb_method;
import :methods.change_method;
import :methods.data_method;
import :methods.percentage_method;
import :methods.roc_method;
import :methods.rvol_method;
import :methods.rsi_method;
import :methods.rma_method;
import :methods.ema_method;
import :methods.highest_method;
import :methods.hma_method;
import :methods.kc_method;
import :methods.lookback_method;
import :methods.lowest_method;
import :methods.macd_method;
import :methods.ohlcv_method;
import :methods.operators_method;
import :methods.adaptive_ma_method;
import :methods.sma_method;
import :methods.wma_method;
import :methods.rma_method;
import :methods.select_output_method;
import :methods.series_node_method;
import :methods.series_value_method;
import :methods.stddev_method;
import :methods.stoch_method;
import :methods.stoch_rsi_method;
import :methods.tr_method;
import :methods.value_method;
import :methods.wma_method;
import :methods.all_of_method;
import :methods.any_of_method;
import :methods.crossover_method;
import :methods.crossunder_method;
import :methods.logical_method;
import :methods.boolean_method;
import :methods.comparison_method;
import :methods.donchian_channel_method;

export namespace pludux {

void pludux_tag_invoke();

inline constexpr struct EvaluateSeriesMethod {
  template<typename TMethod>
  auto operator()(
   this EvaluateSeriesMethod self,
   TMethod&& method,
   AssetSnapshot asset_snapshot,
   MethodContextable auto
    context) noexcept(noexcept(pludux_tag_invoke(self,
                                                 std::forward<TMethod>(method),
                                                 std::move(asset_snapshot),
                                                 context))) -> decltype(auto)
  {
    return pludux_tag_invoke(
     self, std::forward<TMethod>(method), std::move(asset_snapshot), context);
  }

  template<typename TMethod>
  auto operator()(
   this EvaluateSeriesMethod self,
   MethodOutput output,
   TMethod&& method,
   AssetSnapshot asset_snapshot,
   MethodContextable auto
    context) noexcept(noexcept(pludux_tag_invoke(self,
                                                 output,
                                                 std::forward<TMethod>(method),
                                                 std::move(asset_snapshot),
                                                 context))) -> decltype(auto)
    requires requires {
      {
        pludux_tag_invoke(self,
                          std::forward<TMethod>(method),
                          std::move(asset_snapshot),
                          context)
      } -> std::convertible_to<double>;
      {
        pludux_tag_invoke(self,
                          output,
                          std::forward<TMethod>(method),
                          std::move(asset_snapshot),
                          context)
      } -> std::convertible_to<double>;
    }
  {
    return pludux_tag_invoke(self,
                             output,
                             std::forward<TMethod>(method),
                             std::move(asset_snapshot),
                             context);
  }

  template<typename TMethod>
  auto operator()(this EvaluateSeriesMethod self,
                  MethodOutput output,
                  TMethod&& method,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> double
    requires requires {
      {
        pludux_tag_invoke(self,
                          std::forward<TMethod>(method),
                          std::move(asset_snapshot),
                          context)
      } -> std::convertible_to<double>;
      requires(!requires {
        {
          pludux_tag_invoke(self,
                            output,
                            std::forward<TMethod>(method),
                            std::move(asset_snapshot),
                            context)
        } -> std::convertible_to<double>;
      });
    }
  {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto operator()(this EvaluateSeriesMethod,
                  const EvaluateSeriesMethod&) noexcept -> double = delete;
} evaluate_series_method;

template<typename TParameterMethod>
auto evaluated_method_to_size(const TParameterMethod& parameter,
                              AssetSnapshot asset_snapshot,
                              MethodContextable auto context) noexcept
 -> std::size_t
{
  return static_cast<std::size_t>(
   evaluate_series_method(parameter, std::move(asset_snapshot), context));
}

template<typename TMethod>
  requires std::is_arithmetic_v<TMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       TMethod method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return static_cast<double>(method);
}

// ATR
template<typename TPeriodMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const AtrMethod<TPeriodMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);
  const auto ma_method =
   AdaptiveMaMethod{TrMethod{}, method.ma_smoothing_type(), period};

  return evaluate_series_method(ma_method, std::move(asset_snapshot), context);
}

// TR
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const TrMethod& method,
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
template<typename TMaSourceMethod, typename TParameterMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const BbMethod<TMaSourceMethod, TParameterMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   MethodOutput::MiddleBand, method, std::move(asset_snapshot), context);
}

template<typename TMaSourceMethod, typename TParameterMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 MethodOutput output,
 const BbMethod<TMaSourceMethod, TParameterMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto ma_source = method.source();
  const auto ma_period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);
  const auto stddev =
   evaluate_series_method(method.stddev(), asset_snapshot, context);
  const auto ma_method =
   AdaptiveMaMethod{ma_source, method.ma_method_type(), ma_period};
  const auto middle =
   evaluate_series_method(ma_method, asset_snapshot, context);

  const auto stddev_method = StddevMethod{ma_source, ma_period};
  const auto std_dev =
   evaluate_series_method(stddev_method, asset_snapshot, context);
  const auto std_dev_scaled = std_dev * stddev;
  switch(output) {
  case MethodOutput::MiddleBand:
    return middle;
  case MethodOutput::UpperBand:
    return middle + std_dev_scaled;
  case MethodOutput::LowerBand:
    return middle - std_dev_scaled;
  default:
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// Change
template<typename TSourceMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const ChangeMethod<TSourceMethod>& method,
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
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const DataMethod& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return asset_snapshot.data(method.field());
}

// Percentage
template<typename TMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const PercentageMethod<TMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto total =
   evaluate_series_method(method.base(), asset_snapshot, context);
  const auto percentage = total * (method.percent() / 100.0);
  return percentage;
}

// SMA
template<typename TSourceMethod, typename TPeriodMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const SmaMethod<TSourceMethod, TPeriodMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);
  const auto asset_size = asset_snapshot.size();
  if(asset_size < period) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto sum = 0.0;
  for(auto i = 0uz; i < period; ++i) {
    sum += evaluate_series_method(method.source(), asset_snapshot[i], context);
  }

  return sum / period;
}

// EMA RMA
template<template<typename, typename> typename TEmaMethod,
         typename TSourceMethod,
         typename TPeriodMethod>
  requires(
   std::same_as<std::remove_cvref_t<TEmaMethod<TSourceMethod, TPeriodMethod>>,
                EmaMethod<TSourceMethod, TPeriodMethod>> ||
   std::same_as<std::remove_cvref_t<TEmaMethod<TSourceMethod, TPeriodMethod>>,
                RmaMethod<TSourceMethod, TPeriodMethod>>)
auto evaluate_ema_or_rma_with_cached_results(
 std::vector<double>& cached_results,
 const TEmaMethod<TSourceMethod, TPeriodMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto& source_method = method.source();
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);

  const auto prev_result = cached_results.empty()
                            ? std::numeric_limits<double>::quiet_NaN()
                            : cached_results.back();
  if(std::isnan(prev_result)) {
    return evaluate_series_method(
     SmaMethod{source_method, period}, asset_snapshot, context);
  }

  const auto alpha =
   std::is_same_v<std::remove_cvref_t<TEmaMethod<TSourceMethod, TPeriodMethod>>,
                  EmaMethod<TSourceMethod, TPeriodMethod>>
    ? 2.0 / (period + 1)
    : 1.0 / period;

  const auto source =
   evaluate_series_method(source_method, asset_snapshot, context);

  const auto result = alpha * source + (1 - alpha) * prev_result;

  return result;
}

template<template<typename, typename> typename TEmaMethod,
         typename TSourceMethod,
         typename TPeriodMethod>
  requires(
   std::same_as<std::remove_cvref_t<TEmaMethod<TSourceMethod, TPeriodMethod>>,
                EmaMethod<TSourceMethod, TPeriodMethod>> ||
   std::same_as<std::remove_cvref_t<TEmaMethod<TSourceMethod, TPeriodMethod>>,
                RmaMethod<TSourceMethod, TPeriodMethod>>)
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const TEmaMethod<TSourceMethod, TPeriodMethod>& method,
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
    const auto result = evaluate_ema_or_rma_with_cached_results(
     cached_results, method, asset_snapshot[i], context);
    cached_results.push_back(result);
  }

  return evaluate_ema_or_rma_with_cached_results(
   cached_results, method, std::move(asset_snapshot), context);
}

// WMA
template<typename TSourceMethod, typename TPeriodMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const WmaMethod<TSourceMethod, TPeriodMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);
  const auto asset_size = asset_snapshot.size();
  if(asset_size < period) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto norm = 0.0;
  auto sum = 0.0;
  for(auto i = 0uz; i < period; ++i) {
    const auto weight = (period - i) * period;
    sum += evaluate_series_method(method.source(), asset_snapshot[i], context) *
           weight;
    norm += weight;
  }

  return sum / norm;
}

// HMA
template<typename TSourceMethod, typename TPeriodMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const HmaMethod<TSourceMethod, TPeriodMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);
  const auto wam1 = WmaMethod{method.source(), period / 2};
  const auto scalar_2_method = ValueMethod{2.0};
  const auto times_2_wam1 = MultiplyMethod{scalar_2_method, wam1};

  const auto wam2 = WmaMethod{method.source(), period};
  const auto diff = SubtractMethod{times_2_wam1, wam2};

  const auto hma = WmaMethod{diff, static_cast<std::size_t>(std::sqrt(period))};

  return evaluate_series_method(hma, asset_snapshot, context);
}

// RSI
template<typename TSourceMethod, typename TPeriodMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const RsiMethod<TSourceMethod, TPeriodMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);
  const auto rs_method = DivideMethod{
   RmaMethod{PositivePartMethod{ChangeMethod{method.source()}}, period},
   RmaMethod{NegativePartMethod{ChangeMethod{method.source()}}, period}};

  const auto rs = evaluate_series_method(rs_method, asset_snapshot, context);
  const auto rsi = 100 - (100 / (1 + rs));

  return rsi;
}

// ROC
template<typename TSourceMethod, typename TPeriodMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const RocMethod<TSourceMethod, TPeriodMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);
  const auto source_size = asset_snapshot.size();
  if(source_size < period) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  const auto current =
   evaluate_series_method(method.source(), asset_snapshot, context);
  const auto end =
   evaluate_series_method(method.source(), asset_snapshot[period], context);

  return 100.0 * (current - end) / end;
}

// RVOL
template<typename TPeriodMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const RvolMethod<TPeriodMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);
  const auto volume = VolumeMethod{};
  const auto sma_volume = SmaMethod{VolumeMethod{}, period};
  const auto rvol = DivideMethod{volume, sma_volume};
  const auto rvol_result =
   evaluate_series_method(rvol, asset_snapshot, context);
  return rvol_result;
}

// Highest
template<typename TSourceMethod, typename TPeriodMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const HighestMethod<TSourceMethod, TPeriodMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);

  if(asset_snapshot.size() < period) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto highest = std::numeric_limits<double>::min();
  for(auto i = 0uz; i < period; ++i) {
    const auto value =
     evaluate_series_method(method.source(), asset_snapshot[i], context);
    highest = std::max(highest, value);
  }
  return highest;
}

// KC
template<typename TMaSourceMethod, typename TParameterMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const KcMethod<TMaSourceMethod, TParameterMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   MethodOutput::MiddleBand, method, std::move(asset_snapshot), context);
}

template<typename TMaSourceMethod, typename TParameterMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 MethodOutput output,
 const KcMethod<TMaSourceMethod, TParameterMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);
  const auto multiplier =
   evaluate_series_method(method.multiplier(), asset_snapshot, context);
  const auto band_atr_period =
   evaluated_method_to_size(method.band_atr_period(), asset_snapshot, context);
  const auto band_range = [&]() -> double {
    switch(method.band_method_type()) {
    case KcBandMethodType::Atr: {
      const auto atr_method = AtrMethod{band_atr_period};
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
   AdaptiveMaMethod{method.source(), method.ma_method_type(), period};
  const auto middle =
   evaluate_series_method(ma_method, asset_snapshot, context);

  switch(output) {
  case MethodOutput::MiddleBand:
    return middle;
  case MethodOutput::UpperBand:
    return middle + (multiplier * band_range());
  case MethodOutput::LowerBand:
    return middle - (multiplier * band_range());
  default:
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// DONCHIAN CHANNEL
template<typename TPeriodMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const DonchianChannelMethod<TPeriodMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   MethodOutput::MiddleBand, method, std::move(asset_snapshot), context);
}

template<typename TPeriodMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       MethodOutput output,
                       const DonchianChannelMethod<TPeriodMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);
  if(asset_snapshot.size() < period) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto highest = std::numeric_limits<double>::min();
  auto lowest = std::numeric_limits<double>::max();
  for(auto i = 0uz; i < period; ++i) {
    const auto high =
     evaluate_series_method(HighMethod{}, asset_snapshot[i], context);
    const auto low =
     evaluate_series_method(LowMethod{}, asset_snapshot[i], context);
    highest = std::max(highest, high);
    lowest = std::min(lowest, low);
  }

  switch(output) {
  case MethodOutput::MiddleBand:
    return (highest + lowest) / 2.0;
  case MethodOutput::UpperBand:
    return highest;
  case MethodOutput::LowerBand:
    return lowest;
  default:
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// Lookback
template<typename TSourceMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const LookbackMethod<TSourceMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   method.source(), asset_snapshot[method.period()], context);
}

// SelectOutput
template<typename TSourceMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const SelectOutputMethod<TSourceMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   method.output(), method, asset_snapshot, context);
}

template<typename TSourceMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       MethodOutput output,
                       const SelectOutputMethod<TSourceMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   output, method.source(), asset_snapshot, context);
}

// Lowest

template<typename TSourceMethod, typename TPeriodMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const LowestMethod<TSourceMethod, TPeriodMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);

  if(asset_snapshot.size() < period) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto lowest = std::numeric_limits<double>::max();
  for(auto i = 0uz; i < period; ++i) {
    const auto value =
     evaluate_series_method(method.source(), asset_snapshot[i], context);
    lowest = std::min(lowest, value);
  }
  return lowest;
}

// MACD

template<typename TSourceMethod, typename TParameterMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const MacdMethod<TSourceMethod, TParameterMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   MethodOutput::MacdLine, method, asset_snapshot, context);
}

template<typename TSourceMethod, typename TParameterMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 MethodOutput output,
 const MacdMethod<TSourceMethod, TParameterMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto short_period =
   evaluated_method_to_size(method.short_period(), asset_snapshot, context);
  const auto long_period =
   evaluated_method_to_size(method.long_period(), asset_snapshot, context);
  const auto signal_period =
   evaluated_method_to_size(method.signal_period(), asset_snapshot, context);
  const auto macd_method =
   SubtractMethod{EmaMethod{method.source(), short_period},
                  EmaMethod{method.source(), long_period}};
  const auto signal_ema = EmaMethod{macd_method, signal_period};

  const auto macd =
   evaluate_series_method(macd_method, asset_snapshot, context);
  const auto signal =
   evaluate_series_method(signal_ema, asset_snapshot, context);
  const auto histogram = macd - signal;

  switch(output) {
  case MethodOutput::MacdLine:
    return macd;
  case MethodOutput::SignalLine:
    return signal;
  case MethodOutput::Histogram:
    return histogram;
  default:
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// OhlcvMethod (Open, High, Low, Close, Volume)

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       OpenMethod method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return asset_snapshot.open();
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       HighMethod method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return asset_snapshot.high();
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       LowMethod method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return asset_snapshot.low();
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       CloseMethod method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return asset_snapshot.close();
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       VolumeMethod method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return asset_snapshot.volume();
}

// Operators (Multiply, Divide, Add, Subtract, Negate, Abs, AbsDiff, Sqrt, Max,
// Min, PositivePart, NegativePart)

template<typename TBinaryFn,
         typename TLeftOperandMethod,
         typename TRightOperandMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const BinaryOperatorMethod<TBinaryFn, TLeftOperandMethod, TRightOperandMethod>&
  method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto left_result =
   evaluate_series_method(method.left(), asset_snapshot, context);
  const auto right_result =
   evaluate_series_method(method.right(), asset_snapshot, context);

  return TBinaryFn{}(left_result, right_result);
}

template<typename TUnaryFn, typename TOperandMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const UnaryOperatorMethod<TUnaryFn, TOperandMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto operand_result =
   evaluate_series_method(method.operand(), asset_snapshot, context);

  return TUnaryFn{}(operand_result);
}

// Adaptive MA

template<typename TSourceMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const AdaptiveMaMethod<TSourceMethod>& method,
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

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const SeriesNodeMethod& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  if constexpr(std::is_same_v<std::monostate, decltype(context)>) {
    return std::numeric_limits<double>::quiet_NaN();
  } else {
    return context.call_series_method(method.name(), asset_snapshot);
  }
}

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       MethodOutput output,
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

auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const SeriesValueMethod& method,
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

template<typename TSourceMethod, typename TPeriodMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const StddevMethod<TSourceMethod, TPeriodMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto period =
   evaluated_method_to_size(method.period(), asset_snapshot, context);
  const auto asset_size = asset_snapshot.size();
  if(asset_size < period) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto sum = 0.0;
  for(auto i = 0uz; i < period; ++i) {
    sum += evaluate_series_method(method.source(), asset_snapshot[i], context);
  }
  const auto mean = sum / period;

  auto sum_squared_diff = 0.0;
  for(auto i = 0uz; i < period; ++i) {
    const auto diff =
     evaluate_series_method(method.source(), asset_snapshot[i], context) - mean;
    sum_squared_diff += diff * diff;
  }

  const auto variance = sum_squared_diff / period;
  const auto stddev = std::sqrt(variance);
  return stddev;
}

// Stochastic
template<typename TParameterMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const StochMethod<TParameterMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   MethodOutput::KPercent, method, asset_snapshot, context);
}

template<typename TParameterMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       MethodOutput output,
                       const StochMethod<TParameterMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  const auto k_period =
   evaluated_method_to_size(method.k_period(), asset_snapshot, context);
  const auto k_smooth =
   evaluated_method_to_size(method.k_smooth(), asset_snapshot, context);
  const auto d_period =
   evaluated_method_to_size(method.d_period(), asset_snapshot, context);
  const auto close = CloseMethod{};
  const auto highest_high = HighestMethod{HighMethod{}, k_period};
  const auto lowest_low = LowestMethod{LowMethod{}, k_period};
  const auto stoch = DivideMethod{MultiplyMethod{ValueMethod{100},
                                                 SubtractMethod{
                                                  close,
                                                  lowest_low,
                                                 }},
                                  SubtractMethod{
                                   highest_high,
                                   lowest_low,
                                  }};

  const auto k_percent = SmaMethod{stoch, k_smooth};

  switch(output) {
  case MethodOutput::KPercent:
    return evaluate_series_method(k_percent, asset_snapshot, context);
  case MethodOutput::DPercent:
    return evaluate_series_method(
     SmaMethod{k_percent, d_period}, asset_snapshot, context);
  default:
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// Stochastic RSI
template<typename TRsiSourceMethod, typename TParameterMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const StochRsiMethod<TRsiSourceMethod, TParameterMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  return evaluate_series_method(
   MethodOutput::KPercent, method, std::move(asset_snapshot), context);
}

template<typename TRsiSourceMethod, typename TParameterMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 MethodOutput output,
 const StochRsiMethod<TRsiSourceMethod, TParameterMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto rsi_period =
   evaluated_method_to_size(method.rsi_period(), asset_snapshot, context);
  const auto k_period =
   evaluated_method_to_size(method.k_period(), asset_snapshot, context);
  const auto k_smooth =
   evaluated_method_to_size(method.k_smooth(), asset_snapshot, context);
  const auto d_period =
   evaluated_method_to_size(method.d_period(), asset_snapshot, context);
  const auto rsi = RsiMethod{method.rsi_source(), rsi_period};
  const auto highest_rsi = HighestMethod{rsi, k_period};
  const auto lowest_rsi = LowestMethod{rsi, k_period};
  const auto stoch = DivideMethod{MultiplyMethod{ValueMethod{100},
                                                 SubtractMethod{
                                                  rsi,
                                                  lowest_rsi,
                                                 }},
                                  SubtractMethod{
                                   highest_rsi,
                                   lowest_rsi,
                                  }};

  const auto k_percent = SmaMethod{stoch, k_smooth};

  switch(output) {
  case MethodOutput::KPercent:
    return evaluate_series_method(k_percent, asset_snapshot, context);
  case MethodOutput::DPercent:
    return evaluate_series_method(
     SmaMethod{k_percent, d_period}, asset_snapshot, context);
  default:
    return std::numeric_limits<double>::quiet_NaN();
  }
}

// Value
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const ValueMethod& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return method.value();
}

// AllOf
template<typename TConditionMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const AllOfMethod<TConditionMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return std::ranges::all_of(
   method.conditions(), [&asset_snapshot, &context](const auto& method) {
     return static_cast<bool>(
      evaluate_series_method(method, asset_snapshot, context));
   });
}

// AnyOf
template<typename TConditionMethod>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const AnyOfMethod<TConditionMethod>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return std::ranges::any_of(
   method.conditions(), [&asset_snapshot, &context](const auto& method) {
     return static_cast<bool>(
      evaluate_series_method(method, asset_snapshot, context));
   });
}

// Crossover
template<typename TSourceMethod, typename TReferenceMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const CrossoverMethod<TSourceMethod, TReferenceMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto source_current =
   evaluate_series_method(method.source(), asset_snapshot, context);
  const auto reference_current =
   evaluate_series_method(method.reference(), asset_snapshot, context);

  const auto source_previous =
   evaluate_series_method(method.source(), asset_snapshot[1], context);
  const auto reference_previous =
   evaluate_series_method(method.reference(), asset_snapshot[1], context);

  const auto crossed_over = (source_previous <= reference_previous) &&
                            (source_current > reference_current);

  return crossed_over;
}

// Crossunder
template<typename TSourceMethod, typename TReferenceMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const CrossunderMethod<TSourceMethod, TReferenceMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto source_current =
   evaluate_series_method(method.source(), asset_snapshot, context);
  const auto reference_current =
   evaluate_series_method(method.reference(), asset_snapshot, context);

  const auto source_previous =
   evaluate_series_method(method.source(), asset_snapshot[1], context);
  const auto reference_previous =
   evaluate_series_method(method.reference(), asset_snapshot[1], context);

  const auto crossed_under = (source_previous >= reference_previous) &&
                             (source_current < reference_current);

  return crossed_under;
}

// Boolean (True, False)
template<bool TBool>
auto pludux_tag_invoke(EvaluateSeriesMethod,
                       const BooleanMethod<TBool>& method,
                       AssetSnapshot asset_snapshot,
                       MethodContextable auto context) noexcept -> double
{
  return TBool;
}

// Binary Logical (And, Or, Xor)
template<typename TOperation,
         typename TFirstCondition,
         typename TSecondCondition>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const BinaryLogicalMethod<TOperation, TFirstCondition, TSecondCondition>&
  method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto first_result =
   evaluate_series_method(method.first_condition(), asset_snapshot, context);
  const auto second_result =
   evaluate_series_method(method.second_condition(), asset_snapshot, context);

  return TOperation{}(first_result, second_result);
}

// Unary Logical (Not)
template<typename TOperation, typename TOtherCondition>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const UnaryLogicalMethod<TOperation, TOtherCondition>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto other_result =
   evaluate_series_method(method.other_condition(), asset_snapshot, context);

  return TOperation{}(other_result);
}

// Comparison (Greater, GreaterEqual, Less, LessEqual, Equal, NotEqual)
template<typename TComparator,
         typename TTargetMethod,
         typename TThresholdMethod>
auto pludux_tag_invoke(
 EvaluateSeriesMethod,
 const ComparisonMethod<TComparator, TTargetMethod, TThresholdMethod>& method,
 AssetSnapshot asset_snapshot,
 MethodContextable auto context) noexcept -> double
{
  const auto target_result =
   evaluate_series_method(method.target(), asset_snapshot, context);
  const auto threshold_result =
   evaluate_series_method(method.threshold(), asset_snapshot, context);

  return TComparator{}(target_result, threshold_result);
}

} // namespace pludux
