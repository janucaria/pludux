module;

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <variant>
#include <vector>

export module pludux:screener.all_of_filter;

import :asset_snapshot;
import :series.method_call_context;
import :screener.screener_filter;

export namespace pludux::screener {

class AllOfFilter {
public:
  AllOfFilter() = default;

  explicit AllOfFilter(std::vector<ScreenerFilter> filters)
  : conditions_{std::move(filters)}
  {
  }

  auto operator==(const AllOfFilter& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  series::MethodCallContext<double> auto context)
   -> bool
  {
    return std::ranges::all_of(self.conditions_,
                               [&asset_snapshot, &context](const auto& filter) {
                                 return filter(asset_snapshot, context);
                               });
  }

  auto conditions(this const auto& self) noexcept
   -> const std::vector<ScreenerFilter>&
  {
    return self.conditions_;
  }

private:
  std::vector<ScreenerFilter> conditions_;
};

} // namespace pludux::screener