#include <algorithm>
#include <format>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <pludux/asset_snapshot.hpp>

#include <pludux/screener/lookback_method.hpp>

namespace pludux::screener {

LookbackMethod::LookbackMethod(ScreenerMethod source, std::size_t period)
: source_{std::move(source)}
, period_{period}
{
}

auto LookbackMethod::operator==(const LookbackMethod& other) const noexcept
 -> bool = default;

auto LookbackMethod::operator()(this const LookbackMethod self,
                                AssetSnapshot asset_data) -> PolySeries<double>
{
  return LookbackSeries{self.source_(asset_data), self.period_};
}

auto LookbackMethod::source(this const LookbackMethod self) noexcept
 -> ScreenerMethod
{
  return self.source_;
}

void LookbackMethod::source(this LookbackMethod self,
                            ScreenerMethod source) noexcept
{
  self.source_ = std::move(source);
}

auto LookbackMethod::period(this const LookbackMethod self) noexcept
 -> std::size_t
{
  return self.period_;
}

void LookbackMethod::period(this LookbackMethod self,
                            std::size_t period) noexcept
{
  self.period_ = period;
}

} // namespace pludux::screener