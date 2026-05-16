module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

export module pludux:series.kc_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

import :series.ohlcv_method;
import :series.ma_method_type;

export namespace pludux {

enum class KcBandMethodType { Atr, Tr, RangeHighLow };

template<typename TSourceMethod = CloseMethod>
class KcMethod {
public:
  KcMethod()
  : KcMethod{10, 1.5, 14}
  {
  }

  KcMethod(std::size_t period, double multiplier, std::size_t band_atr_period)
  : KcMethod{TSourceMethod{}, period, multiplier, band_atr_period}
  {
  }

  KcMethod(std::size_t period,
           double multiplier,
           std::size_t band_atr_period,
           KcBandMethodType band_method_type,
           MaMethodType ma_method_type)
  : KcMethod{TSourceMethod{},
             period,
             multiplier,
             band_atr_period,
             band_method_type,
             ma_method_type}
  {
  }

  KcMethod(TSourceMethod source,
           std::size_t period,
           double multiplier,
           std::size_t band_atr_period,
           KcBandMethodType band_method_type = KcBandMethodType::Atr,
           MaMethodType ma_method_type = MaMethodType::Ema)
  : source_{std::move(source)}
  , period_{period}
  , multiplier_{multiplier}
  , band_atr_period_{band_atr_period}
  , band_method_type_{band_method_type}
  , ma_method_type_{ma_method_type}
  {
  }

  auto operator==(const KcMethod& other) const noexcept -> bool = default;

  auto source(this const KcMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this KcMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto ma_method_type(this const KcMethod& self) noexcept -> MaMethodType
  {
    return self.ma_method_type_;
  }

  void ma_method_type(this KcMethod& self, MaMethodType ma_method_type) noexcept
  {
    self.ma_method_type_ = ma_method_type;
  }

  auto period(this const KcMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this KcMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

  auto band_method_type(this const KcMethod& self) noexcept -> KcBandMethodType
  {
    return self.band_method_type_;
  }

  void band_method_type(this KcMethod& self,
                        KcBandMethodType band_method_type) noexcept
  {
    self.band_method_type_ = band_method_type;
  }

  auto band_atr_period(this const KcMethod& self) noexcept -> std::size_t
  {
    return self.band_atr_period_;
  }

  void band_atr_period(this KcMethod& self,
                       std::size_t band_atr_period) noexcept
  {
    self.band_atr_period_ = band_atr_period;
  }

  auto multiplier(this const KcMethod& self) noexcept -> double
  {
    return self.multiplier_;
  }

  void multiplier(this KcMethod& self, double multiplier) noexcept
  {
    self.multiplier_ = multiplier;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
  double multiplier_;

  std::size_t band_atr_period_;
  KcBandMethodType band_method_type_;
  MaMethodType ma_method_type_;
};

KcMethod() -> KcMethod<CloseMethod>;

} // namespace pludux