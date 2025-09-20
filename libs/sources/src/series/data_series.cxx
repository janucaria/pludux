module;

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

export module pludux:series.data_series;

import :series.series_output;

export namespace pludux {

// TODO: can't use deducing this DataSeries becuase of MSVC bug
template<typename T>
  requires std::numeric_limits<T>::has_quiet_NaN
class DataSeries {
public:
  using ValueType = T;

  explicit DataSeries(std::initializer_list<ValueType> data)
  : DataSeries(data.begin(), data.end())
  {
  }

  template<typename TBidirectIt>
  DataSeries(TBidirectIt first, TBidirectIt last)
  : data_{std::make_reverse_iterator(last), std::make_reverse_iterator(first)}
  {
  }

  /**
   * The 0 lookback is the latest value.
   * If the lookback is out of bounds, return NaN.
   */
  auto operator[](std::size_t lookback) const noexcept
   -> SeriesOutput<ValueType>
  {
    if(lookback >= data_.size()) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    const auto value_index = data_.size() - 1 - lookback;
    return data_[value_index];
  }

  auto size() const noexcept -> std::size_t
  {
    return data_.size();
  }

  auto data() const noexcept -> const std::vector<ValueType>&
  {
    return data_;
  }

  void data(std::vector<ValueType> new_data)
  {
    data_ = std::move(new_data);
  }

private:
  std::vector<ValueType> data_;
};

template<typename TBidirectIt>
DataSeries(TBidirectIt, TBidirectIt)
 -> DataSeries<typename std::iterator_traits<TBidirectIt>::value_type>;

} // namespace pludux
