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

auto pludux_tag_invoke(NodeToErasedMethod,
                       const EquityNode& node,
                       NodeToErasedMethodContext& context) noexcept
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  static_cast<void>(node);
  static_cast<void>(context);
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{EquityMethod{}};
}

auto pludux_tag_invoke(NodeToErasedMethod,
                       const EquityPercentNode& node,
                       NodeToErasedMethodContext& context) noexcept
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  static_cast<void>(node);
  static_cast<void>(context);
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{EquityPercentMethod{}};
}

} // namespace pludux::backtest
