module;

#include <functional>
#include <utility>

export module pludux:nodes.comparison_node;

import :methods.comparison_method;
import :node_to_erased_method;
import :nodes.erased_node;

export namespace pludux {

template<typename TComparator>
class ComparisonNode {
public:
  ComparisonNode(ErasedNode target, ErasedNode threshold)
  : target_{std::move(target)}
  , threshold_{std::move(threshold)}
  {
  }

  auto operator==(const ComparisonNode& other) const noexcept -> bool = default;

  auto target(this const ComparisonNode& self) noexcept -> const ErasedNode&
  {
    return self.target_;
  }

  void target(this ComparisonNode& self, ErasedNode target) noexcept
  {
    self.target_ = std::move(target);
  }

  auto threshold(this const ComparisonNode& self) noexcept -> const ErasedNode&
  {
    return self.threshold_;
  }

  void threshold(this ComparisonNode& self, ErasedNode threshold) noexcept
  {
    self.threshold_ = std::move(threshold);
  }

private:
  ErasedNode target_;
  ErasedNode threshold_;
};

using GreaterEqualNode = ComparisonNode<std::greater_equal<>>;

using GreaterThanNode = ComparisonNode<std::greater<>>;

using LessThanNode = ComparisonNode<std::less<>>;

using LessEqualNode = ComparisonNode<std::less_equal<>>;

using EqualNode = ComparisonNode<std::equal_to<>>;

using NotEqualNode = ComparisonNode<std::not_equal_to<>>;

template<typename TComparator>
auto pludux_tag_invoke(NodeToErasedMethod,
                       const ComparisonNode<TComparator>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  auto target = node_to_erased_method(node.target(), context);
  auto threshold = node_to_erased_method(node.threshold(), context);
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   ComparisonMethod<TComparator,
                    ErasedSeriesMethod<ErasedSeriesMethodContext>,
                    ErasedSeriesMethod<ErasedSeriesMethodContext>>{
    std::move(target), std::move(threshold)}};
}

} // namespace pludux
