module;

export module pludux.backtest:strategy_performance_node;

import :strategy_performance;
import :strategy_performance_method;

export namespace pludux::backtest {

class StrategyPerformanceNode {
public:
  explicit StrategyPerformanceNode(StrategyPerformanceMetric metric) noexcept
  : metric_{metric}
  {
  }

  auto operator==(const StrategyPerformanceNode&) const noexcept
   -> bool = default;

  auto metric(this const StrategyPerformanceNode& self) noexcept
   -> StrategyPerformanceMetric
  {
    return self.metric_;
  }

private:
  StrategyPerformanceMetric metric_{StrategyPerformanceMetric::LifetimeCount};
};

auto pludux_tag_invoke(NodeToErasedMethod<EntryFilterMethodContext>,
                       const StrategyPerformanceNode& node,
                       NodeToErasedMethodContext&) noexcept
 -> ErasedSeriesMethod<EntryFilterMethodContext>
{
  return ErasedSeriesMethod<EntryFilterMethodContext>{
   StrategyPerformanceMethod{node.metric()}};
}

} // namespace pludux::backtest
