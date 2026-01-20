module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.macd_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.cached_results_ema_method;
import :series.operators_method;
import :series.ohlcv_method;

export namespace pludux {

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
  , macd_method_{SubtractMethod{CachedResultsEmaMethod{source_, short_period},
                                CachedResultsEmaMethod{source_, long_period}}}
  , signal_ema_{CachedResultsEmaMethod{macd_method_, signal_period}}
  {
  }

  auto operator==(const MacdMethod& other) const noexcept -> bool = default;

  auto operator()(this const MacdMethod& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return self(asset_snapshot, SeriesOutput::MacdLine, context);
  }

  auto operator()(this const MacdMethod& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    const auto macd = self.macd_method_(asset_snapshot, context);
    const auto signal = self.signal_ema_(asset_snapshot, context);
    const auto histogram = macd - signal;

    switch(output) {
    case SeriesOutput::MacdLine:
      return macd;
    case SeriesOutput::SignalLine:
      return signal;
    case SeriesOutput::Histogram:
      return histogram;
    default:
      return std::numeric_limits<ResultType>::quiet_NaN();
    }
  }

  auto source(this const MacdMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this MacdMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
    self.update_internal_methods();
  }

  auto short_period(this const MacdMethod& self) noexcept -> std::size_t
  {
    return self.short_period_;
  }

  void short_period(this MacdMethod& self, std::size_t period) noexcept
  {
    self.short_period_ = period;
    self.update_internal_methods();
  }

  auto fast_period(this const MacdMethod& self) noexcept -> std::size_t
  {
    return self.short_period_;
  }

  void fast_period(this MacdMethod& self, std::size_t period) noexcept
  {
    self.short_period_ = period;
    self.update_internal_methods();
  }

  auto long_period(this const MacdMethod& self) noexcept -> std::size_t
  {
    return self.long_period_;
  }

  void long_period(this MacdMethod& self, std::size_t period) noexcept
  {
    self.long_period_ = period;
    self.update_internal_methods();
  }

  auto slow_period(this const MacdMethod& self) noexcept -> std::size_t
  {
    return self.long_period_;
  }

  void slow_period(this MacdMethod& self, std::size_t period) noexcept
  {
    self.long_period_ = period;
    self.update_internal_methods();
  }

  auto signal_period(this const MacdMethod& self) noexcept -> std::size_t
  {
    return self.signal_period_;
  }

  void signal_period(this MacdMethod& self, std::size_t period) noexcept
  {
    self.signal_period_ = period;
    self.update_internal_methods();
  }

private:
  TSourceMethod source_;
  std::size_t short_period_;
  std::size_t long_period_;
  std::size_t signal_period_;

  SubtractMethod<CachedResultsEmaMethod<TSourceMethod>,
                 CachedResultsEmaMethod<TSourceMethod>>
   macd_method_;

  CachedResultsEmaMethod<SubtractMethod<CachedResultsEmaMethod<TSourceMethod>,
                                        CachedResultsEmaMethod<TSourceMethod>>>
   signal_ema_;

  void update_internal_methods(this MacdMethod& self) noexcept
  {
    self.macd_method_ =
     SubtractMethod{CachedResultsEmaMethod{self.source_, self.short_period_},
                    CachedResultsEmaMethod{self.source_, self.long_period_}};
    self.signal_ema_ =
     CachedResultsEmaMethod{self.macd_method_, self.signal_period_};
  }
};

} // namespace pludux
