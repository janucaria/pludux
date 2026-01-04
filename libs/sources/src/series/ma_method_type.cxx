module;

#include <cstddef>
#include <limits>

export module pludux:series.ma_method_type;

import :asset_snapshot;
import :method_contextable;

export import :series.sma_method;
export import :series.ema_method;
export import :series.wma_method;
export import :series.rma_method;
export import :series.hma_method;

export namespace pludux {

enum class MaMethodType { Sma, Ema, Wma, Rma, Hma };

template<typename MaSource>
  requires requires { typename MaSource::ResultType; }
auto call_ma_method(MaMethodType ma_type,
                    MaSource ma_source,
                    std::size_t period,
                    AssetSnapshot asset_snapshot,
                    MethodContextable auto context) ->
 typename MaSource::ResultType
{
  switch(ma_type) {
  case MaMethodType::Sma: {
    const auto ma = SmaMethod{ma_source, period};
    return ma(asset_snapshot, context);
  }
  case MaMethodType::Ema: {
    const auto ma = EmaMethod{ma_source, period};
    return ma(asset_snapshot, context);
  }
  case MaMethodType::Wma: {
    const auto ma = WmaMethod{ma_source, period};
    return ma(asset_snapshot, context);
  }
  case MaMethodType::Rma: {
    const auto ma = RmaMethod{ma_source, period};
    return ma(asset_snapshot, context);
  }
  case MaMethodType::Hma: {
    const auto ma = HmaMethod{ma_source, period};
    return ma(asset_snapshot, context);
  }
  default:
    return std::numeric_limits<typename MaSource::ResultType>::quiet_NaN();
  }
}

} // namespace pludux
