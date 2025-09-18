#ifndef PLUDUX_PLUDUX_SCREENER_DATA_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_DATA_METHOD_HPP

#include <cmath>
#include <string>
#include <vector>

#include <pludux/series.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

class DataMethod {
public:
  explicit DataMethod(std::string field);

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>;

  auto operator==(const DataMethod& other) const noexcept -> bool;

  auto field() const -> const std::string&;

private:
  std::string field_{};
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_FIELD_METHOD_HPP
