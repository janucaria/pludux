module;

#include <limits>
#include <utility>

export module pludux:methods.ohlcv_method;

namespace pludux {

template<typename>
class OhlcvMethod {
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

export struct OpenMethod : OhlcvMethod<OpenMethod> {};

export struct HighMethod : OhlcvMethod<HighMethod> {};

export struct LowMethod : OhlcvMethod<LowMethod> {};

export struct CloseMethod : OhlcvMethod<CloseMethod> {};

export struct VolumeMethod : OhlcvMethod<VolumeMethod> {};

} // namespace pludux
