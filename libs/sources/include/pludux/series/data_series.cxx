module;
#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

export module pludux.series.data_series;

namespace pludux2 {

export template<typename T>
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
   * The 0 reverse_index is the latest value.
   * If the reverse_index is out of bounds, return NaN.
   */
  auto operator[](std::size_t index) const -> ValueType
  {
    if(index >= data_.size()) {
      return std::numeric_limits<ValueType>::quiet_NaN();
    }

    const auto value_index = data_.size() - 1 - index;
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

} // namespace pludux2
