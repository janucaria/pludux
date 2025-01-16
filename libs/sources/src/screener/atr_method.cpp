#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset.hpp>
#include <pludux/screener/atr_method.hpp>
#include <pludux/ta.hpp>

namespace pludux::screener {

AtrMethod::AtrMethod(int period)
: period_{period}
{
}

auto AtrMethod::operator()(const AssetDataProvider& asset_data) const -> PolySeries<double>
{
  const auto high_series = asset_data.high();
  const auto low_series = asset_data.low();
  const auto close_series = asset_data.close();

  const auto atr = ta::atr(high_series, low_series, close_series, period_);
  return atr;
}

} // namespace pludux::screener