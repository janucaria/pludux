module;

#include <string>
#include <utility>

export module pludux:nodes.series_result_node;

import :methods.series_result_method;
import :node_to_erased_method;

export namespace pludux {

class SeriesResultNode {
public:
  SeriesResultNode(std::string name)
  : name_{std::move(name)}
  {
  }

  auto operator==(const SeriesResultNode& other) const noexcept
   -> bool = default;

  auto name(this const SeriesResultNode& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this SeriesResultNode& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

private:
  std::string name_{};
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const SeriesResultNode& node,
                       NodeToErasedMethodContext&) -> AnySeriesMethod
{
  return AnySeriesMethod{SeriesResultMethod{node.name()}};
}

} // namespace pludux
