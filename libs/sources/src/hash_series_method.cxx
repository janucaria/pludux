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
                                     ValueMethod,
                                     VolumeMethod,
                                     mp11::mp_quote<WmaMethod>>;

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
                 { hash_series_method(value) } -> std::convertible_to<std::size_t>;
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

template<typename TMethodOp1, typename TMethodOp2>
auto hash_series_method(
 const MultiplyMethod<TMethodOp1, TMethodOp2>& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto left_hash = hash_series_method_or_std_hash(method.left());
  const auto right_hash = hash_series_method_or_std_hash(method.right());
  return merge_hashes(type_hash, left_hash, right_hash);
}

template<typename TMethodOp1, typename TMethodOp2>
auto hash_series_method(
 const DivideMethod<TMethodOp1, TMethodOp2>& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto dividend_hash = hash_series_method_or_std_hash(method.dividend());
  const auto divisor_hash = hash_series_method_or_std_hash(method.divisor());
  return merge_hashes(type_hash, dividend_hash, divisor_hash);
}

template<typename TMethodOp1, typename TMethodOp2>
auto hash_series_method(
 const AddMethod<TMethodOp1, TMethodOp2>& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto augend_hash = hash_series_method_or_std_hash(method.augend());
  const auto addend_hash = hash_series_method_or_std_hash(method.addend());
  return merge_hashes(type_hash, augend_hash, addend_hash);
}

template<typename TMethodOp1, typename TMethodOp2>
auto hash_series_method(
 const SubtractMethod<TMethodOp1, TMethodOp2>& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto minuend_hash = hash_series_method_or_std_hash(method.minuend());
  const auto subtrahend_hash =
   hash_series_method_or_std_hash(method.subtrahend());
  return merge_hashes(type_hash, minuend_hash, subtrahend_hash);
}

template<typename TMethodOp>
auto hash_series_method(const NegateMethod<TMethodOp>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto operand_hash = hash_series_method_or_std_hash(method.operand());
  return merge_hashes(type_hash, operand_hash);
}

template<typename TMethodOp>
auto hash_series_method(const AbsMethod<TMethodOp>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto operand_hash = hash_series_method_or_std_hash(method.operand());
  return merge_hashes(type_hash, operand_hash);
}

template<typename TMethodOp1, typename TMethodOp2>
auto hash_series_method(
 const AbsDiffMethod<TMethodOp1, TMethodOp2>& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto left_hash = hash_series_method_or_std_hash(method.left());
  const auto right_hash = hash_series_method_or_std_hash(method.right());
  return merge_hashes(type_hash, left_hash, right_hash);
}

template<typename TMethodOp>
auto hash_series_method(const SqrtMethod<TMethodOp>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto operand_hash = hash_series_method_or_std_hash(method.operand());
  return merge_hashes(type_hash, operand_hash);
}

// MAX
template<typename TMethodOp1, typename TMethodOp2>
auto hash_series_method(
 const MaxMethod<TMethodOp1, TMethodOp2>& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto left_hash = hash_series_method_or_std_hash(method.left());
  const auto right_hash = hash_series_method_or_std_hash(method.right());
  return merge_hashes(type_hash, left_hash, right_hash);
}

// MIN
template<typename TMethodOp1, typename TMethodOp2>
auto hash_series_method(
 const MinMethod<TMethodOp1, TMethodOp2>& method) noexcept -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto left_hash = hash_series_method_or_std_hash(method.left());
  const auto right_hash = hash_series_method_or_std_hash(method.right());
  return merge_hashes(type_hash, left_hash, right_hash);
}

// POSITIVE PART
template<typename TMethodOp>
auto hash_series_method(const PositivePartMethod<TMethodOp>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto operand_hash = hash_series_method_or_std_hash(method.operand());
  return merge_hashes(type_hash, operand_hash);
}

// NEGATIVE PART
template<typename TMethodOp>
auto hash_series_method(const NegativePartMethod<TMethodOp>& method) noexcept
 -> std::size_t
{
  const auto type_hash = series_type_hash_id_of(method);
  const auto operand_hash = hash_series_method_or_std_hash(method.operand());
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
  return merge_hashes(
    type_hash,
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

} // namespace pludux