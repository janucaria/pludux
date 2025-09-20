module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.change_series;

import :series.series_output;

export namespace pludux {

template<typename TSeries>
class ChangeSeries {
public:
  using ValueType = typename TSeries::ValueType;

  explicit ChangeSeries(TSeries series)
  : series_{std::move(series)}
  {
  }

  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> SeriesOutput<ValueType>
  {
    return self.series_[lookback] - self.series_[lookback + 1];
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.series_.size();
  }

private:
  TSeries series_;
};

} // namespace pludux
