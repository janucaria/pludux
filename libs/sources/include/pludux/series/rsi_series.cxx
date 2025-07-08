module;

#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

export module pludux.series.rsi_series;

import pludux.series.rma_series;
import pludux.series.change_series;
import pludux.series.ref_series;
import pludux.series.sma_series;
import pludux.series.unary_fn_series;

namespace pludux {

export template<typename TSeries>
class RsiSeries {
public:
  using ValueType = typename TSeries::ValueType;

  RsiSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    const auto series = RefSeries{series_};
    const auto changes = ChangeSeries{series};

    const auto gains =
     UnaryFnSeries<std::remove_cvref_t<decltype([](auto value) {
                     if(std::isnan(value)) {
                       return std::numeric_limits<ValueType>::quiet_NaN();
                     }
                     return value > 0 ? value : 0;
                   })>,
                   std::remove_cvref_t<decltype(changes)>>{changes};
    const auto losses =
     UnaryFnSeries<std::remove_cvref_t<decltype([](auto value) {
                     if(std::isnan(value)) {
                       return std::numeric_limits<ValueType>::quiet_NaN();
                     }
                     return value < 0 ? -value : 0;
                   })>,
                   std::remove_cvref_t<decltype(changes)>>{changes};

    const auto avg_gain = RmaSeries{gains, period_};
    const auto avg_loss = RmaSeries{losses, period_};

    const auto rs = avg_gain[index] / avg_loss[index];
    const auto rsi = 100 - (100 / (1 + rs));

    return rsi;
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size();
  }

private:
  TSeries series_;
  std::size_t period_;
};

} // namespace pludux
