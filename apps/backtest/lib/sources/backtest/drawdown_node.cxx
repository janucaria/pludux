export module pludux.backtest:drawdown_node;

import pludux;

import :drawdown_method;

export namespace pludux::backtest {

class DrawdownNode {
public:
  DrawdownNode() = default;

  auto operator==(const DrawdownNode& other) const noexcept -> bool = default;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const DrawdownNode& node,
                       NodeToErasedMethodContext& context) noexcept
 -> AnySeriesMethod
{
  static_cast<void>(node);
  static_cast<void>(context);
  return AnySeriesMethod{DrawdownMethod{}};
}

} // namespace pludux::backtest
