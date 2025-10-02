module;

#include <functional>
#include <type_traits>
#include <vector>

export module pludux:screener.comparison_filter;

import :asset_snapshot;
import :series.method_contextable;

import :screener.screener_filter;
import :series.any_method;

export namespace pludux::screener {

template<typename TComparator>
  requires std::is_invocable_r_v<bool, TComparator, double, double>
class ComparisonFilter {
public:
  ComparisonFilter(series::AnyMethod target, series::AnyMethod threshold)
  : target_{std::move(target)}
  , threshold_{std::move(threshold)}
  {
  }

  auto operator==(const ComparisonFilter& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_data,
                  series::MethodContextable auto context) -> bool
  {
    const auto target_result = self.target_(asset_data, context);
    const auto threshold_result = self.threshold_(asset_data, context);

    return TComparator{}(target_result, threshold_result);
  }

  auto target(this const auto& self) noexcept -> const series::AnyMethod&
  {
    return self.target_;
  }

  auto threshold(this const auto& self) noexcept -> const series::AnyMethod&
  {
    return self.threshold_;
  }

private:
  series::AnyMethod target_;
  series::AnyMethod threshold_;
};

using GreaterEqualFilter = ComparisonFilter<std::greater_equal<>>;
using GreaterThanFilter = ComparisonFilter<std::greater<>>;
using LessThanFilter = ComparisonFilter<std::less<>>;
using LessEqualFilter = ComparisonFilter<std::less_equal<>>;
using EqualFilter = ComparisonFilter<std::equal_to<>>;
using NotEqualFilter = ComparisonFilter<std::not_equal_to<>>;

} // namespace pludux::screener
