module;

#include <algorithm>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <functional>
#include <limits>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include <boost/mp11.hpp>

export module pludux:hash_series_method;

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

export namespace pludux {

namespace mp11 = boost::mp11;

using MethodRegistry = mp11::mp_list<mp11::mp_quote<AbsDiffMethod>,
                                     mp11::mp_quote<AbsMethod>,
                                     mp11::mp_quote<AdaptiveMaMethod>,
                                     mp11::mp_quote<AddMethod>,
                                     AtrMethod,
                                     mp11::mp_quote<BbMethod>,
                                     mp11::mp_quote<ChangeMethod>,
                                     CloseMethod,
                                     DataMethod,
                                     mp11::mp_quote<DivideMethod>,
                                     mp11::mp_quote<EmaMethod>,
                                     mp11::mp_quote<HighestMethod>,
                                     HighMethod,
                                     mp11::mp_quote<HmaMethod>,
                                     mp11::mp_quote<KcMethod>,
                                     mp11::mp_quote<LookbackMethod>,
                                     mp11::mp_quote<LowestMethod>,
                                     LowMethod,
                                     mp11::mp_quote<MacdMethod>,
                                     mp11::mp_quote<MaxMethod>,
                                     mp11::mp_quote<MinMethod>,
                                     mp11::mp_quote<MultiplyMethod>,
                                     mp11::mp_quote<NegateMethod>,
                                     mp11::mp_quote<NegativePartMethod>,
                                     OpenMethod,
                                     mp11::mp_quote<PercentageMethod>,
                                     mp11::mp_quote<PositivePartMethod>,
                                     mp11::mp_quote<RmaMethod>,
                                     mp11::mp_quote<RocMethod>,
                                     mp11::mp_quote<RsiMethod>,
                                     RvolMethod,
                                     mp11::mp_quote<SelectOutputMethod>,
                                     SeriesNodeMethod,
                                     SeriesValueMethod,
                                     mp11::mp_quote<SmaMethod>,
                                     mp11::mp_quote<SqrtMethod>,
                                     mp11::mp_quote<StddevMethod>,
                                     StochMethod,
                                     mp11::mp_quote<StochRsiMethod>,
                                     mp11::mp_quote<SubtractMethod>,
                                     TrMethod,
                                     ValueMethod,
                                     VolumeMethod,
                                     mp11::mp_quote<WmaMethod>,
                                     mp11::mp_quote<AllOfMethod>,
                                     mp11::mp_quote<AnyOfMethod>,
                                     mp11::mp_quote<CrossoverMethod>,
                                     mp11::mp_quote<CrossunderMethod>,
                                     mp11::mp_quote<LogicalAndMethod>,
                                     mp11::mp_quote<LogicalOrMethod>,
                                     mp11::mp_quote<LogicalNotMethod>,
                                     mp11::mp_quote<LogicalXorMethod>,
                                     TrueMethod,
                                     FalseMethod,
                                     mp11::mp_quote<EqualMethod>,
                                     mp11::mp_quote<NotEqualMethod>,
                                     mp11::mp_quote<GreaterThanMethod>,
                                     mp11::mp_quote<GreaterEqualMethod>,
                                     mp11::mp_quote<LessThanMethod>,
                                     mp11::mp_quote<LessEqualMethod>>;

template<typename TMethod>
consteval auto series_type_hash_id_of(const TMethod&) -> std::size_t
{
  return mp11::mp_find<MethodRegistry, TMethod>::value + 1;
}

template<template<typename...> typename TMethod, typename... TArgs>
consteval auto series_type_hash_id_of(const TMethod<TArgs...>&) -> std::size_t
{
  return mp11::mp_find<MethodRegistry, mp11::mp_quote<TMethod>>::value + 1;
}

template<typename... TArgs>
  requires(std::same_as<std::remove_cvref_t<TArgs>, std::size_t> && ...)
constexpr auto merge_hashes(std::size_t hash1,
                            std::size_t hash2,
                            TArgs... hashes) noexcept -> std::size_t
{
  std::size_t seed = hash1;
  seed ^= hash2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  if constexpr(sizeof...(hashes) > 0) {
    return merge_hashes(seed, hashes...);
  } else {
    return seed;
  }
}

template<typename T>
constexpr auto hash_series_method_or_std_hash(const T& value) noexcept
 -> std::size_t
{
  if constexpr(requires {
                 {
                   hash_series_method(value)
                 } -> std::convertible_to<std::size_t>;
               }) {
    // Preserve method parameters by delegating to the dedicated method hasher.
    return hash_series_method(value);
  } else {
    return std::hash<T>{}(value);
  }
}

// ATR
auto hash_series_method(const AtrMethod& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, period_hash);
}

// TR
auto hash_series_method(const TrMethod& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  return type_hash;
}

// BB
template<typename TSourceMethod>
auto hash_series_method(const BbMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto ma_type_hash =
   hash_series_method_or_std_hash(method.ma_method_type());
  const auto ma_source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  const auto stddev_hash = hash_series_method_or_std_hash(method.stddev());

  return merge_hashes(
   type_hash, ma_type_hash, ma_source_hash, period_hash, stddev_hash);
}

// Change
template<typename TSourceMethod>
auto hash_series_method(const ChangeMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  return merge_hashes(type_hash, source_hash);
}

// Data
auto hash_series_method(const DataMethod& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto field_hash = hash_series_method_or_std_hash(method.field());
  return merge_hashes(type_hash, field_hash);
}

// Percentage
template<typename TMethod>
auto hash_series_method(const PercentageMethod<TMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto base_hash = hash_series_method_or_std_hash(method.base());
  const auto percent_hash = hash_series_method_or_std_hash(method.percent());

  return merge_hashes(type_hash, base_hash, percent_hash);
}

// SMA
template<typename TSourceMethod>
auto hash_series_method(const SmaMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, source_hash, period_hash);
}

// EMA
template<typename TSourceMethod>
auto hash_series_method(const EmaMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, source_hash, period_hash);
}

// RMA
template<typename TSourceMethod>
auto hash_series_method(const RmaMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, source_hash, period_hash);
}

// WMA
template<typename TSourceMethod>
auto hash_series_method(const WmaMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, source_hash, period_hash);
}

// HMA
template<typename TSourceMethod>
auto hash_series_method(const HmaMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, source_hash, period_hash);
}

// RSI
template<typename TSourceMethod>
auto hash_series_method(const RsiMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, source_hash, period_hash);
}

// ROC
template<typename TSourceMethod>
auto hash_series_method(const RocMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, source_hash, period_hash);
}

// RVOL
auto hash_series_method(const RvolMethod& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, period_hash);
}

// Highest
template<typename TSourceMethod>
auto hash_series_method(const HighestMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, source_hash, period_hash);
}

// KC
template<typename TMaSourceMethod>
auto hash_series_method(const KcMethod<TMaSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto ma_source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  const auto ma_method_type_hash =
   hash_series_method_or_std_hash(method.ma_method_type());
  const auto multiplier_hash =
   hash_series_method_or_std_hash(method.multiplier());
  const auto band_method_type_hash =
   hash_series_method_or_std_hash(method.band_method_type());
  const auto band_atr_period_hash =
   hash_series_method_or_std_hash(method.band_atr_period());
  return merge_hashes(type_hash,
                      ma_source_hash,
                      period_hash,
                      ma_method_type_hash,
                      multiplier_hash,
                      band_method_type_hash,
                      band_atr_period_hash);
}

// Lookback

template<typename TSourceMethod>
auto hash_series_method(const LookbackMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, source_hash, period_hash);
}

// SelectOutput
template<typename TSourceMethod>
auto hash_series_method(
 const SelectOutputMethod<TSourceMethod>& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto output_hash = hash_series_method_or_std_hash(method.output());
  return merge_hashes(type_hash, source_hash, output_hash);
}

// Lowest

template<typename TSourceMethod>
auto hash_series_method(const LowestMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, source_hash, period_hash);
}

// MACD

template<typename TSourceMethod>
auto hash_series_method(const MacdMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto short_period_hash =
   hash_series_method_or_std_hash(method.short_period());
  const auto long_period_hash =
   hash_series_method_or_std_hash(method.long_period());
  const auto signal_period_hash =
   hash_series_method_or_std_hash(method.signal_period());
  return merge_hashes(type_hash,
                      source_hash,
                      short_period_hash,
                      long_period_hash,
                      signal_period_hash);
}

// OhlcvMethod (Open, High, Low, Close, Volume)

auto hash_series_method(const OpenMethod& method) noexcept -> std::size_t
{
  return series_type_hash_id_of(method);
}

auto hash_series_method(const HighMethod& method) noexcept -> std::size_t
{
  return series_type_hash_id_of(method);
}

auto hash_series_method(const LowMethod& method) noexcept -> std::size_t
{
  return series_type_hash_id_of(method);
}

auto hash_series_method(const CloseMethod& method) noexcept -> std::size_t
{
  return series_type_hash_id_of(method);
}

auto hash_series_method(const VolumeMethod& method) noexcept -> std::size_t
{
  return series_type_hash_id_of(method);
}

// Operators (Multiply, Divide, Add, Subtract, Negate, Abs, AbsDiff, Sqrt, Max,
// Min, PositivePart, NegativePart)
template<typename TBinaryFn,
         typename TLeftOperandMethod,
         typename TRightOperandMethod>
auto hash_series_method(
 const BinaryOperatorMethod<TBinaryFn, TLeftOperandMethod, TRightOperandMethod>&
  method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);

  const auto left_result = hash_series_method(method.left());
  const auto right_result = hash_series_method(method.right());

  return merge_hashes(type_hash, left_result, right_result);
}

template<typename TUnaryFn, typename TOperandMethod>
auto hash_series_method(
 const UnaryOperatorMethod<TUnaryFn, TOperandMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto operand_hash = hash_series_method(method.operand());
  return merge_hashes(type_hash, operand_hash);
}

// Adaptive MA
template<typename TSourceMethod>
auto hash_series_method(const AdaptiveMaMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto ma_type_hash = hash_series_method_or_std_hash(method.ma_type());
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, source_hash, period_hash);
}

// SeriesNode

auto hash_series_method(const SeriesNodeMethod& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto name_hash = hash_series_method_or_std_hash(method.name());
  return merge_hashes(type_hash, name_hash);
}

// SeriesValue

auto hash_series_method(const SeriesValueMethod& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto name_hash = hash_series_method_or_std_hash(method.name());
  return merge_hashes(type_hash, name_hash);
}

// Stddev

template<typename TSourceMethod>
auto hash_series_method(const StddevMethod<TSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto period_hash = hash_series_method_or_std_hash(method.period());
  return merge_hashes(type_hash, source_hash, period_hash);
}

// Stochastic
auto hash_series_method(const StochMethod& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto k_period_hash = hash_series_method_or_std_hash(method.k_period());
  const auto d_period_hash = hash_series_method_or_std_hash(method.d_period());
  const auto k_smooth_hash = hash_series_method_or_std_hash(method.k_smooth());
  return merge_hashes(type_hash, k_period_hash, d_period_hash, k_smooth_hash);
}

// Stochastic RSI
template<typename TRsiSourceMethod>
auto hash_series_method(const StochRsiMethod<TRsiSourceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto rsi_source_hash =
   hash_series_method_or_std_hash(method.rsi_source());
  const auto rsi_period_hash =
   hash_series_method_or_std_hash(method.rsi_period());
  const auto k_period_hash = hash_series_method_or_std_hash(method.k_period());
  const auto d_period_hash = hash_series_method_or_std_hash(method.d_period());
  const auto k_smooth_hash = hash_series_method_or_std_hash(method.k_smooth());
  return merge_hashes(type_hash,
                      rsi_source_hash,
                      rsi_period_hash,
                      k_period_hash,
                      d_period_hash,
                      k_smooth_hash);
}

// Value
auto hash_series_method(const ValueMethod& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto value_hash = hash_series_method_or_std_hash(method.value());
  return merge_hashes(type_hash, value_hash);
}

// AllOf
template<typename TConditionMethod>
auto hash_series_method(const AllOfMethod<TConditionMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  std::size_t conditions_hash = 0;
  for(const auto& condition : method.conditions()) {
    const auto condition_hash = hash_series_method_or_std_hash(condition);
    conditions_hash = merge_hashes(conditions_hash, condition_hash);
  }
  return merge_hashes(type_hash, conditions_hash);
}

// AnyOf
template<typename TConditionMethod>
auto hash_series_method(const AnyOfMethod<TConditionMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  std::size_t conditions_hash = 0;
  for(const auto& condition : method.conditions()) {
    const auto condition_hash = hash_series_method_or_std_hash(condition);
    conditions_hash = merge_hashes(conditions_hash, condition_hash);
  }
  return merge_hashes(type_hash, conditions_hash);
}

// Crossover
template<typename TSourceMethod, typename TReferenceMethod>
auto hash_series_method(
 const CrossoverMethod<TSourceMethod, TReferenceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto reference_hash =
   hash_series_method_or_std_hash(method.reference());
  return merge_hashes(type_hash, source_hash, reference_hash);
}

// Crossunder
template<typename TSourceMethod, typename TReferenceMethod>
auto hash_series_method(
 const CrossunderMethod<TSourceMethod, TReferenceMethod>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto source_hash = hash_series_method_or_std_hash(method.source());
  const auto reference_hash =
   hash_series_method_or_std_hash(method.reference());
  return merge_hashes(type_hash, source_hash, reference_hash);
}

// Boolean (True, False)
template<bool TBool>
auto hash_series_method(const BooleanMethod<TBool>& method) noexcept
 -> std::size_t
{
  return series_type_hash_id_of(method);
}

// Binary Logical (And, Or, Xor)
template<typename TOperation,
         typename TFirstCondition,
         typename TSecondCondition>
auto hash_series_method(
 const BinaryLogicalMethod<TOperation, TFirstCondition, TSecondCondition>&
  method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);

  const auto first_result = hash_series_method(method.first_condition());
  const auto second_result = hash_series_method(method.second_condition());

  return merge_hashes(type_hash, first_result, second_result);
}

// Unary Logical (Not)
template<typename TOperation, typename TOtherCondition>
auto hash_series_method(
 const UnaryLogicalMethod<TOperation, TOtherCondition>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto other_result = hash_series_method(method.other_condition());
  return merge_hashes(type_hash, other_result);
}

// Comparison (Greater, GreaterEqual, Less, LessEqual, Equal, NotEqual)
template<typename TComparator,
         typename TTargetMethod,
         typename TThresholdMethod>
auto hash_series_method(
 const ComparisonMethod<TComparator, TTargetMethod, TThresholdMethod>&
  method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto target_result = hash_series_method(method.target());
  const auto threshold_result = hash_series_method(method.threshold());

  return merge_hashes(type_hash, target_result, threshold_result);
}

} // namespace pludux