module;

#include <string>
#include <utility>

export module pludux:nodes.data_node;


export namespace pludux {

class DataNode {
public:
  DataNode() = default;

  explicit DataNode(std::string field)
  : field_{std::move(field)}
  {
  }

  auto operator==(const DataNode& other) const noexcept -> bool = default;


  auto field(this const DataNode& self) -> const std::string&
  {
    return self.field_;
  }

  void field(this DataNode& self, std::string new_field)
  {
    self.field_ = std::move(new_field);
  }

private:
  std::string field_{};
};

} // namespace pludux