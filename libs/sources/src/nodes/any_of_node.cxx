module;

#include <initializer_list>
#include <utility>
#include <vector>

export module pludux:nodes.any_of_node;

import :methods.any_of_method;
import :node_to_erased_method;
import :nodes.erased_node;

export namespace pludux {

template<typename TContext>
class AnyOfNode final {
public:
  AnyOfNode(
    std::initializer_list<ErasedNode<TContext>> conditions)
  : conditions_{conditions}
  {
  }

  explicit AnyOfNode(
    std::vector<ErasedNode<TContext>> conditions)
  : conditions_{std::move(conditions)}
  {
  }

  auto operator==(const AnyOfNode& other) const noexcept -> bool = default;

  auto conditions(this const AnyOfNode& self) noexcept
    -> const std::vector<ErasedNode<TContext>>&
  {
    return self.conditions_;
  }

  void conditions(
   this AnyOfNode& self,
    std::vector<ErasedNode<TContext>> conditions) noexcept
  {
    self.conditions_ = std::move(conditions);
  }

private:
   std::vector<ErasedNode<TContext>> conditions_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                         const AnyOfNode<TContext>& node,
                        NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  auto conditions = std::vector<ErasedSeriesMethod<TContext>>{};
  conditions.reserve(node.conditions().size());
  for(const auto& condition : node.conditions()) {
    conditions.emplace_back(
     node_to_erased_method<TContext>(condition, context));
  }

  return ErasedSeriesMethod<TContext>{
   AnyOfMethod<ErasedSeriesMethod<TContext>>{std::move(conditions)}};
}

} // namespace pludux
