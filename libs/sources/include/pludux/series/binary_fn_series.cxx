module;

#include <cassert>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

export module pludux.series.binary_fn_series;

namespace pludux {

export template<typename TBinaryFn, typename TSeriesOp1, typename TSeriesOp2>
class BinaryFnSeries {
public:
  using ValueType = std::invoke_result_t<TBinaryFn,
                                         typename TSeriesOp1::ValueType,
                                         typename TSeriesOp2::ValueType>;

  BinaryFnSeries(TSeriesOp1 operand1, TSeriesOp2 operand2)
  : operand1_{std::move(operand1)}
  , operand2_{std::move(operand2)}
  {
  }

  auto operator[](std::size_t index) const -> ValueType
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

export template<typename TSeriesOp1, typename TSeriesOp2>
using AddSeries = BinaryFnSeries<std::plus<>, TSeriesOp1, TSeriesOp2>;

export template<typename TSeriesOp1, typename TSeriesOp2>
using SubtractSeries = BinaryFnSeries<std::minus<>, TSeriesOp1, TSeriesOp2>;

export template<typename TSeriesOp1, typename TSeriesOp2>
using MultiplySeries =
 BinaryFnSeries<std::multiplies<>, TSeriesOp1, TSeriesOp2>;

export template<typename TSeriesOp1, typename TSeriesOp2>
using DivideSeries = BinaryFnSeries<std::divides<>, TSeriesOp1, TSeriesOp2>;

} // namespace pludux
