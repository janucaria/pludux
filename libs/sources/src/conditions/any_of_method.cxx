module;

#include <algorithm>
#include <stdexcept>
#include <vector>

export module pludux:conditions.any_of_method;

import :asset_snapshot;
import :method_contextable;
import :conditions.any_condition_method;

export namespace pludux {

class AnyOfMethod {
public:
  AnyOfMethod() = default;

  explicit AnyOfMethod(std::vector<AnyConditionMethod> conditions)
  : conditions_{std::move(conditions)}
  {
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_data,
                  MethodContextable auto context) -> bool
  {
    return std::ranges::any_of(self.conditions_,
                               [&asset_data, &context](const auto& condition) {
                                 return condition(asset_data, context);
                               });
  }

  auto operator==(const AnyOfMethod& other) const noexcept -> bool = default;

  auto conditions(this const auto& self) noexcept
   -> const std::vector<AnyConditionMethod>&
  {
    return self.conditions_;
  }

private:
  std::vector<AnyConditionMethod> conditions_;
};

} // namespace pludux