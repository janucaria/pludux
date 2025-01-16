#ifndef PLUDUX_PLUDUX_SCREENER_VALUE_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_VALUE_METHOD_HPP

#include <vector>

#include <pludux/asset.hpp>
#include <pludux/series.hpp>
#include <pludux/asset_data_provider.hpp>

namespace pludux::screener {

class ValueMethod {
public:
  explicit ValueMethod(double value);

  auto operator()(const AssetDataProvider& asset_data) const -> PolySeries<double>;

  auto value() const -> double;

private:
  double value_;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_VALUE_METHOD_HPP
