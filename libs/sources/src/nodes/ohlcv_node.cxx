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

export auto pludux_tag_invoke(NodeToErasedMethod,
                              const OpenNode&,
                              NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{OpenMethod{}};
}

export auto pludux_tag_invoke(NodeToErasedMethod,
                              const HighNode&,
                              NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{HighMethod{}};
}

export auto pludux_tag_invoke(NodeToErasedMethod,
                              const LowNode&,
                              NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{LowMethod{}};
}

export auto pludux_tag_invoke(NodeToErasedMethod,
                              const CloseNode&,
                              NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{CloseMethod{}};
}

export auto pludux_tag_invoke(NodeToErasedMethod,
                              const VolumeNode&,
                              NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<ErasedSeriesMethodContext>
{
  return ErasedSeriesMethod<ErasedSeriesMethodContext>{VolumeMethod{}};
}

} // namespace pludux
