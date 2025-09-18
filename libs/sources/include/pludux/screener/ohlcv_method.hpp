#ifndef PLUDUX_PLUDUX_SCREENER_OHLCV_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_OHLCV_METHOD_HPP

#include <cmath>
#include <functional>
#include <string>
#include <vector>

#include <pludux/series.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

namespace details {

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

} // namespace details

class OpenMethod
: public details::OhlcvMethod<OpenMethod, &AssetSnapshot::get_open_values> {
public:
  using details::OhlcvMethod<OpenMethod,
                             &AssetSnapshot::get_open_values>::OhlcvMethod;
};

class HighMethod
: public details::OhlcvMethod<HighMethod, &AssetSnapshot::get_high_values> {
public:
  using details::OhlcvMethod<HighMethod,
                             &AssetSnapshot::get_high_values>::OhlcvMethod;
};

class LowMethod
: public details::OhlcvMethod<LowMethod, &AssetSnapshot::get_low_values> {
public:
  using details::OhlcvMethod<LowMethod,
                             &AssetSnapshot::get_low_values>::OhlcvMethod;
};

class CloseMethod
: public details::OhlcvMethod<CloseMethod, &AssetSnapshot::get_close_values> {
public:
  using details::OhlcvMethod<CloseMethod,
                             &AssetSnapshot::get_close_values>::OhlcvMethod;
};

class VolumeMethod
: public details::OhlcvMethod<VolumeMethod, &AssetSnapshot::get_volume_values> {
public:
  using details::OhlcvMethod<VolumeMethod,
                             &AssetSnapshot::get_volume_values>::OhlcvMethod;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_FIELD_METHOD_HPP
