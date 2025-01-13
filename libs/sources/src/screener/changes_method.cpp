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

auto ChangesMethod::run_all(const AssetDataProvider& asset_data) const -> Series
{
  const auto operand_series = operand_.run_all(asset_data);
  const auto result = ta::changes(operand_series);
  return result;
}

auto ChangesMethod::run_one(const AssetDataProvider& asset_data) const -> double
{
  const auto series = run_all(asset_data);
  return series[0];
}

} // namespace pludux::screener