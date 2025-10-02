module;

#include <functional>

export module pludux:series.ohlcv_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

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

export struct OpenMethod : OhlcvMethod<OpenMethod> {
public:
  using ResultType = double;

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return asset_snapshot.open();
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }
};

export struct HighMethod : OhlcvMethod<HighMethod> {
public:
  using ResultType = double;

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return asset_snapshot.high();
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }
};

export struct LowMethod : OhlcvMethod<LowMethod> {
public:
  using ResultType = double;

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return asset_snapshot.low();
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }
};

export struct CloseMethod : OhlcvMethod<CloseMethod> {
public:
  using ResultType = double;

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return asset_snapshot.close();
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }
};

export struct VolumeMethod : OhlcvMethod<VolumeMethod> {
public:
  using ResultType = double;

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return asset_snapshot.volume();
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept
   -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }
};

} // namespace pludux
