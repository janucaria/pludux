module;

#include <cstddef>

export module pludux.screener.value_method;

import pludux.asset_snapshot;
import pludux.series;

namespace pludux::screener {

export class ValueMethod {
public:
  explicit ValueMethod(double value, std::size_t offset = 0)
  : value_{value}
  , offset_{offset}
  {
  }

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>
  {
    return SubSeries{
     PolySeries<double>{RepeatSeries{value_, asset_data.size()}},
     static_cast<std::ptrdiff_t>(offset_)};
  }

  auto operator==(const ValueMethod& other) const noexcept -> bool = default;

  auto value() const -> double
  {
    return value_;
  }

private:
  double value_;
  std::size_t offset_;
};

} // namespace pludux::screener