module;

#include <utility>

export module pludux:nodes.percentage_node;

import :methods.percentage_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;

export namespace pludux {

class PercentageNode {
public:
  PercentageNode()
  : PercentageNode{CloseNode{}, 100.0}
  {
  }

  explicit PercentageNode(double percent)
  : PercentageNode{CloseNode{}, percent}
  {
  }

  PercentageNode(ErasedNode base, double percent)
  : base_{std::move(base)}
  , percent_{percent}
  {
  }

  auto operator==(const PercentageNode& other) const noexcept -> bool = default;

  auto base(this const PercentageNode& self) noexcept -> const ErasedNode&
  {
    return self.base_;
  }

  void base(this PercentageNode& self, ErasedNode new_base) noexcept
  {
    self.base_ = std::move(new_base);
  }

  auto percent(this const PercentageNode& self) noexcept -> double
  {
    return self.percent_;
  }

  void percent(this PercentageNode& self, double new_percent) noexcept
  {
    self.percent_ = new_percent;
  }

private:
  ErasedNode base_;
  double percent_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const PercentageNode& node,
                       NodeToErasedMethodContext& context) -> AnySeriesMethod
{
  return AnySeriesMethod{PercentageMethod{
   node_to_erased_method(node.base(), context), node.percent()}};
}

} // namespace pludux
