module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

export module pludux:series.kc_series;

import :series.series_output;

export namespace pludux {

template<typename TRangeSeries, typename TMaSeries>
class KcSeries {
public:
  using ValueType = std::common_type_t<typename TRangeSeries::ValueType,
                                       typename TMaSeries::ValueType>;

  KcSeries(TMaSeries ma, TRangeSeries range, double multiplier)
  : multiplier_{multiplier}
  , ma_{std::move(ma)}
  , range_{std::move(range)}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    const auto range = self.range_[lookback] * self.multiplier_;
    const auto middle = self.ma_[lookback];

    const auto upper = middle + range;
    const auto lower = middle - range;

    return {{middle, upper, lower},
            {
             {OutputName::MiddleBand, 0},
             {OutputName::UpperBand, 1},
             {OutputName::LowerBand, 2},
            }};
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.ma_.size();
  }

private:
  double multiplier_;
  TMaSeries ma_;
  TRangeSeries range_;
};

} // namespace pludux
