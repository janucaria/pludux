module;

#include <cstddef>

export module pludux.series.ref_series;

namespace pludux {

export template<typename TSeries>
class RefSeries {
public:
  using ValueType = typename TSeries::ValueType;

  explicit RefSeries(TSeries& series)
  : series_{series}
  {
  }

  auto operator[](std::size_t index) const -> TSeries::ValueType
  {
    return series_[index];
  }

  auto size() const noexcept -> std::size_t
  {
    return series_.size();
  }

private:
  TSeries& series_;
};

} // namespace pludux
