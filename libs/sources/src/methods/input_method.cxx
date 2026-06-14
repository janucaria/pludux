module;

#include <string>
#include <utility>

export module pludux:methods.input_method;

export namespace pludux {

class InputMethod {
public:
  InputMethod()
  : InputMethod{""}
  {
  }

  InputMethod(std::string name)
  : name_{std::move(name)}
  {
  }

  auto operator==(const InputMethod& other) const noexcept -> bool = default;

  auto name(this const InputMethod& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this InputMethod& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

private:
  std::string name_;
};

} // namespace pludux