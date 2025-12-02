export module pludux:conditions.boolean_method;

import :asset_snapshot;
import :method_contextable;

namespace pludux {

template<typename, bool boolean_value>
struct BooleanMethod {
  auto operator==(const BooleanMethod&) const noexcept -> bool = default;

  auto operator()(this const BooleanMethod,
                  AssetSnapshot,
                  MethodContextable auto context) -> bool
  {
    return boolean_value;
  }
};

export struct TrueMethod : BooleanMethod<TrueMethod, true> {};

export struct FalseMethod : BooleanMethod<FalseMethod, false> {};

} // namespace pludux
