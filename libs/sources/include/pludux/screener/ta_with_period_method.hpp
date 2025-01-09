#ifndef PLUDUX_PLUDUX_SCREENER_TA_WITH_PERIOD_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_TA_WITH_PERIOD_METHOD_HPP

#include <type_traits>
#include <utility>

#include <pludux/asset.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>
#include <pludux/ta.hpp>
#include <pludux/asset_data_provider.hpp>

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

  auto run_one(const AssetDataProvider& asset_data) const -> double
  {
    const auto series = run_all(asset_data);
    return series[0];
  }

  auto run_all(const AssetDataProvider& asset_data) const -> Series
  {
    const auto sources = target_.run_all(asset_data);
    const auto series = T{}(sources, period_);
    return series.subseries(offset_);
  }

  auto period() const noexcept -> int
  {
    return period_;
  }

  auto offset() const noexcept -> int
  {
    return offset_;
  }

  auto target() const noexcept -> const ScreenerMethod&
  {
    return target_;
  }

private:
  int period_{};
  int offset_{};
  ScreenerMethod target_;
};

inline constexpr auto sma_wrapper = [](const Series& series,
                                       std::size_t period) {
  return ta::sma(series, period);
};

inline constexpr auto ema_wrapper = [](const Series& series,
                                       std::size_t period) {
  return ta::ema(series, period);
};

inline constexpr auto wma_wrapper = [](const Series& series,
                                       std::size_t period) {
  return ta::wma(series, period);
};

inline constexpr auto rma_wrapper = [](const Series& series,
                                       std::size_t period) {
  return ta::rma(series, period);
};

inline constexpr auto hma_wrapper = [](const Series& series,
                                       std::size_t period) {
  return ta::hma(series, period);
};

inline constexpr auto rsi_wrapper = [](const Series& series,
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
