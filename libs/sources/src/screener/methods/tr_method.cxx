module;

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:screener.tr_method;

import :asset_snapshot;
import :screener.method_call_context;
import :screener.method_output;

import :screener.ohlcv_method;

export namespace pludux::screener {

class TrMethod {
public:
  using ResultType = std::common_type_t<typename HighMethod::ResultType,
                                        typename LowMethod::ResultType,
                                        typename CloseMethod::ResultType>;

  TrMethod() = default;

  auto operator==(const TrMethod& other) const noexcept -> bool = default;

  auto operator()(this auto self,
                  AssetSnapshot asset_snapshot,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    const auto high = HighMethod{}(asset_snapshot, context);
    const auto low = LowMethod{}(asset_snapshot, context);

    const auto close_last_index = asset_snapshot.size() - 1;
    const auto prev_close = close_last_index == 0
                             ? CloseMethod{}(asset_snapshot, context)
                             : CloseMethod{}(asset_snapshot[1], context);

    const auto hl = std::abs(high - low);
    const auto hc = std::abs(high - prev_close);
    const auto lc = std::abs(low - prev_close);

    return std::max(std::max(hl, hc), lc);
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
