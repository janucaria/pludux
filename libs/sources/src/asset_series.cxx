module;

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <span>
#include <utility>

export module pludux:asset_series;

export namespace pludux {

class AssetSeries {
public:
  AssetSeries() = default;

  explicit AssetSeries(std::span<const double> data_view)
  : data_view_{data_view}
  {
  }

  auto operator[](this const AssetSeries& self, std::size_t lookback) noexcept
   -> double
  {
    if(lookback >= self.data_view_.size()) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    const auto value_index = self.data_view_.size() - 1 - lookback;
    return self.data_view_[value_index];
  }

  auto size(this const AssetSeries& self) noexcept -> std::size_t
  {
    return self.data_view_.size();
  }

private:
  std::span<const double> data_view_;
};

} // namespace pludux
