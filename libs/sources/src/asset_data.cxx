module;

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <utility>
#include <vector>

export module pludux:asset_data;

export namespace pludux {

class AssetData {
public:
  AssetData() = default;

  AssetData(std::initializer_list<double> data)
  : AssetData(data.begin(), data.end())
  {
  }

  template<typename TBidirectIt>
  AssetData(TBidirectIt first, TBidirectIt last)
  : data_{std::make_reverse_iterator(last), std::make_reverse_iterator(first)}
  {
  }

  /**
   * The 0 lookback is the latest value.
   * If the lookback is out of bounds, return NaN.
   */
  auto operator[](this const auto& self, std::size_t lookback) noexcept
   -> double
  {
    if(lookback >= self.data_.size()) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const auto value_index = self.data_.size() - 1 - lookback;
    return self.data_[value_index];
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.data_.size();
  }

  auto data(this const auto& self) noexcept -> const std::vector<double>&
  {
    return self.data_;
  }

  void data(this auto& self, std::vector<double> new_data)
  {
    self.data_ = std::move(new_data);
  }

private:
  std::vector<double> data_;
};

} // namespace pludux
