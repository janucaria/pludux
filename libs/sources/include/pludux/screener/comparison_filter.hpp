#ifndef PLUDUX_PLUDUX_SCREENER_COMPARISON_FILTER_HPP
#define PLUDUX_PLUDUX_SCREENER_COMPARISON_FILTER_HPP

#include <vector>

#include <pludux/asset.hpp>
#include <pludux/screener/screener_filter.hpp>
#include <pludux/screener/screener_method.hpp>

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

  auto operator()(const Asset& asset) const -> bool
  {
    const auto target_result = target_.run_one(asset);
    const auto threshold_result = threshold_.run_one(asset);

    return compare_(target_result, threshold_result);
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
