module;

#include <cstddef>
#include <utility>

export module pludux:screener.lookback_method;

import :asset_snapshot;
import :screener.screener_method;

export namespace pludux::screener {

class LookbackMethod {
public:
  LookbackMethod(ScreenerMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const LookbackMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    return LookbackSeries{self.source_(asset_data), self.period_};
  }

  auto source(this const auto& self) noexcept -> ScreenerMethod
  {
    return self.source_;
  }

  void source(this auto& self, ScreenerMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const auto& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this auto& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  ScreenerMethod source_;
  std::size_t period_;
};

} // namespace pludux::screener