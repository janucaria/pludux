#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset.hpp>
#include <pludux/ta.hpp>

#include <pludux/screener/sma_method.hpp>

namespace pludux::screener {

SmaMethod::SmaMethod(int period, ScreenerMethod target, int offset)
  : period_{period}
  , offset_{offset}
  , target_{std::move(target)}
{
}

auto SmaMethod::run_one(const Asset& asset) const -> double
{
  const auto series = run_all(asset);
  return series[0];
}

auto SmaMethod::run_all(const Asset& asset) const -> Series
{
  const auto sources = target_.run_all(asset);
  const auto series = pludux::ta::sma(sources, period_);
  return series.subseries(offset_);
}

} // namespace pludux::screener