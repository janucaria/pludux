module;

#include <cstddef>
#include <limits>
#include <span>

export module pludux:asset_series;

export namespace pludux {

class AssetSeries {
public:
  AssetSeries() = default;

  explicit AssetSeries(std::span<const double> data_view)
  : data_view_{data_view}
  {
  }

  auto operator[](this const AssetSeries& self, std::size_t index) noexcept
   -> double
  {
    if(index >= self.data_view_.size()) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    return self.data_view_[index];
  }

  auto size(this const AssetSeries& self) noexcept -> std::size_t
  {
    return self.data_view_.size();
  }

private:
  std::span<const double> data_view_;
};

} // namespace pludux
