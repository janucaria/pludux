module;

#include <string>
#include <utility>

export module pludux:nodes.series_value_node;

import :methods.series_value_method;
import :node_to_erased_method;

export namespace pludux {

class SeriesValueNode {
public:
  SeriesValueNode(std::string name)
  : name_{std::move(name)}
  {
  }

  auto operator==(const SeriesValueNode& other) const noexcept
   -> bool = default;

  auto name(this const SeriesValueNode& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this SeriesValueNode& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

private:
  std::string name_{};
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const SeriesValueNode& node,
                       NodeToErasedMethodContext&) -> AnySeriesMethod
{
  return AnySeriesMethod{SeriesValueMethod{node.name()}};
}

} // namespace pludux
