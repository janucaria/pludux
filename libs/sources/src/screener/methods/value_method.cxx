module;

#include <algorithm>
#include <iterator>
#include <vector>

export module pludux:screener.value_method;

import :asset_snapshot;

export namespace pludux::screener {

class ValueMethod {
public:
  explicit ValueMethod(double value)
  : value_{value}
  {
  }

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    return RepeatSeries{value_, asset_data.size()};
  }

  auto operator==(const ValueMethod& other) const noexcept -> bool = default;

  auto value() const -> double
  {
    return value_;
  }

private:
  double value_;
};

} // namespace pludux::screener
