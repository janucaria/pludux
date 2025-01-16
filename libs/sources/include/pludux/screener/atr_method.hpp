#ifndef PLUDUX_PLUDUX_SCREENER_ATR_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_ATR_METHOD_HPP

#include <pludux/asset_data_provider.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class AtrMethod {
public:
  explicit AtrMethod(int period);

  auto operator()(const AssetDataProvider& asset_data) const -> PolySeries<double>;

private:
  int period_;
};

} // namespace pludux::screener

#endif
