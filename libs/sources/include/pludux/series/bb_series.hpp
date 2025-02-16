#ifndef PLUDUX_PLUDUX_SERIES_BB_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_BB_SERIES_HPP

#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

#include "binary_fn_series.hpp"
#include "ref_series.hpp"
#include "sma_series.hpp"

namespace pludux {

enum class BbOutput { middle, upper, lower };

template<typename TMaSeries>
  requires requires(TMaSeries ma) {
    { ma.input() };
    { ma.period() } -> std::same_as<std::size_t>;
  }
class BbSeries {
public:
  using ValueType = std::common_type_t<double, typename TMaSeries::ValueType>;

  BbSeries(BbOutput output, TMaSeries ma, double stddev)
  : output_{output}
  , ma_{std::move(ma)}
  , stddev_{stddev}
  {
  }

  template<typename TSeries>
  BbSeries(BbOutput output, TSeries input, std::size_t period, double stddev)
  : BbSeries{output, SmaSeries{std::move(input), period}, stddev}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    const auto period = ma_.period();
    const auto& input = ma_.input();

    const auto begin = index;
    const auto end = index + period;

    auto sum_squared_diff = 0.0;
    for(auto i = begin; i < end; ++i) {
      const auto diff = input[i] - ma_[index];
      sum_squared_diff += diff * diff;
    }

    const auto std_dev = sqrt(sum_squared_diff / period);
    const auto std_dev_scaled = std_dev * stddev_;

    switch(output_) {
    case BbOutput::upper:
      return ma_[index] + std_dev_scaled;
    case BbOutput::lower:
      return ma_[index] - std_dev_scaled;
    case BbOutput::middle:
    default:
      return ma_[index];
    }
  }

  auto size() const noexcept -> std::size_t
  {
    return ma_.size();
  }

  auto output() const noexcept -> BbOutput
  {
    return output_;
  }

  void output(BbOutput output) noexcept
  {
    output_ = output;
  }

private:
  BbOutput output_;
  TMaSeries ma_;
  double stddev_;
};

template<typename TMaSeries>
BbSeries(BbOutput, TMaSeries, double) -> BbSeries<TMaSeries>;

template<typename TSeries>
BbSeries(BbOutput, TSeries, std::size_t, double)
 -> BbSeries<SmaSeries<TSeries>>;

} // namespace pludux

#endif