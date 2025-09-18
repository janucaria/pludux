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

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>
  {
    const auto ma_series = ma_(asset_data);
    const auto range_series = range_(asset_data);

    const auto kc = KcSeries{ma_series, range_series, multiplier_};

    return kc;
  }

  auto multiplier() const noexcept -> double
  {
    return multiplier_;
  }

  void multiplier(double multiplier) noexcept
  {
    multiplier_ = multiplier;
  }

  auto ma() const noexcept -> ScreenerMethod
  {
    return ma_;
  }

  void ma(ScreenerMethod ma) noexcept
  {
    ma_ = std::move(ma);
  }

  auto range() const noexcept -> ScreenerMethod
  {
    return range_;
  }

  void range(ScreenerMethod range) noexcept
  {
    range_ = std::move(range);
  }

private:
  double multiplier_;
  ScreenerMethod ma_;
  ScreenerMethod range_;
};

} // namespace pludux::screener