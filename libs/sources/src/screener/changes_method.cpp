#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset.hpp>
#include <pludux/ta.hpp>

#include <pludux/screener/changes_method.hpp>

namespace pludux::screener {

ChangesMethod::ChangesMethod(ScreenerMethod operand)
: operand_{operand}
{
}

auto ChangesMethod::operator()(const AssetDataProvider& asset_data) const -> PolySeries<double>
{
  const auto operand_series = operand_(asset_data);
  const auto result = ta::changes(operand_series);
  return result;
}


} // namespace pludux::screener