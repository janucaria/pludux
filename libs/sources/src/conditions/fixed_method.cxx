export module pludux:conditions.fixed_method;

import :asset_snapshot;
import :method_contextable;

namespace pludux {

template<typename, bool boolean_value>
struct FixedMethod {
  auto operator==(const FixedMethod&) const noexcept -> bool = default;

  auto operator()(this const FixedMethod,
                  AssetSnapshot,
                  MethodContextable auto context) -> bool
  {
    return boolean_value;
  }
};

export struct AlwaysMethod : FixedMethod<AlwaysMethod, true> {};

export struct NeverMethod : FixedMethod<NeverMethod, false> {};

} // namespace pludux
