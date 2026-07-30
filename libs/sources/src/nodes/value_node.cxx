export module pludux:nodes.value_node;

import :methods.value_method;
import :node_to_erased_method;

export namespace pludux {

class ValueNode {
public:
  explicit ValueNode(double value)
  : value_{value}
  {
  }

  auto operator==(const ValueNode& other) const noexcept -> bool = default;

  auto value(this ValueNode self) noexcept -> double
  {
    return self.value_;
  }

  void value(this ValueNode& self, double new_value) noexcept
  {
    self.value_ = new_value;
  }

private:
  double value_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const ValueNode& node,
                       NodeToErasedMethodContext&) -> AnySeriesMethod
{
  return AnySeriesMethod{ValueMethod{node.value()}};
}

} // namespace pludux
