module;

export module pludux:series.value_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

export namespace pludux {

class ValueMethod {
public:
  explicit ValueMethod(double value)
  : value_{value}
  {
  }

  auto operator==(const ValueMethod& other) const noexcept -> bool = default;

  auto value(this ValueMethod self) noexcept -> double
  {
    return self.value_;
  }

  void value(this ValueMethod& self, double new_value) noexcept
  {
    self.value_ = new_value;
  }

private:
  double value_;
};

} // namespace pludux
