module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>

export module pludux.series.hma_series;

import pludux.series.repeat_series;
import pludux.series.binary_fn_series;
import pludux.series.ref_series;
import pludux.series.wma_series;

namespace pludux {

export template<typename TSeries>
class HmaSeries {
public:
  using ValueType = typename TSeries::ValueType;

  HmaSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    const auto wam1 = WmaSeries{RefSeries{series_}, period_ / 2};
    const auto scalar_2_series = RepeatSeries{2.0, wam1.size()};
    const auto times_2_wam1 = MultiplySeries{scalar_2_series, wam1};

    const auto wam2 = WmaSeries{RefSeries{series_}, period_};
    const auto diff = SubtractSeries{times_2_wam1, wam2};

    const auto hma =
     WmaSeries{diff, static_cast<std::size_t>(std::sqrt(period_))};

    return hma[index];
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size();
  }

  auto input() const noexcept -> const TSeries&
  {
    return series_;
  }

  auto period() const noexcept -> std::size_t
  {
    return period_;
  }

private:
  TSeries series_;
  std::size_t period_;
};

} // namespace pludux
