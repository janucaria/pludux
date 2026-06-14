module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>
#include <utility>

export module pludux:constrained_numeric_input;

export namespace pludux {

class ConstrainedNumericInput {
public:
  enum class ValueRepresentation { Decimal, SignedInteger, UnsignedInteger };

  ConstrainedNumericInput()
  : ConstrainedNumericInput{""}
  {
  }

  ConstrainedNumericInput(
   std::string label,
   ValueRepresentation representation = ValueRepresentation::Decimal,
   double value = 0.0)
  : label_{std::move(label)}
  , representation_{representation}
  , value_{value}
  {
  }

  auto operator==(const ConstrainedNumericInput& other) const noexcept -> bool
  {
    return label_ == other.label_ && representation_ == other.representation_ &&
           value_ == other.value_;
  }

  auto label(this const ConstrainedNumericInput& self) noexcept
   -> const std::string&
  {
    return self.label_;
  }

  void label(this ConstrainedNumericInput& self, std::string new_label) noexcept
  {
    self.label_ = std::move(new_label);
  }

  auto representation(this const ConstrainedNumericInput& self) noexcept
   -> ValueRepresentation
  {
    return self.representation_;
  }

  void representation(this ConstrainedNumericInput& self,
                      ValueRepresentation new_representation) noexcept
  {
    self.representation_ = new_representation;
  }

  auto value(this const ConstrainedNumericInput& self) noexcept -> double
  {
    return self.value_;
  }

  void value(this ConstrainedNumericInput& self, double new_value) noexcept
  {
    self.value_ = new_value;
  }

  auto resolved_value(this const ConstrainedNumericInput& self) noexcept
   -> double
  {
    switch(self.representation_) {
    case ValueRepresentation::Decimal:
      return self.value_;
    case ValueRepresentation::SignedInteger:
      return std::trunc(self.value_);
    case ValueRepresentation::UnsignedInteger:
      return std::max(0.0, std::trunc(self.value_));
    default:
      return self.value_;
    }
  }

private:
  std::string label_;
  ValueRepresentation representation_;
  double value_;
};

} // namespace pludux