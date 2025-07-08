module;

#include <cassert>
#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

export module pludux.series.unary_fn_series;

namespace pludux {

export template<typename TUnaryFn, typename TSeriesOp>
class UnaryFnSeries {
public:
  using ValueType =
   std::invoke_result_t<TUnaryFn, typename TSeriesOp::ValueType>;

  explicit UnaryFnSeries(TSeriesOp operand) noexcept
  : operand_{std::move(operand)}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    const auto operand_value = operand_[index];
    return TUnaryFn{}(operand_value);
  }

  auto size() const noexcept -> std::size_t
  {
    return operand_.size();
  }

private:
  TSeriesOp operand_;
};

export template<typename TSeriesOp>
using NegateSeries = UnaryFnSeries<std::negate<>, TSeriesOp>;

} // namespace pludux
