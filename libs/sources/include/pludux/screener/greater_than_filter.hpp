#ifndef PLUDUX_PLUDUX_SCREENER_GREATER_THAN_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_GREATER_THAN_FILTER_HPP

#include <vector>

#include <pludux/asset.hpp>
#include <pludux/screener/screener_filter.hpp>
#include <pludux/screener/screener_method.hpp>

namespace pludux::screener {

class GreaterThanFilter {
public:
  GreaterThanFilter(ScreenerMethod target, ScreenerMethod threshold);

  auto operator()(const Asset& asset) const -> bool;

private:
  ScreenerMethod target_;
  ScreenerMethod threshold_;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_GREATER_THAN_FILTER_HPP
