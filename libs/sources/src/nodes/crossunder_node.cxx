module;

#include <utility>

export module pludux:nodes.crossunder_node;

import :methods.crossunder_method;
import :node_to_erased_method;
import :nodes.erased_node;

export namespace pludux {

class CrossunderNode {
public:
  CrossunderNode(ErasedNode<ErasedSeriesMethodContext> source,
                 ErasedNode<ErasedSeriesMethodContext> reference)
  : source_{std::move(source)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossunderNode& other) const noexcept -> bool = default;

  auto source(this const CrossunderNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.source_;
  }

  void source(this CrossunderNode& self,
              ErasedNode<ErasedSeriesMethodContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto reference(this const CrossunderNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.reference_;
  }

  void reference(this CrossunderNode& self,
                 ErasedNode<ErasedSeriesMethodContext> reference) noexcept
  {
    self.reference_ = std::move(reference);
  }

private:
  ErasedNode<ErasedSeriesMethodContext> source_;
  ErasedNode<ErasedSeriesMethodContext> reference_;
};

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const CrossunderNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  auto source = node_to_erased_method<TContext>(node.source(), context);
  auto reference = node_to_erased_method<TContext>(node.reference(), context);
  return ErasedSeriesMethod<TContext>{
   CrossunderMethod<ErasedSeriesMethod<TContext>, ErasedSeriesMethod<TContext>>{
    std::move(source), std::move(reference)}};
}

} // namespace pludux
