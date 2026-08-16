module;

export module pludux.backtest:model_performance_node;

import :model_performance;
import :model_performance_method;

export namespace pludux::backtest {

class ModelPerformanceNode {
public:
  explicit ModelPerformanceNode(ModelPerformanceMetric metric) noexcept
  : metric_{metric}
  {
  }

  auto operator==(const ModelPerformanceNode&) const noexcept
   -> bool = default;

  auto metric(this const ModelPerformanceNode& self) noexcept
   -> ModelPerformanceMetric
  {
    return self.metric_;
  }

private:
  ModelPerformanceMetric metric_{ModelPerformanceMetric::LifetimeCount};
};

auto pludux_tag_invoke(NodeToErasedMethod<EntryFilterMethodContext>,
                        const ModelPerformanceNode& node,
                       NodeToErasedMethodContext&) noexcept
 -> ErasedSeriesMethod<EntryFilterMethodContext>
{
  return ErasedSeriesMethod<EntryFilterMethodContext>{
    ModelPerformanceMethod{node.metric()}};
}

} // namespace pludux::backtest
