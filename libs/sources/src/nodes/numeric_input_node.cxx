module;

#include <string>
#include <utility>

export module pludux:nodes.numeric_input_node;


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

  auto operator==(const NumericInputNode& other) const noexcept
   -> bool = default;

  auto label(this const NumericInputNode& self) noexcept
   -> const std::string&
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

} // namespace pludux
