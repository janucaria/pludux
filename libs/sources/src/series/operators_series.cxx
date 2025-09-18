module;

#include <cassert>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

export module pludux:series.operators_series;

import :series.series_output;

export namespace pludux {

template<typename TBinaryFn, typename TSeriesOp1, typename TSeriesOp2>
class BinaryOpSeries {
public:
  using ValueType = std::invoke_result_t<TBinaryFn,
                                         typename TSeriesOp1::ValueType,
                                         typename TSeriesOp2::ValueType>;

  BinaryOpSeries(TSeriesOp1 operand1, TSeriesOp2 operand2)
  : operand1_{std::move(operand1)}
  , operand2_{std::move(operand2)}
  {
  }

  auto operator[](std::size_t index) const -> SeriesOutput<ValueType>
  {
    const auto operand1_value = operand1_[index];
    const auto operand2_value = operand2_[index];
    return TBinaryFn{}(operand1_value, operand2_value);
  }

  auto size() const noexcept -> std::size_t
  {
    assert(operand1_.size() == operand2_.size());
    return operand1_.size();
  }

private:
  TSeriesOp1 operand1_;
  TSeriesOp2 operand2_;
};

template<typename TUnaryFn, typename TSeriesOp>
class UnaryOpSeries {
public:
  using ValueType =
   std::invoke_result_t<TUnaryFn, typename TSeriesOp::ValueType>;

  explicit UnaryOpSeries(TSeriesOp operand) noexcept
  : operand_{std::move(operand)}
  {
  }

  auto operator[](std::size_t index) const noexcept -> SeriesOutput<ValueType>
  {
    const auto operand_value = static_cast<ValueType>(operand_[index]);
    return TUnaryFn{}(operand_value);
  }

  auto size() const noexcept -> std::size_t
  {
    return operand_.size();
  }

private:
  TSeriesOp operand_;
};

template<typename TSeriesOp>
using NegateSeries = UnaryOpSeries<std::negate<>, TSeriesOp>;

template<typename TSeriesOp1, typename TSeriesOp2>
using AddSeries = BinaryOpSeries<std::plus<>, TSeriesOp1, TSeriesOp2>;

template<typename TSeriesOp1, typename TSeriesOp2>
using SubtractSeries = BinaryOpSeries<std::minus<>, TSeriesOp1, TSeriesOp2>;

template<typename TSeriesOp1, typename TSeriesOp2>
using MultiplySeries =
 BinaryOpSeries<std::multiplies<>, TSeriesOp1, TSeriesOp2>;

template<typename TSeriesOp1, typename TSeriesOp2>
using DivideSeries = BinaryOpSeries<std::divides<>, TSeriesOp1, TSeriesOp2>;

} // namespace pludux
