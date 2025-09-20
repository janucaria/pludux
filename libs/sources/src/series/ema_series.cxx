module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>
export module pludux:series.ema_series;

import :series.series_output;
import :series.sma_series;

export namespace pludux {

template<typename TSeries>
class EmaSeries {
public:
  using ValueType = typename TSeries::ValueType;

  EmaSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    const auto series_size = self.size();
    const auto alpha = 2.0 / (self.period_ + 1);
    const auto sma_index = series_size - self.period_;
    auto ema_value = std::numeric_limits<ValueType>::quiet_NaN();
    for(auto ii = series_size; ii > lookback; --ii) {
      const auto i = ii - 1;
      if(i > sma_index) {
        continue;
      }

      if(std::isnan(ema_value)) {
        ema_value = SmaSeries{self.series_, self.period_}[i];
        continue;
      }

      const auto series_value = self.series_[i];
      ema_value = series_value * alpha + ema_value * (1 - alpha);
    }

    return ema_value;
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
