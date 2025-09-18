module;

#include <cmath>
#include <functional>
#include <string>
#include <vector>

export module pludux.screener.ohlcv_method;

import pludux.asset_snapshot;

namespace pludux::screener {

template<typename TSelf, auto asset_snapshot_get_values>
class OhlcvMethod {
public:
  auto operator==(const OhlcvMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    return std::invoke(asset_snapshot_get_values, asset_data);
  }
};

export class OpenMethod
: public OhlcvMethod<OpenMethod, &AssetSnapshot::get_open_values> {
public:
  using OhlcvMethod<OpenMethod, &AssetSnapshot::get_open_values>::OhlcvMethod;
};

export class HighMethod
: public OhlcvMethod<HighMethod, &AssetSnapshot::get_high_values> {
public:
  using OhlcvMethod<HighMethod, &AssetSnapshot::get_high_values>::OhlcvMethod;
};

export class LowMethod
: public OhlcvMethod<LowMethod, &AssetSnapshot::get_low_values> {
public:
  using OhlcvMethod<LowMethod, &AssetSnapshot::get_low_values>::OhlcvMethod;
};

export class CloseMethod
: public OhlcvMethod<CloseMethod, &AssetSnapshot::get_close_values> {
public:
  using OhlcvMethod<CloseMethod, &AssetSnapshot::get_close_values>::OhlcvMethod;
};

export class VolumeMethod
: public OhlcvMethod<VolumeMethod, &AssetSnapshot::get_volume_values> {
public:
  using OhlcvMethod<VolumeMethod,
                    &AssetSnapshot::get_volume_values>::OhlcvMethod;
};

} // namespace pludux::screener

