module;

#include <cstddef>
#include <utility>

export module pludux.screener.lookback_method;

import pludux.asset_snapshot;
import pludux.screener.screener_method;

export namespace pludux::screener {

class LookbackMethod {
public:
  LookbackMethod(ScreenerMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const LookbackMethod& other) const noexcept -> bool = default;

  auto operator()(this const LookbackMethod self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    return LookbackSeries{self.source_(asset_data), self.period_};
  }

  auto source(this const LookbackMethod self) noexcept -> ScreenerMethod
  {
    return self.source_;
  }

  void source(this LookbackMethod self, ScreenerMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const LookbackMethod self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this LookbackMethod self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  ScreenerMethod source_;
  std::size_t period_;
};

} // namespace pludux::screener