#include <pludux/screener/screener_filter.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

auto ScreenerFilter::operator()(AssetSnapshot asset_snapshot) const noexcept
 -> bool
{
  return impl_->operator()(std::move(asset_snapshot));
}

auto ScreenerFilter::operator==(const ScreenerFilter& other) const noexcept
 -> bool
{
  return impl_->operator==(other);
}

auto ScreenerFilter::operator!=(const ScreenerFilter& other) const noexcept
 -> bool
{
  return impl_->operator!=(other);
}

} // namespace pludux::screener