module;

#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

export module pludux:series.wma_series;

import :series.series_output;

export namespace pludux {

template<typename TSeries>
class WmaSeries {
public:
  using ValueType = typename TSeries::ValueType;

  explicit WmaSeries(TSeries series, std::size_t period)
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

    auto norm = ValueType{0};
    auto sum = ValueType{0};
    for(auto i = begin; i < end; ++i) {
      const auto weight = (self.period_ - (i - begin)) * self.period_;
      sum += self.series_[i] * weight;
      norm += weight;
    }

    return sum / norm;
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
