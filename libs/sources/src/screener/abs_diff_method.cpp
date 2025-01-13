#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset.hpp>

#include <pludux/screener/abs_diff_method.hpp>

namespace pludux::screener {

AbsDiffMethod::AbsDiffMethod(ScreenerMethod operand1, ScreenerMethod operand2)
: operand1_{operand1}
, operand2_{operand2}
{
}

auto AbsDiffMethod::run_all(const AssetDataProvider& asset_data) const -> Series
{
  const auto operand1_series = operand1_.run_all(asset_data);
  const auto operand2_series = operand2_.run_all(asset_data);

  auto result = std::vector<double>{};
  std::transform(operand1_series.data().begin(),
                 operand1_series.data().end(),
                 operand2_series.data().begin(),
                 std::back_inserter(result),
                 [](double operand1, double operand2) {
                   return std::abs(operand1 - operand2);
                 });

  return Series{std::move(result)};
}

auto AbsDiffMethod::run_one(const AssetDataProvider& asset_data) const -> double
{
  const auto series = run_all(asset_data);
  return series[0];
}

} // namespace pludux::screener