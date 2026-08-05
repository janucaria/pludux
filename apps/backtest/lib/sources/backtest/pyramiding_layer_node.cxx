export module pludux.backtest:pyramiding_layer_node;

import pludux;

import :pyramiding_layer_method;

export namespace pludux::backtest {

class PyramidingLayerNode {
public:
  auto operator==(const PyramidingLayerNode&) const noexcept -> bool = default;
};

auto pludux_tag_invoke(NodeToErasedMethod<ErasedSeriesMethodContext>,
                       const PyramidingLayerNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{PyramidingLayerMethod{}};
}

} // namespace pludux::backtest
