module;

#include <functional>

export module pludux:screener.ohlcv_method;

import :asset_snapshot;
import :screener.method_call_context;
import :screener.method_output;

namespace pludux::screener {

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
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    return asset_snapshot.open();
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodOutput output,
                  MethodCallContext<ResultType> auto context) noexcept
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
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    return asset_snapshot.high();
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodOutput output,
                  MethodCallContext<ResultType> auto context) noexcept
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
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    return asset_snapshot.low();
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodOutput output,
                  MethodCallContext<ResultType> auto context) noexcept
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
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    return asset_snapshot.close();
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodOutput output,
                  MethodCallContext<ResultType> auto context) noexcept
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
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    return asset_snapshot.volume();
  }

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodOutput output,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }
};

} // namespace pludux::screener
