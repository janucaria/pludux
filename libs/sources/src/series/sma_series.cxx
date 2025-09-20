module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.sma_series;

import :series.series_output;

export namespace pludux {

template<typename TSeries>
class SmaSeries {
public:
  using ValueType = typename TSeries::ValueType;

  SmaSeries(TSeries series, std::size_t period)
  : series_{std::move(series)}
  , period_{period}
  {
  }

  auto operator[](this const auto& self, std::size_t index) noexcept
   -> SeriesOutput<ValueType>
  {
    const auto series_size = self.size();
    const auto nan_index = series_size - self.period_;
    if(index > nan_index) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    const auto begin = index;
    const auto end = index + self.period_;

    auto sum = ValueType{0};
    for(auto i = begin; i < end; ++i) {
      sum += self.series_[i];
    }

    return sum / self.period_;
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
