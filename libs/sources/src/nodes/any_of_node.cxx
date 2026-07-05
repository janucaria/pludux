module;

#include <initializer_list>
#include <utility>
#include <vector>

export module pludux:nodes.any_of_node;

import :methods.any_of_method;
import :node_to_erased_method;
import :nodes.erased_node;

export namespace pludux {

class AnyOfNode {
public:
  AnyOfNode(std::initializer_list<ErasedNode> conditions)
  : conditions_{conditions}
  {
  }

  explicit AnyOfNode(std::vector<ErasedNode> conditions)
  : conditions_{std::move(conditions)}
  {
  }

  auto operator==(const AnyOfNode& other) const noexcept -> bool = default;

  auto conditions(this const AnyOfNode& self) noexcept
   -> const std::vector<ErasedNode>&
  {
    return self.conditions_;
  }

  void conditions(this AnyOfNode& self,
                  std::vector<ErasedNode> conditions) noexcept
  {
    self.conditions_ = std::move(conditions);
  }

private:
  std::vector<ErasedNode> conditions_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const AnyOfNode& node,
                       NodeToErasedMethodContext& context) -> AnySeriesMethod
{
  auto conditions = std::vector<AnySeriesMethod>{};
  conditions.reserve(node.conditions().size());
  for(const auto& condition : node.conditions()) {
    conditions.emplace_back(node_to_erased_method(condition, context));
  }

  return AnySeriesMethod{AnyOfMethod<AnySeriesMethod>{std::move(conditions)}};
}

} // namespace pludux
