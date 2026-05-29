module;

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.tr_method;

export namespace pludux {

class TrMethod {
public:
  TrMethod() = default;

  auto operator==(const TrMethod& other) const noexcept -> bool = default;
};

} // namespace pludux
