#ifndef PLUDUX_PLUDUX_SCREENER_COMPARISON_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_COMPARISON_FILTER_HPP

#include <vector>

#include <pludux/screener/screener_method.hpp>

import pludux.asset_snapshot;
import pludux.screener.screener_filter;

namespace pludux::screener {

template<typename T>
class ComparisonFilter {
public:
  ComparisonFilter(ScreenerMethod target, ScreenerMethod threshold)
  : compare_{}
  , target_{std::move(target)}
  , threshold_{std::move(threshold)}
  {
  }

  auto operator()(AssetSnapshot asset_data) const -> bool
  {
    const auto target_result = target_(asset_data)[0];
    const auto threshold_result = threshold_(asset_data)[0];

    return compare_(target_result, threshold_result);
  }

  auto operator==(const ComparisonFilter& other) const noexcept -> bool
  {
    return target_ == other.target_ && threshold_ == other.threshold_;
  }

  auto operator!=(const ComparisonFilter& other) const noexcept
   -> bool = default;

  auto target() const -> const ScreenerMethod&
  {
    return target_;
  }

  auto threshold() const -> const ScreenerMethod&
  {
    return threshold_;
  }

private:
  T compare_;
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

#endif // PLUDUX_PLUDUX_SCREENER_GREATER_EQUAL_FILTER_HPP
