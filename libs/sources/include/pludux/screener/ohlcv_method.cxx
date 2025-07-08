module;

#include <cmath>
#include <functional>
#include <string>
#include <vector>

export module pludux.screener.ohlcv_method;

import pludux.asset_snapshot;
import pludux.series;

namespace pludux::screener {

namespace details {

template<typename TSelf, auto asset_snapshot_get_values>
class OhlcvMethod {
public:
  explicit OhlcvMethod(std::size_t offset = 0)
  : offset_{offset}
  {
  }

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>
  {
    return SubSeries<PolySeries<double>>{
     std::invoke(asset_snapshot_get_values, asset_data),
     static_cast<std::ptrdiff_t>(offset_)};
  }

  auto operator==(const OhlcvMethod& other) const noexcept -> bool = default;

  auto offset() const noexcept -> std::size_t
  {
    return offset_;
  }

private:
  std::size_t offset_{};
};

} // namespace details

export class OpenMethod
: public details::OhlcvMethod<OpenMethod, &AssetSnapshot::get_open_values> {
public:
  using details::OhlcvMethod<OpenMethod,
                             &AssetSnapshot::get_open_values>::OhlcvMethod;
};

export class HighMethod
: public details::OhlcvMethod<HighMethod, &AssetSnapshot::get_high_values> {
public:
  using details::OhlcvMethod<HighMethod,
                             &AssetSnapshot::get_high_values>::OhlcvMethod;
};

export class LowMethod
: public details::OhlcvMethod<LowMethod, &AssetSnapshot::get_low_values> {
public:
  using details::OhlcvMethod<LowMethod,
                             &AssetSnapshot::get_low_values>::OhlcvMethod;
};

export class CloseMethod
: public details::OhlcvMethod<CloseMethod, &AssetSnapshot::get_close_values> {
public:
  using details::OhlcvMethod<CloseMethod,
                             &AssetSnapshot::get_close_values>::OhlcvMethod;
};

export class VolumeMethod
: public details::OhlcvMethod<VolumeMethod, &AssetSnapshot::get_volume_values> {
public:
  using details::OhlcvMethod<VolumeMethod,
                             &AssetSnapshot::get_volume_values>::OhlcvMethod;
};

} // namespace pludux::screener
