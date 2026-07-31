module;

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

export module pludux:nodes.numeric_input_node;

import :methods.value_method;
import :node_to_erased_method;

export namespace pludux {

class NumericInputNode {
public:
  enum class ValueRepresentation { Decimal, SignedInteger, UnsignedInteger };

  NumericInputNode()
  : NumericInputNode{""}
  {
  }

  NumericInputNode(
   std::string label,
   ValueRepresentation representation = ValueRepresentation::Decimal,
   double value = 0.0)
  : label_{std::move(label)}
  , representation_{representation}
  , value_{value}
  {
  }

  auto operator==(this const NumericInputNode& self,
                  const NumericInputNode& other) noexcept -> bool
  {
    return self.label_ == other.label_ &&
           self.representation_ == other.representation_ &&
           self.value_ == other.value_;
  }

  auto operator!=(this const NumericInputNode& self,
                  const NumericInputNode& other) noexcept -> bool
  {
    return !(self == other);
  }

  auto label(this const NumericInputNode& self) noexcept -> const std::string&
  {
    return self.label_;
  }

  void label(this NumericInputNode& self, std::string new_label) noexcept
  {
    self.label_ = std::move(new_label);
  }

  auto representation(this const NumericInputNode& self) noexcept
   -> ValueRepresentation
  {
    return self.representation_;
  }

  void representation(this NumericInputNode& self,
                      ValueRepresentation new_representation) noexcept
  {
    self.representation_ = new_representation;
  }

  auto value(this const NumericInputNode& self) noexcept -> double
  {
    return self.value_;
  }

  void value(this NumericInputNode& self, double new_value) noexcept
  {
    self.value_ = new_value;
  }

private:
  std::string label_;
  ValueRepresentation representation_;
  double value_;
};

auto resolve_numeric_input_value(
 double value, NumericInputNode::ValueRepresentation representation) noexcept
 -> double
{
  switch(representation) {
  case NumericInputNode::ValueRepresentation::Decimal:
    return value;
  case NumericInputNode::ValueRepresentation::SignedInteger:
    return std::trunc(value);
  case NumericInputNode::ValueRepresentation::UnsignedInteger:
    return std::max(0.0, std::trunc(value));
  default:
    return value;
  }
}

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const NumericInputNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto value = context.consume(node.value());
  return ErasedSeriesMethod<TContext>{
   ValueMethod{resolve_numeric_input_value(value, node.representation())}};
}

} // namespace pludux
