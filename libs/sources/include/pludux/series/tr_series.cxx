module;

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux.series.tr_series;

namespace pludux {

export template<typename THighSeries,
                typename TLowSeries,
                typename TCloseSeries>
class TrSeries {
public:
  using ValueType = std::common_type_t<typename THighSeries::ValueType,
                                       typename TLowSeries::ValueType,
                                       typename TCloseSeries::ValueType>;

  TrSeries(THighSeries highs, TLowSeries lows, TCloseSeries closes)
  : highs_{std::move(highs)}
  , lows_{std::move(lows)}
  , closes_{std::move(closes)}
  {
  }

  auto operator[](std::size_t index) const noexcept -> ValueType
  {
    const auto high = highs_[index];
    const auto low = lows_[index];

    const auto close_last_index = closes_.size() - 1;
    const auto prev_close =
     index == close_last_index ? closes_[index] : closes_[index + 1];

    const auto hl = std::abs(high - low);
    const auto hc = std::abs(high - prev_close);
    const auto lc = std::abs(low - prev_close);

    return std::max(std::max(hl, hc), lc);
  }

  auto size() const noexcept -> std::size_t
  {
    assert(highs_.size() == lows_.size());
    assert(highs_.size() == closes_.size());

    return highs_.size();
  }

private:
  THighSeries highs_;
  TLowSeries lows_;
  TCloseSeries closes_;
};

} // namespace pludux
