module;

export module pludux.screener.boolean_filter;

import pludux.asset_snapshot;

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

export struct TrueFilter : detail::BooleanFilter<TrueFilter, true> {};

export struct FalseFilter : detail::BooleanFilter<FalseFilter, false> {};

} // namespace pludux::screener
