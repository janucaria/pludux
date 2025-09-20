module;

#include <cstddef>
#include <limits>

export module pludux:series.repeat_series;

import :series.series_output;

export namespace pludux {

template<typename TValue>
  requires std::numeric_limits<TValue>::has_quiet_NaN
class RepeatSeries {
public:
  using ValueType = TValue;

  RepeatSeries(TValue value, std::size_t size)
  : value_{value}
  , size_{size}
  {
  }

  // TODO: can't use SeriesOutput here because of MSVC bug
  auto operator[](this const auto self, std::size_t index) -> ValueType
  {
    if(index >= self.size()) {
      return std::numeric_limits<TValue>::quiet_NaN();
    }

    return self.value_;
  }

  auto size(this const auto self) noexcept -> std::size_t
  {
    return self.size_;
  }

private:
  TValue value_;
  std::size_t size_;
};

} // namespace pludux
