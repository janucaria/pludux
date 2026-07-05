module;

#include <utility>

export module pludux:nodes.crossunder_node;

import :methods.crossunder_method;
import :node_to_erased_method;
import :nodes.erased_node;

export namespace pludux {

class CrossunderNode {
public:
  CrossunderNode(ErasedNode source, ErasedNode reference)
  : source_{std::move(source)}
  , reference_{std::move(reference)}
  {
  }

  auto operator==(const CrossunderNode& other) const noexcept -> bool = default;

  auto source(this const CrossunderNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this CrossunderNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto reference(this const CrossunderNode& self) noexcept -> const ErasedNode&
  {
    return self.reference_;
  }

  void reference(this CrossunderNode& self, ErasedNode reference) noexcept
  {
    self.reference_ = std::move(reference);
  }

private:
  ErasedNode source_;
  ErasedNode reference_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const CrossunderNode& node,
                       NodeToErasedMethodContext& context) -> AnySeriesMethod
{
  auto source = node_to_erased_method(node.source(), context);
  auto reference = node_to_erased_method(node.reference(), context);
  return AnySeriesMethod{CrossunderMethod<AnySeriesMethod, AnySeriesMethod>{
   std::move(source), std::move(reference)}};
}

} // namespace pludux
