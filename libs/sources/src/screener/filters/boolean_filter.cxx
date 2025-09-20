export module pludux:screener.boolean_filter;

import :asset_snapshot;

namespace pludux::screener {

template<typename, bool boolean_value>
struct BooleanFilter {
  auto operator==(const BooleanFilter&) const noexcept -> bool = default;

  auto operator()(this const auto, AssetSnapshot) -> bool
  {
    return boolean_value;
  }
};

export struct TrueFilter : BooleanFilter<TrueFilter, true> {};

export struct FalseFilter : BooleanFilter<FalseFilter, false> {};

} // namespace pludux::screener
