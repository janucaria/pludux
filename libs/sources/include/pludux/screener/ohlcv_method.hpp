#ifndef PLUDUX_PLUDUX_SCREENER_OHLCV_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_OHLCV_METHOD_HPP

#include <cmath>
#include <functional>
#include <string>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

namespace details {

template<typename TSelf, auto asset_snapshot_get_values>
class OhlcvMethod {
public:
  explicit OhlcvMethod(std::size_t offset = 0)
  : offset_{offset}
  {
  }

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    return LookbackSeries<PolySeries<double>>{
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
