module;

#include <algorithm>
#include <cmath>
#include <format>
#include <iterator>
#include <string>
#include <vector>

export module pludux.screener.data_method;

import pludux.asset_snapshot;

export namespace pludux::screener {

class DataMethod {
public:
  explicit DataMethod(std::string field)
  : field_{std::move(field)}
  {
  }

  auto operator==(const DataMethod& other) const noexcept -> bool = default;

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    if(!asset_data.contains(field_)) {
      return RepeatSeries{std::numeric_limits<double>::quiet_NaN(),
                          asset_data.size()};
    }
    return asset_data.get_values(field_);
  }

  auto field() const -> const std::string&
  {
    return field_;
  }

private:
  std::string field_{};
};

} // namespace pludux::screener
