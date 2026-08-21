module;

#include <utility>
#include <variant>

export module pludux:nodes.crossover_node;

import :methods.crossover_method;
import :node_to_erased_method;
import :nodes.erased_node;

export namespace pludux {

template<typename TContext = std::monostate>
class CrossoverNode {
public:
   CrossoverNode(ErasedNode<TContext> source, ErasedNode<TContext> reference)
  : source_{std::move(source)}
  , reference_{std::move(reference)}
  {
  }

   auto operator==(const CrossoverNode& other) const noexcept -> bool = default;

   auto source(this const CrossoverNode& self) noexcept
    -> const ErasedNode<TContext>&
  {
    return self.source_;
  }

   void source(this CrossoverNode& self, ErasedNode<TContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

   auto reference(this const CrossoverNode& self) noexcept
    -> const ErasedNode<TContext>&
  {
    return self.reference_;
  }

   void reference(this CrossoverNode& self, ErasedNode<TContext> reference) noexcept
  {
    self.reference_ = std::move(reference);
  }

private:
   ErasedNode<TContext> source_;
   ErasedNode<TContext> reference_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const CrossoverNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  auto source = node_to_erased_method<TContext>(node.source(), context);
  auto reference = node_to_erased_method<TContext>(node.reference(), context);
  return ErasedSeriesMethod<TContext>{
   CrossoverMethod<ErasedSeriesMethod<TContext>, ErasedSeriesMethod<TContext>>{
    std::move(source), std::move(reference)}};
}

} // namespace pludux
