#ifndef PLUDUX_PLUDUX_SERIES_UNARY_FN_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_UNARY_FN_SERIES_HPP

#include <cassert>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

#include <pludux/series/series_output.hpp>

namespace pludux {

template<typename TUnaryFn, typename TSeriesOp>
class UnaryFnSeries {
public:
  using ValueType =
   std::invoke_result_t<TUnaryFn, typename TSeriesOp::ValueType>;

  explicit UnaryFnSeries(TSeriesOp operand) noexcept
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
using NegateSeries = UnaryFnSeries<std::negate<>, TSeriesOp>;

} // namespace pludux

#endif