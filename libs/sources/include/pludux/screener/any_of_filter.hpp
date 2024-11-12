#ifndef PLUDUX_PLUDUX_SCREENER_ANY_OF_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_ANY_OF_FILTER_HPP

#include <vector>

#include <pludux/asset.hpp>
#include <pludux/screener/screener_filter.hpp>

namespace pludux::screener {

class AnyOfFilter {
public:
  AnyOfFilter();

  explicit AnyOfFilter(std::vector<ScreenerFilter> filters);

  auto operator()(const Asset& asset) const -> bool;

private:
  std::vector<ScreenerFilter> filters_;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_ANY_OF_FILTER_HPP
