module;

#include <utility>
#include <variant>

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

template<typename TContext = std::monostate>
class SelectOutputNode final {
public:
  SelectOutputNode(ErasedNode<TContext> source, NodeOutput output)
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

  auto source(this const SelectOutputNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.source_;
  }

  void source(this SelectOutputNode& self, ErasedNode<TContext> source) noexcept
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
  ErasedNode<TContext> source_;
  NodeOutput output_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const SelectOutputNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{
   SelectOutputMethod{node_to_erased_method<TContext>(node.source(), context),
                      static_cast<MethodOutput>(node.output())}};
}

} // namespace pludux
