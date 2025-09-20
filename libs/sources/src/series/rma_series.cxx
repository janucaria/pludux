module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

export module pludux:series.rma_series;

import :series.series_output;
import :series.sma_series;

export namespace pludux {

template<typename TSeries>
class RmaSeries {
public:
  using ValueType = typename TSeries::ValueType;

  RmaSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    const auto series_size = self.size();

    const auto alpha = 1.0 / self.period_;
    const auto sma_index = series_size - self.period_;
    auto rma_value = std::numeric_limits<ValueType>::quiet_NaN();
    for(auto ii = series_size; ii > lookback; --ii) {
      const auto i = ii - 1;
      if(i > sma_index) {
        continue;
      }

      if(std::isnan(rma_value)) {
        rma_value = SmaSeries{self.series_, self.period_}[i];
        continue;
      }

      const auto series_value = self.series_[i];
      rma_value = series_value * alpha + rma_value * (1 - alpha);
    }

    return rma_value;
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
