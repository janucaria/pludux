module;

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <variant>
#include <vector>

export module pludux:conditions.all_of_method;

import :asset_snapshot;
import :series.method_contextable;
import :conditions.any_condition_method;

export namespace pludux {

class AllOfMethod {
public:
  AllOfMethod() = default;

  explicit AllOfMethod(std::vector<AnyConditionMethod> filters)
  : conditions_{std::move(filters)}
  {
  }

  auto operator==(const AllOfMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) -> bool
  {
    return std::ranges::all_of(self.conditions_,
                               [&asset_snapshot, &context](const auto& filter) {
                                 return filter(asset_snapshot, context);
                               });
  }

  auto conditions(this const auto& self) noexcept
   -> const std::vector<AnyConditionMethod>&
  {
    return self.conditions_;
  }

private:
  std::vector<AnyConditionMethod> conditions_;
};

} // namespace pludux