module;

#include <functional>

export module pludux:screener.ohlcv_method;

import :asset_snapshot;

namespace pludux::screener {

template<typename TSelf, auto TFunc>
class OhlcvMethod {
public:
  auto operator==(const OhlcvMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto&, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    return std::invoke(TFunc, asset_data);
  }
};

export struct OpenMethod
: OhlcvMethod<OpenMethod, &AssetSnapshot::get_open_values> {};

export struct HighMethod
: OhlcvMethod<HighMethod, &AssetSnapshot::get_high_values> {};

export struct LowMethod
: OhlcvMethod<LowMethod, &AssetSnapshot::get_low_values> {};

export struct CloseMethod
: OhlcvMethod<CloseMethod, &AssetSnapshot::get_close_values> {};

export struct VolumeMethod
: OhlcvMethod<VolumeMethod, &AssetSnapshot::get_volume_values> {};

} // namespace pludux::screener
