module;

#include <utility>

export module pludux:nodes.select_output_node;

import :methods.select_output_method;
import :node_to_erased_method;
import :nodes.erased_node;

export namespace pludux {

enum class NodeOutput {
  UpperBand,
  MiddleBand,
  LowerBand,
  MacdLine,
  SignalLine,
  Histogram,
  KPercent,
  DPercent
};

class SelectOutputNode {
public:
  SelectOutputNode(ErasedNode source, NodeOutput output)
  : source_{std::move(source)}
  , output_{output}
  {
  }

  SelectOutputNode(const SelectOutputNode& other, NodeOutput output)
  : SelectOutputNode{other.source(), output}
  {
  }

  auto operator==(const SelectOutputNode& other) const noexcept
   -> bool = default;

  auto source(this const SelectOutputNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this SelectOutputNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto output(this const SelectOutputNode& self) noexcept -> NodeOutput
  {
    return self.output_;
  }

  void output(this SelectOutputNode& self, NodeOutput output_name) noexcept
  {
    self.output_ = std::move(output_name);
  }

private:
  ErasedNode source_;
  NodeOutput output_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const SelectOutputNode& node,
                       NodeToErasedMethodContext& context) -> AnySeriesMethod
{
  return AnySeriesMethod{
   SelectOutputMethod{node_to_erased_method(node.source(), context),
                      static_cast<MethodOutput>(node.output())}};
}

} // namespace pludux
