#ifndef PLUDUX_PLUDUX_SCREENER_All_OF_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_All_OF_FILTER_HPP

#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_filter.hpp>

namespace pludux::screener {

class AllOfFilter {
public:
  AllOfFilter();

  explicit AllOfFilter(std::vector<ScreenerFilter> filters);

  auto operator()(AssetSnapshot asset_data) const -> bool;

  auto filters() const -> const std::vector<ScreenerFilter>&;

private:
  std::vector<ScreenerFilter> filters_;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_All_OF_FILTER_HPP
