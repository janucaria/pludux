#ifndef PLUDUX_PLUDUX_SCREENER_ATR_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_ATR_METHOD_HPP

#include <pludux/asset_data_provider.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class AtrMethod {
public:
  explicit AtrMethod(int period);

  auto run_one(const AssetDataProvider& asset_data) const -> double;

  auto run_all(const AssetDataProvider& asset_data) const -> Series;

private:
  int period_;
};

} // namespace pludux::screener

#endif
