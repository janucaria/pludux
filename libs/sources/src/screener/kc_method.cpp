#include <pludux/screener/kc_method.hpp>

namespace pludux::screener {

KcMethod::KcMethod(KcOutput output,
                   ScreenerMethod ma,
                   ScreenerMethod range,
                   double multiplier,
                   std::size_t offset)
: offset_{offset}
, output_{output}
, multiplier_{multiplier}
, ma_{std::move(ma)}
, range_{std::move(range)}
{
}

auto KcMethod::operator()(AssetSnapshot asset_data) const
 -> SubSeries<PolySeries<double>>
{
  const auto ma_series = ma_(asset_data);
  const auto range_series = range_(asset_data);

  const auto kc = KcSeries{output_, ma_series, range_series, multiplier_};
  return SubSeries{PolySeries<double>{kc},
                   static_cast<std::ptrdiff_t>(offset_)};
}

auto KcMethod::operator==(const KcMethod& other) const noexcept
 -> bool = default;

auto KcMethod::offset() const noexcept -> std::size_t
{
  return offset_;
}

void KcMethod::offset(std::size_t offset) noexcept
{
  offset_ = offset;
}

auto KcMethod::output() const noexcept -> KcOutput
{
  return output_;
}

void KcMethod::output(KcOutput output) noexcept
{
  output_ = output;
}

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