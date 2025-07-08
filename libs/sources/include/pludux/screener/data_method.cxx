module;

#include <algorithm>
#include <cmath>
#include <format>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

export module pludux.screener.data_method;

import pludux.asset_snapshot;
import pludux.series;

namespace pludux::screener {

export class DataMethod {
public:
  explicit DataMethod(std::string field, std::size_t offset = 0)
  : field_{std::move(field)}
  , offset_{offset}
  {
  }

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>
  {
    if(!asset_data.contains(field_)) {
      return SubSeries{
       PolySeries<double>{RepeatSeries{std::numeric_limits<double>::quiet_NaN(),
                                       asset_data.size()}},
       static_cast<std::ptrdiff_t>(offset_)};
    }
    return SubSeries<PolySeries<double>>{asset_data.get_values(field_),
                                         static_cast<std::ptrdiff_t>(offset_)};
  }

  auto operator==(const DataMethod& other) const noexcept -> bool = default;

  auto field() const -> const std::string&
  {
    return field_;
  }

  auto offset() const -> int
  {
    return offset_;
  }

private:
  std::string field_{};
  std::size_t offset_{};
};

} // namespace pludux::screener
