module;

#include <algorithm>
#include <cmath>
#include <format>
#include <iterator>
#include <string>
#include <vector>

export module pludux:screener.data_method;

import :asset_snapshot;

export namespace pludux::screener {

class DataMethod {
public:
  explicit DataMethod(std::string field)
  : field_{std::move(field)}
  {
  }

  auto operator==(const DataMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data) -> PolySeries<double>
  {
    if(!asset_data.contains(self.field_)) {
      return RepeatSeries{std::numeric_limits<double>::quiet_NaN(),
                          asset_data.size()};
    }
    return asset_data.get_values(self.field_);
  }

  auto field(this const auto& self) -> const std::string&
  {
    return self.field_;
  }

private:
  std::string field_{};
};

} // namespace pludux::screener
