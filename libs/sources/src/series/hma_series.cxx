module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.hma_series;

import :series.series_output;
import :series.ref_series;
import :series.repeat_series;
import :series.wma_series;
import :series.operators_series;

export namespace pludux {

template<typename TSeries>
class HmaSeries {
public:
  using ValueType = typename TSeries::ValueType;

  HmaSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    const auto wam1 = WmaSeries{RefSeries{self.series_}, self.period_ / 2};
    const auto scalar_2_series = RepeatSeries{2.0, wam1.size()};
    const auto times_2_wam1 = MultiplySeries{scalar_2_series, wam1};

    const auto wam2 = WmaSeries{RefSeries{self.series_}, self.period_};
    const auto diff = SubtractSeries{times_2_wam1, wam2};

    const auto hma =
     WmaSeries{diff, static_cast<std::size_t>(std::sqrt(self.period_))};

    return hma[lookback];
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.series_.size();
  }

  auto input(this const auto& self) noexcept -> const TSeries&
  {
    return self.series_;
  }

  auto period(this const auto& self) noexcept -> std::size_t
  {
    return self.period_;
  }

private:
  TSeries series_;
  std::size_t period_;
};

} // namespace pludux
