#ifndef PLUDUX_PLUDUX_SCREENER_TA_WITH_PERIOD_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_TA_WITH_PERIOD_METHOD_HPP

#include <type_traits>
#include <utility>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/ohlcv_method.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/ta.hpp>

namespace pludux::screener {

namespace details {

template<typename, typename T, std::size_t default_period = 20>
class TaWithPeroidMethod {
public:
  TaWithPeroidMethod()
  : TaWithPeroidMethod{default_period, CloseMethod{}, 0}
  {
  }

  TaWithPeroidMethod(int period, ScreenerMethod input, int offset)
  : period_(period)
  , offset_(offset)
  , input_(std::move(input))
  {
  }

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>
  {
    const auto sources = input_(asset_data);
    const auto series = T{}(sources, period_);
    return SubSeries{PolySeries<double>{series},
                     static_cast<std::ptrdiff_t>(offset_)};
  }

  auto operator==(const TaWithPeroidMethod& other) const noexcept
   -> bool = default;

  auto period() const noexcept -> std::size_t
  {
    return period_;
  }

  void period(std::size_t period) noexcept
  {
    period_ = period;
  }

  auto offset() const noexcept -> std::size_t
  {
    return offset_;
  }

  void offset(std::size_t offset) noexcept
  {
    offset_ = offset;
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
  std::size_t offset_{};
  ScreenerMethod input_;
};

inline constexpr auto sma_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return ta::sma(series, period);
};

inline constexpr auto ema_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return ta::ema(series, period);
};

inline constexpr auto wma_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return ta::wma(series, period);
};

inline constexpr auto rma_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return ta::rma(series, period);
};

inline constexpr auto hma_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return ta::hma(series, period);
};

inline constexpr auto rsi_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return ta::rsi(series, period);
};

inline constexpr auto roc_wrapper = []<typename TSeries>(TSeries series,
                                                         std::size_t period) {
  return RocSeries{series, period};
};

inline constexpr auto rvol_wrapper = []<typename TSeries>(TSeries series,
                                                          std::size_t period) {
  return RvolSeries{series, period};
};

} // namespace details

class SmaMethod
: public details::
   TaWithPeroidMethod<SmaMethod, std::decay_t<decltype(details::sma_wrapper)>> {
  using details::TaWithPeroidMethod<
   SmaMethod,
   std::decay_t<decltype(details::sma_wrapper)>>::TaWithPeroidMethod;
};

class EmaMethod
: public details::
   TaWithPeroidMethod<EmaMethod, std::decay_t<decltype(details::ema_wrapper)>> {
  using details::TaWithPeroidMethod<
   EmaMethod,
   std::decay_t<decltype(details::ema_wrapper)>>::TaWithPeroidMethod;
};

class WmaMethod
: public details::
   TaWithPeroidMethod<WmaMethod, std::decay_t<decltype(details::wma_wrapper)>> {
  using details::TaWithPeroidMethod<
   WmaMethod,
   std::decay_t<decltype(details::wma_wrapper)>>::TaWithPeroidMethod;
};

class RmaMethod
: public details::
   TaWithPeroidMethod<RmaMethod, std::decay_t<decltype(details::rma_wrapper)>> {
  using details::TaWithPeroidMethod<
   RmaMethod,
   std::decay_t<decltype(details::rma_wrapper)>>::TaWithPeroidMethod;
};

class HmaMethod
: public details::
   TaWithPeroidMethod<HmaMethod, std::decay_t<decltype(details::hma_wrapper)>> {
  using details::TaWithPeroidMethod<
   HmaMethod,
   std::decay_t<decltype(details::hma_wrapper)>>::TaWithPeroidMethod;
};

class RsiMethod
: public details::TaWithPeroidMethod<
   RsiMethod,
   std::decay_t<decltype(details::rsi_wrapper)>,
   14> {
  using details::TaWithPeroidMethod<
   RsiMethod,
   std::decay_t<decltype(details::rsi_wrapper)>,
   14>::TaWithPeroidMethod;
};

class RocMethod
: public details::
   TaWithPeroidMethod<RocMethod, std::decay_t<decltype(details::roc_wrapper)>> {
  using details::TaWithPeroidMethod<
   RocMethod,
   std::decay_t<decltype(details::roc_wrapper)>>::TaWithPeroidMethod;
};

class RvolMethod
: public details::TaWithPeroidMethod<
   RvolMethod,
   std::decay_t<decltype(details::rvol_wrapper)>> {
  using details::TaWithPeroidMethod<
   RvolMethod,
   std::decay_t<decltype(details::rvol_wrapper)>>::TaWithPeroidMethod;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_MA_METHOD_HPP
