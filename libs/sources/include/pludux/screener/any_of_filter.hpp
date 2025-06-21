#ifndef PLUDUX_PLUDUX_SCREENER_ANY_OF_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_ANY_OF_FILTER_HPP

#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_filter.hpp>

namespace pludux::screener {

class AnyOfFilter {
public:
  AnyOfFilter();

  explicit AnyOfFilter(std::vector<ScreenerFilter> filters);

  auto operator()(AssetSnapshot asset_data) const -> bool;

  auto operator==(const AnyOfFilter& other) const noexcept-> bool;

  auto filters() const -> const std::vector<ScreenerFilter>&;

private:
  std::vector<ScreenerFilter> conditions_;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_ANY_OF_FILTER_HPP
