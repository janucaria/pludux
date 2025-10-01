module;

#include <algorithm>
#include <stdexcept>
#include <vector>

export module pludux:screener.any_of_filter;

import :asset_snapshot;
import :screener.method_call_context;
import :screener.screener_filter;

export namespace pludux::screener {

class AnyOfFilter {
public:
  AnyOfFilter() = default;

  explicit AnyOfFilter(std::vector<ScreenerFilter> conditions)
  : conditions_{std::move(conditions)}
  {
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_data,
                  MethodCallContext<double> auto context) -> bool
  {
    return std::ranges::any_of(self.conditions_,
                               [&asset_data, &context](const auto& filter) {
                                 return filter(asset_data, context);
                               });
  }

  auto operator==(const AnyOfFilter& other) const noexcept -> bool = default;

  auto conditions(this const auto& self) noexcept
   -> const std::vector<ScreenerFilter>&
  {
    return self.conditions_;
  }

private:
  std::vector<ScreenerFilter> conditions_;
};

} // namespace pludux::screener