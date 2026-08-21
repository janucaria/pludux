export module pludux.backtest:pyramiding_layer_node;

import pludux;

import :pyramiding_layer_method;

export namespace pludux::backtest {

class PyramidingLayerNode {
public:
  auto operator==(const PyramidingLayerNode&) const noexcept -> bool = default;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const PyramidingLayerNode&,
                        NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{PyramidingLayerMethod{}};
}

} // namespace pludux::backtest
