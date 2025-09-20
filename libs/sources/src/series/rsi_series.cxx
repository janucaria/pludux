module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

export module pludux:series.rsi_series;

import :series.series_output;
import :series.ref_series;
import :series.change_series;
import :series.rma_series;
import :series.operators_series;

export namespace pludux {

template<typename TSeries>
class RsiSeries {
public:
  using ValueType = typename TSeries::ValueType;

  RsiSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    const auto series = RefSeries{self.series_};
    const auto changes = ChangeSeries{series};

    const auto gains =
     UnaryOpSeries<std::remove_cvref_t<decltype([](auto value) {
                     if(std::isnan(value)) {
                       return std::numeric_limits<ValueType>::quiet_NaN();
                     }
                     return value > 0 ? value : 0;
                   })>,
                   std::remove_cvref_t<decltype(changes)>>{changes};
    const auto losses =
     UnaryOpSeries<std::remove_cvref_t<decltype([](auto value) {
                     if(std::isnan(value)) {
                       return std::numeric_limits<ValueType>::quiet_NaN();
                     }
                     return value < 0 ? -value : 0;
                   })>,
                   std::remove_cvref_t<decltype(changes)>>{changes};

    const auto avg_gain = RmaSeries{gains, self.period_};
    const auto avg_loss = RmaSeries{losses, self.period_};

    const auto rs = avg_gain[lookback] / avg_loss[lookback];
    const auto rsi = 100 - (100 / (1 + rs));

    return rsi;
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.series_.size();
  }

private:
  TSeries series_;
  std::size_t period_;
};

} // namespace pludux
