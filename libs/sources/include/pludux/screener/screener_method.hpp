#ifndef PLUDUX_PLUDUX_SCREENER_SCREENER_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_SCREENER_METHOD_HPP

#include <functional>
#include <memory>
#include <vector>

#include <pludux/asset.hpp>
#include <pludux/asset_data_provider.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

using ScreenerMethod =
 std::function<auto(const AssetDataProvider&)->PolySeries<double>>;

template<typename T>
auto screener_method_cast(const ScreenerMethod& method) noexcept -> const T*
{
  return method.target<T>();
}

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_SCREENER_METHOD_HPP
