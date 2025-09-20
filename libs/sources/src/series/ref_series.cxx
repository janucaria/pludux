module;

#include <cstddef>

export module pludux:series.ref_series;

import :series.series_output;

export namespace pludux {

template<typename TSeries>
class RefSeries {
public:
  using ValueType = typename TSeries::ValueType;

  explicit RefSeries(TSeries& series)
  : series_{series}
  {
  }

  auto operator[](this const auto self, std::size_t index)
   -> SeriesOutput<ValueType>
  {
    return self.series_[index];
  }

  auto size(this const auto self) noexcept -> std::size_t
  {
    return self.series_.size();
  }

private:
  TSeries& series_;
};

} // namespace pludux
