module;

#include <initializer_list>
#include <utility>
#include <vector>

export module pludux:nodes.all_of_node;

import :methods.all_of_method;
import :node_to_erased_method;
import :nodes.erased_node;

export namespace pludux {

class AllOfNode {
public:
  AllOfNode() = default;

  AllOfNode(std::initializer_list<ErasedNode> conditions)
  : conditions_{conditions}
  {
  }

  explicit AllOfNode(std::vector<ErasedNode> conditions)
  : conditions_{std::move(conditions)}
  {
  }

  auto operator==(const AllOfNode& other) const noexcept -> bool = default;

  auto conditions(this const AllOfNode& self) noexcept
   -> const std::vector<ErasedNode>&
  {
    return self.conditions_;
  }

  void conditions(this AllOfNode& self,
                  std::vector<ErasedNode> conditions) noexcept
  {
    self.conditions_ = std::move(conditions);
  }

private:
  std::vector<ErasedNode> conditions_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const AllOfNode& node,
                       NodeToErasedMethodContext& context) -> AnySeriesMethod
{
  auto conditions = std::vector<AnySeriesMethod>{};
  conditions.reserve(node.conditions().size());
  for(const auto& condition : node.conditions()) {
    conditions.emplace_back(node_to_erased_method(condition, context));
  }

  return AnySeriesMethod{AllOfMethod<AnySeriesMethod>{std::move(conditions)}};
}

} // namespace pludux
