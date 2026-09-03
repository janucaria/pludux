module;

#include <functional>
#include <utility>

export module pludux:nodes.comparison_node;

import :methods.comparison_method;
import :node_to_erased_method;
import :nodes.erased_node;

export namespace pludux {

template<typename TComparator, typename TContext>
class ComparisonNode {
public:
  ComparisonNode(ErasedNode<TContext> target, ErasedNode<TContext> threshold)
  : target_{std::move(target)}
  , threshold_{std::move(threshold)}
  {
  }

  auto operator==(const ComparisonNode& other) const noexcept -> bool = default;

  auto target(this const ComparisonNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.target_;
  }

  void target(this ComparisonNode& self, ErasedNode<TContext> target) noexcept
  {
    self.target_ = std::move(target);
  }

  auto threshold(this const ComparisonNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.threshold_;
  }

  void threshold(this ComparisonNode& self,
                 ErasedNode<TContext> threshold) noexcept
  {
    self.threshold_ = std::move(threshold);
  }

private:
  ErasedNode<TContext> target_;
  ErasedNode<TContext> threshold_;
};

template<typename TContext>
using GreaterEqualNode = ComparisonNode<std::greater_equal<>, TContext>;

template<typename TContext>
using GreaterThanNode = ComparisonNode<std::greater<>, TContext>;

template<typename TContext>
using LessThanNode = ComparisonNode<std::less<>, TContext>;

template<typename TContext>
using LessEqualNode = ComparisonNode<std::less_equal<>, TContext>;

template<typename TContext>
using EqualNode = ComparisonNode<std::equal_to<>, TContext>;

template<typename TContext>
using NotEqualNode = ComparisonNode<std::not_equal_to<>, TContext>;

template<typename TComparator, typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const ComparisonNode<TComparator, TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  auto target = node_to_erased_method<TContext>(node.target(), context);
  auto threshold = node_to_erased_method<TContext>(node.threshold(), context);
  return ErasedSeriesMethod<TContext>{
   ComparisonMethod<TComparator,
                    ErasedSeriesMethod<TContext>,
                    ErasedSeriesMethod<TContext>>{std::move(target),
                                                  std::move(threshold)}};
}

} // namespace pludux
