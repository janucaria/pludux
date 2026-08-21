module;

#include <limits>
#include <utility>

export module pludux:nodes.ohlcv_node;

import :methods.ohlcv_method;
import :node_to_erased_method;

namespace pludux {

template<typename>
class OhlcvNode {
public:
  template<typename TSelf>
  auto operator==(this TSelf self, TSelf other) noexcept -> bool
  {
    return true;
  }

  template<typename TSelf>
  auto operator!=(this TSelf self, const TSelf& other) noexcept -> bool
  {
    return !(self == other);
  }
};

export struct OpenNode : OhlcvNode<OpenNode> {};

export struct HighNode : OhlcvNode<HighNode> {};

export struct LowNode : OhlcvNode<LowNode> {};

export struct CloseNode : OhlcvNode<CloseNode> {};

export struct VolumeNode : OhlcvNode<VolumeNode> {};

export template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const OpenNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{OpenMethod{}};
}

export template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const HighNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{HighMethod{}};
}

export template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const LowNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{LowMethod{}};
}

export template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const CloseNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{CloseMethod{}};
}

export template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const VolumeNode&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{VolumeMethod{}};
}

} // namespace pludux
