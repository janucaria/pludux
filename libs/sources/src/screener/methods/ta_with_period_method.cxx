module;

#include <cstddef>
#include <type_traits>
#include <utility>

export module pludux:screener.ta_with_period_method;

import :asset_snapshot;
import :screener.screener_method;
import :screener.ohlcv_method;

namespace pludux::screener {

template<typename, template<typename...> typename TSeries>
class TaWithPeriodMethod {
public:
  TaWithPeriodMethod(ScreenerMethod input, std::size_t period)
  : period_{period}
  , input_{std::move(input)}
  {
  }

  auto operator==(const TaWithPeriodMethod& other) const noexcept
   -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    const auto sources = self.input_(asset_data);
    const auto series = TSeries{sources, self.period_};
    return series;
  }

  auto period(this const auto& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this auto& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

  auto input(this const auto& self) noexcept -> const ScreenerMethod&
  {
    return self.input_;
  }

  void input(this auto& self, ScreenerMethod input) noexcept
  {
    self.input_ = std::move(input);
  }

private:
  std::size_t period_{};
  ScreenerMethod input_;
};

export class SmaMethod : public TaWithPeriodMethod<SmaMethod, SmaSeries> {
public:
  SmaMethod()
  : SmaMethod{CloseMethod{}, 20}
  {
  }

  SmaMethod(ScreenerMethod input, std::size_t period)
  : TaWithPeriodMethod<SmaMethod, SmaSeries>{std::move(input), period}
  {
  }
};

export class EmaMethod : public TaWithPeriodMethod<EmaMethod, EmaSeries> {
public:
  EmaMethod()
  : EmaMethod{CloseMethod{}, 20}
  {
  }

  EmaMethod(ScreenerMethod input, std::size_t period)
  : TaWithPeriodMethod<EmaMethod, EmaSeries>{std::move(input), period}
  {
  }
};

export class WmaMethod : public TaWithPeriodMethod<WmaMethod, WmaSeries> {
public:
  WmaMethod()
  : WmaMethod{CloseMethod{}, 20}
  {
  }

  WmaMethod(ScreenerMethod input, std::size_t period)
  : TaWithPeriodMethod<WmaMethod, WmaSeries>{std::move(input), period}
  {
  }
};

export class RmaMethod : public TaWithPeriodMethod<RmaMethod, RmaSeries> {
public:
  RmaMethod()
  : RmaMethod{CloseMethod{}, 20}
  {
  }

  RmaMethod(ScreenerMethod input, std::size_t period)
  : TaWithPeriodMethod<RmaMethod, RmaSeries>{std::move(input), period}
  {
  }
};

export class HmaMethod : public TaWithPeriodMethod<HmaMethod, HmaSeries> {
public:
  HmaMethod()
  : HmaMethod{CloseMethod{}, 20}
  {
  }

  HmaMethod(ScreenerMethod input, std::size_t period)
  : TaWithPeriodMethod<HmaMethod, HmaSeries>{std::move(input), period}
  {
  }
};

export class RsiMethod : public TaWithPeriodMethod<RsiMethod, RsiSeries> {
public:
  RsiMethod()
  : RsiMethod{CloseMethod{}, 14}
  {
  }

  RsiMethod(ScreenerMethod input, std::size_t period)
  : TaWithPeriodMethod<RsiMethod, RsiSeries>{std::move(input), period}
  {
  }
};

export class RocMethod : public TaWithPeriodMethod<RocMethod, RocSeries> {
public:
  RocMethod()
  : RocMethod{CloseMethod{}, 20}
  {
  }

  RocMethod(ScreenerMethod input, std::size_t period)
  : TaWithPeriodMethod<RocMethod, RocSeries>{std::move(input), period}
  {
  }
};

export class RvolMethod : public TaWithPeriodMethod<RvolMethod, RvolSeries> {
public:
  RvolMethod()
  : RvolMethod{CloseMethod{}, 20}
  {
  }

  RvolMethod(ScreenerMethod input, std::size_t period)
  : TaWithPeriodMethod<RvolMethod, RvolSeries>{std::move(input), period}
  {
  }
};

} // namespace pludux::screener
