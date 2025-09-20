module;

#include <functional>
#include <type_traits>
#include <vector>

export module pludux:screener.comparison_filter;

import :asset_snapshot;
import :screener.screener_filter;
import :screener.screener_method;

export namespace pludux::screener {

template<typename TComparator>
  requires std::is_invocable_r_v<bool, TComparator, double, double>
class ComparisonFilter {
public:
  ComparisonFilter(ScreenerMethod target, ScreenerMethod threshold)
  : target_{std::move(target)}
  , threshold_{std::move(threshold)}
  {
  }

  auto operator==(const ComparisonFilter& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data) -> bool
  {
    const auto target_result = self.target_(asset_data)[0];
    const auto threshold_result = self.threshold_(asset_data)[0];

    return TComparator{}(target_result, threshold_result);
  }

  auto target(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.target_;
  }

  auto threshold(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.threshold_;
  }

private:
  ScreenerMethod target_;
  ScreenerMethod threshold_;
};

using GreaterEqualFilter = ComparisonFilter<std::greater_equal<>>;
using GreaterThanFilter = ComparisonFilter<std::greater<>>;
using LessThanFilter = ComparisonFilter<std::less<>>;
using LessEqualFilter = ComparisonFilter<std::less_equal<>>;
using EqualFilter = ComparisonFilter<std::equal_to<>>;
using NotEqualFilter = ComparisonFilter<std::not_equal_to<>>;

} // namespace pludux::screener
