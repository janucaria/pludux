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

  auto operator==(const ValueMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    return RepeatSeries{self.value_, asset_data.size()};
  }

  auto value(this const auto& self) noexcept -> double
  {
    return self.value_;
  }

private:
  double value_;
};

} // namespace pludux::screener
