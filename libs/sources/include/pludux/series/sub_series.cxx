module;

#include <cstddef>
#include <utility>
#include <limits>

export module pludux.series.sub_series;

namespace pludux {

export template<typename TSeries>
class SubSeries {
public:
  using ValueType = typename TSeries::ValueType;

  SubSeries(TSeries series, std::ptrdiff_t shifted_offset)
  : series_{std::move(series)}
  , shifted_offset_{shifted_offset}
  {
  }

  SubSeries(SubSeries<TSeries> series, std::ptrdiff_t shifted_offset)
  : SubSeries{series.series_, series.shifted_offset_ + shifted_offset}
  {
  }

  template<typename USeries>
  SubSeries(SubSeries<USeries> other_series, std::ptrdiff_t shifted_offset)
  : SubSeries{other_series.series(),
              other_series.shifted_offset() + shifted_offset}
  {
  }

  auto operator[](std::size_t index) const -> TSeries::ValueType
  {
    if(index >= series_.size()) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    return series_[index + shifted_offset_];
  }

  auto shifted_offset() const noexcept -> std::ptrdiff_t
  {
    return shifted_offset_;
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size() - shifted_offset_;
  }

  auto series() const noexcept -> const TSeries&
  {
    return series_;
  }

private:
  TSeries series_;
  std::ptrdiff_t shifted_offset_;
};

} // namespace pludux
