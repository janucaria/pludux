module;

#include <functional>
#include <type_traits>
#include <vector>

export module pludux:conditions.comparison_method;

import :asset_snapshot;
import :series.method_contextable;

import :conditions.any_condition_method;
import :series.any_series_method;

export namespace pludux {

template<typename TComparator>
  requires std::is_invocable_r_v<bool, TComparator, double, double>
class ComparisonMethod {
public:
  ComparisonMethod(AnySeriesMethod target, AnySeriesMethod threshold)
  : target_{std::move(target)}
  , threshold_{std::move(threshold)}
  {
  }

  auto operator==(const ComparisonMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_data,
                  MethodContextable auto context) -> bool
  {
    const auto target_result = self.target_(asset_data, context);
    const auto threshold_result = self.threshold_(asset_data, context);

    return TComparator{}(target_result, threshold_result);
  }

  auto target(this const auto& self) noexcept -> const AnySeriesMethod&
  {
    return self.target_;
  }

  auto threshold(this const auto& self) noexcept -> const AnySeriesMethod&
  {
    return self.threshold_;
  }

private:
  AnySeriesMethod target_;
  AnySeriesMethod threshold_;
};

using GreaterEqualMethod = ComparisonMethod<std::greater_equal<>>;
using GreaterThanMethod = ComparisonMethod<std::greater<>>;
using LessThanMethod = ComparisonMethod<std::less<>>;
using LessEqualMethod = ComparisonMethod<std::less_equal<>>;
using EqualMethod = ComparisonMethod<std::equal_to<>>;
using NotEqualMethod = ComparisonMethod<std::not_equal_to<>>;

} // namespace pludux
