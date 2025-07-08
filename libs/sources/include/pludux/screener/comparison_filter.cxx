module;

#include <functional>
#include <vector>

export module pludux.screener.comparison_filter;

import pludux.asset_snapshot;
import pludux.screener.screener_method;
import pludux.screener.screener_filter;

namespace pludux::screener {

export template<typename T>
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

export using GreaterEqualFilter = ComparisonFilter<std::greater_equal<>>;
export using GreaterThanFilter = ComparisonFilter<std::greater<>>;
export using LessThanFilter = ComparisonFilter<std::less<>>;
export using LessEqualFilter = ComparisonFilter<std::less_equal<>>;
export using EqualFilter = ComparisonFilter<std::equal_to<>>;
export using NotEqualFilter = ComparisonFilter<std::not_equal_to<>>;

} // namespace pludux::screener
