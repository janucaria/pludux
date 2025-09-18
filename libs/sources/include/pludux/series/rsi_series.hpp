#ifndef PLUDUX_PLUDUX_SERIES_RSI_SERIES_HPP
#define PLUDUX_PLUDUX_SERIES_RSI_SERIES_HPP

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

#include "rma_series.hpp"
#include "unary_fn_series.hpp"

import pludux.series.series_output;
import pludux.series.ref_series;
import pludux.series.change_series;

namespace pludux {

template<typename TSeries>
class RsiSeries {
public:
  using ValueType = typename TSeries::ValueType;

  RsiSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  {
  }

  auto operator[](std::size_t index) const noexcept -> SeriesOutput<ValueType>
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

#endif