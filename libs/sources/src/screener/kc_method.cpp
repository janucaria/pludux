#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/kc_method.hpp>

namespace pludux::screener {

KcMethod::KcMethod(ScreenerMethod ma, ScreenerMethod range, double multiplier)
: multiplier_{multiplier}
, ma_{std::move(ma)}
, range_{std::move(range)}
{
}

auto KcMethod::operator()(AssetSnapshot asset_data) const -> PolySeries<double>
{
  const auto ma_series = ma_(asset_data);
  const auto range_series = range_(asset_data);

  const auto kc = KcSeries{ma_series, range_series, multiplier_};

  return kc;
}

auto KcMethod::operator==(const KcMethod& other) const noexcept
 -> bool = default;

auto KcMethod::multiplier() const noexcept -> double
{
  return multiplier_;
}

void KcMethod::multiplier(double multiplier) noexcept
{
  multiplier_ = multiplier;
}

auto KcMethod::ma() const noexcept -> ScreenerMethod
{
  return ma_;
}

void KcMethod::ma(ScreenerMethod ma) noexcept
{
  ma_ = std::move(ma);
}

auto KcMethod::range() const noexcept -> ScreenerMethod
{
  return range_;
}

void KcMethod::range(ScreenerMethod range) noexcept
{
  range_ = std::move(range);
}

} // namespace pludux::screener