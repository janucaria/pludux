module;

#include <cstddef>
#include <utility>

export module pludux:screener.kc_method;

import :asset_snapshot;
import :screener.screener_method;

export namespace pludux::screener {

class KcMethod {
public:
  KcMethod(ScreenerMethod ma, ScreenerMethod range, double multiplier)
  : multiplier_{multiplier}
  , ma_{std::move(ma)}
  , range_{std::move(range)}
  {
  }

  auto operator==(const KcMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self, AssetSnapshot asset_data)
   -> PolySeries<double>
  {
    const auto ma_series = self.ma_(asset_data);
    const auto range_series = self.range_(asset_data);

    const auto kc = KcSeries{ma_series, range_series, self.multiplier_};

    return kc;
  }

  auto multiplier(this const auto& self) noexcept -> double
  {
    return self.multiplier_;
  }

  void multiplier(this auto& self, double multiplier) noexcept
  {
    self.multiplier_ = multiplier;
  }

  auto ma(this const auto& self) noexcept -> ScreenerMethod
  {
    return self.ma_;
  }

  void ma(this auto& self, ScreenerMethod ma) noexcept
  {
    self.ma_ = std::move(ma);
  }

  auto range(this const auto& self) noexcept -> ScreenerMethod
  {
    return self.range_;
  }

  void range(this auto& self, ScreenerMethod range) noexcept
  {
    self.range_ = std::move(range);
  }

private:
  double multiplier_;
  ScreenerMethod ma_;
  ScreenerMethod range_;
};

} // namespace pludux::screener