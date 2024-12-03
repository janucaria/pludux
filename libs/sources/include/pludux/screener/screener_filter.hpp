#ifndef PLUDUX_PLUDUX_SCREENER_SCREENER_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_SCREENER_FILTER_HPP

#include <functional>
#include <vector>

#include <pludux/asset.hpp>
#include <pludux/screener/asset_data_provider.hpp>

namespace pludux::screener {

using ScreenerFilter = std::function<auto(const AssetDataProvider&)->bool>;

template<typename T>
auto screener_filter_cast(const ScreenerFilter& filter) noexcept -> const T*
{
  return filter.target<T>();
}

} // namespace pludux::screener

#endif
