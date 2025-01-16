#include <pludux/asset.hpp>
#include <pludux/series.hpp>

#include <pludux/screener/screener_method.hpp>

namespace pludux::screener {

auto ScreenerMethod::run_one(const AssetDataProvider& asset_data) const -> double
{
  return impl_->run_one(asset_data);
}

auto ScreenerMethod::run_all(const AssetDataProvider& asset_data) const -> PolySeries<double>
{
  return impl_->run_all(asset_data);
}

} // namespace pludux::screener