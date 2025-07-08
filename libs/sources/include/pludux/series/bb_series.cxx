module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

export module pludux.series.bb_series;

import pludux.series.binary_fn_series;
import pludux.series.ref_series;
import pludux.series.sma_series;

namespace pludux {

export enum class BbOutput { middle, upper, lower };

export template<typename TMaSeries>
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

    const auto std_dev = std::sqrt(sum_squared_diff / period);
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
