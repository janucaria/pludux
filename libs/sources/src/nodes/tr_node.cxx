module;

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <variant>

export module pludux:nodes.tr_node;

import :methods.tr_method;
import :node_to_erased_method;

export namespace pludux {

template<typename TContext = std::monostate>
class TrNode {
public:
  TrNode() = default;

  auto operator==(const TrNode& other) const noexcept -> bool = default;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                         const TrNode<TContext>&,
                       NodeToErasedMethodContext&)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{TrMethod{}};
}

} // namespace pludux
