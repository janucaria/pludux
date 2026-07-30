module;

#include <string>
#include <utility>

export module pludux:nodes.data_node;

import :methods.data_method;
import :node_to_erased_method;

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

auto pludux_tag_invoke(NodeToErasedMethod,
                       const DataNode& node,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{
   DataMethod{node.field()}};
}

} // namespace pludux
