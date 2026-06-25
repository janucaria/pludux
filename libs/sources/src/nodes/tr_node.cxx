module;

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:nodes.tr_node;


export namespace pludux {

class TrNode {
public:
  TrNode() = default;

  auto operator==(const TrNode& other) const noexcept -> bool = default;

};

} // namespace pludux