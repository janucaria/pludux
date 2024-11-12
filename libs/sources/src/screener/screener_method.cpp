#include <pludux/asset.hpp>
#include <pludux/series.hpp>

#include <pludux/screener/screener_method.hpp>

namespace pludux::screener {

auto ScreenerMethod::run_one(const Asset& asset) const -> double
{
  return impl_->run_one(asset);
}

auto ScreenerMethod::run_all(const Asset& asset) const -> Series
{
  return impl_->run_all(asset);
}

} // namespace pludux::screener