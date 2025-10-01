module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:screener.macd_method;

import :asset_snapshot;
import :screener.method_call_context;
import :screener.method_output;

import :screener.ema_method;
import :screener.operators_method;
import :screener.ohlcv_method;

export namespace pludux::screener {

template<typename TSourceMethod = CloseMethod>
class MacdMethod {
public:
  using ResultType = typename TSourceMethod::ResultType;

  MacdMethod()
  : MacdMethod{12, 26, 9}
  {
  }

  MacdMethod(std::size_t short_period,
             std::size_t long_period,
             std::size_t signal_period)
  : MacdMethod{TSourceMethod{}, short_period, long_period, signal_period}
  {
  }

  MacdMethod(TSourceMethod source,
             std::size_t short_period,
             std::size_t long_period,
             std::size_t signal_period)
  : source_{std::move(source)}
  , short_period_{short_period}
  , long_period_{long_period}
  , signal_period_{signal_period}
  {
  }

  auto operator==(const MacdMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    return self(asset_snapshot, MethodOutput::MacdLine, context);
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodOutput output,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    const auto short_ema = EmaMethod{self.source_, self.short_period_};
    const auto long_ema = EmaMethod{self.source_, self.long_period_};
    const auto macd_method = SubtractMethod{short_ema, long_ema};
    const auto signal_ema = EmaMethod{macd_method, self.signal_period_};

    const auto macd = macd_method(asset_snapshot, context);
    const auto signal = signal_ema(asset_snapshot, context);
    const auto histogram = macd - signal;

    switch(output) {
    case MethodOutput::MacdLine:
      return macd;
    case MethodOutput::SignalLine:
      return signal;
    case MethodOutput::Histogram:
      return histogram;
    default:
      return std::numeric_limits<ResultType>::quiet_NaN();
    }
  }

  auto source(this const auto& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this auto& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto short_period(this const auto& self) noexcept -> std::size_t
  {
    return self.short_period_;
  }

  void short_period(this auto& self, std::size_t period) noexcept
  {
    self.short_period_ = period;
  }

  auto fast_period(this const auto& self) noexcept -> std::size_t
  {
    return self.short_period_;
  }

  void fast_period(this auto& self, std::size_t period) noexcept
  {
    self.short_period_ = period;
  }

  auto long_period(this const auto& self) noexcept -> std::size_t
  {
    return self.long_period_;
  }

  void long_period(this auto& self, std::size_t period) noexcept
  {
    self.long_period_ = period;
  }

  auto slow_period(this const auto& self) noexcept -> std::size_t
  {
    return self.long_period_;
  }

  void slow_period(this auto& self, std::size_t period) noexcept
  {
    self.long_period_ = period;
  }

  auto signal_period(this const auto& self) noexcept -> std::size_t
  {
    return self.signal_period_;
  }

  void signal_period(this auto& self, std::size_t period) noexcept
  {
    self.signal_period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t short_period_;
  std::size_t long_period_;
  std::size_t signal_period_;
};

} // namespace pludux::screener
