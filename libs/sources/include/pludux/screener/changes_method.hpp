#ifndef PLUDUX_PLUDUX_SCREENER_CHANGES_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_CHANGES_METHOD_HPP

#include <pludux/asset_data_provider.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class ChangesMethod {
public:
  explicit ChangesMethod(ScreenerMethod operand);

  auto run_one(const AssetDataProvider& asset_data) const -> double;

  auto run_all(const AssetDataProvider& asset_data) const -> Series;

private:
  ScreenerMethod operand_;
};

} // namespace pludux::screener

#endif
