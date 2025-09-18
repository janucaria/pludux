module;

#include <cstddef>
#include <type_traits>
#include <utility>

export module pludux:screener.ta_with_period_method;

import :asset_snapshot;
import :screener.screener_method;
import :screener.ohlcv_method;

namespace pludux::screener {

template<typename, typename T>
class TaWithPeriodMethod {
public:
  TaWithPeriodMethod(ScreenerMethod input, std::size_t period)
  : period_{period}
  , input_{std::move(input)}
  {
  }

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    const auto sources = input_(asset_data);
    const auto series = T{}(sources, period_);
    return series;
  }

  auto operator==(const TaWithPeriodMethod& other) const noexcept
   -> bool = default;

  auto period() const noexcept -> std::size_t
  {
    return period_;
  }

  void period(std::size_t period) noexcept
  {
    period_ = period;
  }

  auto input() const noexcept -> const ScreenerMethod&
  {
    return input_;
  }

  void input(ScreenerMethod input) noexcept
  {
    input_ = std::move(input);
  }

private:
  std::size_t period_{};
  ScreenerMethod input_;
};

inline constexpr auto sma_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return SmaSeries{series, period};
};

inline constexpr auto ema_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return EmaSeries{series, period};
};

inline constexpr auto wma_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return WmaSeries{series, period};
};

inline constexpr auto rma_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return RmaSeries{series, period};
};

inline constexpr auto hma_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return HmaSeries{series, period};
};

inline constexpr auto rsi_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return RsiSeries{series, period};
};

inline constexpr auto roc_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return RocSeries{series, period};
};

inline constexpr auto rvol_wrapper = []<typename TSeries>(TSeries series,
                                                          std::size_t period) {
  return RvolSeries{series, period};
};

export class SmaMethod
: public TaWithPeriodMethod<SmaMethod, std::decay_t<decltype(sma_wrapper)>> {
public:
  explicit SmaMethod(ScreenerMethod input = CloseMethod{},
                     std::size_t period = 20)
  : TaWithPeriodMethod<SmaMethod, std::decay_t<decltype(sma_wrapper)>>{
     std::move(input), period}
  {
  }
};

export class EmaMethod
: public TaWithPeriodMethod<EmaMethod, std::decay_t<decltype(ema_wrapper)>> {
public:
  explicit EmaMethod(ScreenerMethod input = CloseMethod{},
                     std::size_t period = 20)
  : TaWithPeriodMethod<EmaMethod, std::decay_t<decltype(ema_wrapper)>>{
     std::move(input), period}
  {
  }
};

export class WmaMethod
: public TaWithPeriodMethod<WmaMethod, std::decay_t<decltype(wma_wrapper)>> {
public:
  explicit WmaMethod(ScreenerMethod input = CloseMethod{},
                     std::size_t period = 20)
  : TaWithPeriodMethod<WmaMethod, std::decay_t<decltype(wma_wrapper)>>{
     std::move(input), period}
  {
  }
};

export class RmaMethod
: public TaWithPeriodMethod<RmaMethod, std::decay_t<decltype(rma_wrapper)>> {
public:
  explicit RmaMethod(ScreenerMethod input = CloseMethod{},
                     std::size_t period = 20)
  : TaWithPeriodMethod<RmaMethod, std::decay_t<decltype(rma_wrapper)>>{
     std::move(input), period}
  {
  }
};

export class HmaMethod
: public TaWithPeriodMethod<HmaMethod, std::decay_t<decltype(hma_wrapper)>> {
public:
  explicit HmaMethod(ScreenerMethod input = CloseMethod{},
                     std::size_t period = 20)
  : TaWithPeriodMethod<HmaMethod, std::decay_t<decltype(hma_wrapper)>>{
     std::move(input), period}
  {
  }
};

export class RsiMethod
: public TaWithPeriodMethod<RsiMethod, std::decay_t<decltype(rsi_wrapper)>> {
public:
  explicit RsiMethod(ScreenerMethod input = CloseMethod{},
                     std::size_t period = 14)
  : TaWithPeriodMethod<RsiMethod, std::decay_t<decltype(rsi_wrapper)>>{
     std::move(input), period}
  {
  }
};

export class RocMethod
: public TaWithPeriodMethod<RocMethod, std::decay_t<decltype(roc_wrapper)>> {
public:
  explicit RocMethod(ScreenerMethod input = CloseMethod{},
                     std::size_t period = 20)
  : TaWithPeriodMethod<RocMethod, std::decay_t<decltype(roc_wrapper)>>{
     std::move(input), period}
  {
  }
};

export class RvolMethod
: public TaWithPeriodMethod<RvolMethod, std::decay_t<decltype(rvol_wrapper)>> {
public:
  explicit RvolMethod(ScreenerMethod input = CloseMethod{},
                      std::size_t period = 20)
  : TaWithPeriodMethod<RvolMethod, std::decay_t<decltype(rvol_wrapper)>>{
     std::move(input), period}
  {
  }
};

} // namespace pludux::screener
