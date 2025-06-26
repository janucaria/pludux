#ifndef PLUDUX_PLUDUX_SCREENER_BOOLEAN_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_BOOLEAN_FILTER_HPP

#include <pludux/asset_snapshot.hpp>

namespace pludux::screener {

namespace detail {

template<typename, bool boolean_value>
struct BooleanFilter {
  auto operator()(AssetSnapshot) const -> bool
  {
    return boolean_value;
  }

  auto operator==(const BooleanFilter&) const noexcept -> bool = default;
};

} // namespace detail

struct TrueFilter : detail::BooleanFilter<TrueFilter, true> {};

struct FalseFilter : detail::BooleanFilter<FalseFilter, false> {};

} // namespace pludux::screener

#endif
