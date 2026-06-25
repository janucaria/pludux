module;

#include <string>
#include <utility>

export module pludux:nodes.input_node;


export namespace pludux {

class InputNode {
public:
  InputNode()
  : InputNode{""}
  {
  }

  InputNode(std::string name)
  : name_{std::move(name)}
  {
  }

  auto operator==(const InputNode& other) const noexcept -> bool = default;


  auto name(this const InputNode& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this InputNode& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

private:
  std::string name_;
};

} // namespace pludux