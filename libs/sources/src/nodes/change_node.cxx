module;

#include <utility>

export module pludux:nodes.change_node;

import :methods.change_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;

export namespace pludux {

class ChangeNode {
public:
  ChangeNode()
  : ChangeNode{CloseNode{}}
  {
  }

  explicit ChangeNode(ErasedNode source)
  : source_{std::move(source)}
  {
  }

  auto operator==(const ChangeNode& other) const noexcept -> bool = default;

  auto source(this const ChangeNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this ChangeNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

private:
  ErasedNode source_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const ChangeNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   ChangeMethod{node_to_erased_method(node.source(), context)}};
}

} // namespace pludux
