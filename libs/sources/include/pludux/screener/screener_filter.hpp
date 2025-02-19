#ifndef PLUDUX_PLUDUX_SCREENER_SCREENER_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_SCREENER_FILTER_HPP

#include <functional>
#include <vector>

#include <pludux/asset_snapshot.hpp>

namespace pludux::screener {

using ScreenerFilter = std::function<auto(AssetSnapshot)->bool>;

template<typename T>
auto screener_filter_cast(const ScreenerFilter& filter) noexcept -> const T*
{
  return filter.target<T>();
}

} // namespace pludux::screener

#endif
