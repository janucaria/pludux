#include <pludux/screener/screener_method.hpp>

namespace pludux::screener {

auto ScreenerMethod::operator()(AssetSnapshot asset_snapshot) const
 -> SubSeries<PolySeries<double>>
{
  return impl_->operator()(std::move(asset_snapshot));
}

auto ScreenerMethod::operator==(const ScreenerMethod& other) const noexcept
 -> bool
{
  return impl_->operator==(other);
}

auto ScreenerMethod::operator!=(const ScreenerMethod& other) const noexcept
 -> bool
{
  return impl_->operator!=(other);
}

} // namespace pludux::screener