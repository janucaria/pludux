export module pludux.backtest:drawdown_node;

import pludux;

import :drawdown_method;

export namespace pludux::backtest {

class DrawdownNode {
public:
  DrawdownNode() = default;

  auto operator==(const DrawdownNode& other) const noexcept -> bool = default;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const DrawdownNode& node,
                       NodeToErasedMethodContext& context) noexcept
 -> ErasedSeriesMethod<TContext>
{
  static_cast<void>(node);
  static_cast<void>(context);
  return ErasedSeriesMethod<TContext>{DrawdownMethod{}};
}

} // namespace pludux::backtest
