module;

#include <cstddef>
#include <initializer_list>
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
  : data_{first, last}
  {
  }

  /**
   * If the index is out of bounds, return NaN.
   */
  auto operator[](this const AssetData& self, std::size_t index) noexcept
   -> double
  {
    if(index >= self.data_.size()) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    return self.data_[index];
  }

  auto size(this const AssetData& self) noexcept -> std::size_t
  {
    return self.data_.size();
  }

  auto data(this const AssetData& self) noexcept -> const std::vector<double>&
  {
    return self.data_;
  }

  void data(this AssetData& self, std::vector<double> new_data)
  {
    self.data_ = std::move(new_data);
  }

private:
  std::vector<double> data_;
};

} // namespace pludux
