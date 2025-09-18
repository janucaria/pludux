#ifndef PLUDUX_PLUDUX_SCREENER_VALUE_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_VALUE_METHOD_HPP

#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/series.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

class ValueMethod {
public:
  explicit ValueMethod(double value);

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>;

  auto operator==(const ValueMethod& other) const noexcept -> bool;

  auto value() const -> double;

private:
  double value_;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_VALUE_METHOD_HPP
