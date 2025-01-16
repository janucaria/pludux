#ifndef PLUDUX_PLUDUX_SCREENER_TA_WITH_PERIOD_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_TA_WITH_PERIOD_METHOD_HPP

#include <type_traits>
#include <utility>

#include <pludux/asset.hpp>
#include <pludux/asset_data_provider.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>
#include <pludux/ta.hpp>

namespace pludux::screener {

namespace details {

template<typename, typename T>
class TaWithPeroidMethod {
public:
  TaWithPeroidMethod(int period, ScreenerMethod target, int offset)
  : period_(period)
  , offset_(offset)
  , target_(std::move(target))
  {
  }

  auto
  operator()(const AssetDataProvider& asset_data) const -> PolySeries<double>
  {
    const auto sources = target_(asset_data);
    const auto series = T{}(sources, period_);
    return SubSeries{series, offset_};
  }

  auto period() const noexcept -> std::size_t
  {
    return period_;
  }

  auto offset() const noexcept -> std::size_t
  {
    return offset_;
  }

  auto target() const noexcept -> const ScreenerMethod&
  {
    return target_;
  }

private:
  std::size_t period_{};
  std::size_t offset_{};
  ScreenerMethod target_;
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
: public details::
   TaWithPeroidMethod<RsiMethod, std::decay_t<decltype(details::rsi_wrapper)>> {
  using details::TaWithPeroidMethod<
   RsiMethod,
   std::decay_t<decltype(details::rsi_wrapper)>>::TaWithPeroidMethod;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_MA_METHOD_HPP
