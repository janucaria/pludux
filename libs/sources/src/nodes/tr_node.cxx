module;

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:nodes.tr_node;

import :methods.tr_method;
import :node_to_erased_method;

export namespace pludux {

class TrNode {
public:
  TrNode() = default;

  auto operator==(const TrNode& other) const noexcept -> bool = default;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const TrNode&,
                       NodeToErasedMethodContext&) -> AnySeriesMethod
{
  return AnySeriesMethod{TrMethod{}};
}

} // namespace pludux
