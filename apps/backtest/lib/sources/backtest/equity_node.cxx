export module pludux.backtest:equity_node;

import pludux;

import :equity_method;

export namespace pludux::backtest {

class EquityNode {
public:
  EquityNode() = default;

  auto operator==(const EquityNode& other) const noexcept -> bool = default;
};

class EquityPercentNode {
public:
  EquityPercentNode() = default;

  auto operator==(const EquityPercentNode& other) const noexcept
   -> bool = default;
};

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const EquityNode& node,
                       NodeToErasedMethodContext& context) noexcept
 -> ErasedSeriesMethod<TContext>
{
  static_cast<void>(node);
  static_cast<void>(context);
  return ErasedSeriesMethod<TContext>{EquityMethod{}};
}

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const EquityPercentNode& node,
                       NodeToErasedMethodContext& context) noexcept
 -> ErasedSeriesMethod<TContext>
{
  static_cast<void>(node);
  static_cast<void>(context);
  return ErasedSeriesMethod<TContext>{EquityPercentMethod{}};
}

} // namespace pludux::backtest
