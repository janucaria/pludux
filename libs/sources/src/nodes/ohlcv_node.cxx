module;

#include <limits>
#include <utility>

export module pludux:nodes.ohlcv_node;


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

} // namespace pludux
