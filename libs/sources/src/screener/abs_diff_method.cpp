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

auto AbsDiffMethod::run_all(const AssetDataProvider& asset_data) const -> PolySeries<double>
{
  const auto operand1_series = operand1_.run_all(asset_data);
  const auto operand2_series = operand2_.run_all(asset_data);

  using AbsDiffFn =
   std::remove_cvref_t<decltype([](double operand1, double operand2) {
     return std::abs(operand1 - operand2);
   })>;

  auto result = BinaryFnSeries<AbsDiffFn,
                               std::decay_t<decltype(operand1_series)>,
                               std::decay_t<decltype(operand2_series)>>{
   operand1_series, operand2_series};

  return result;
}

auto AbsDiffMethod::run_one(const AssetDataProvider& asset_data) const -> double
{
  const auto series = run_all(asset_data);
  return series[0];
}

} // namespace pludux::screener