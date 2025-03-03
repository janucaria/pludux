#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset_snapshot.hpp>

#include <pludux/screener/abs_diff_method.hpp>

namespace pludux::screener {

AbsDiffMethod::AbsDiffMethod(ScreenerMethod operand1,
                             ScreenerMethod operand2,
                             std::size_t offset)
: operand1_{operand1}
, operand2_{operand2}
, offset_{offset}
{
}

auto AbsDiffMethod::operator()(AssetSnapshot asset_data) const
 -> SubSeries<PolySeries<double>>
{
  const auto operand1_series = operand1_(asset_data);
  const auto operand2_series = operand2_(asset_data);

  using AbsDiffFn =
   std::remove_cvref_t<decltype([](double operand1, double operand2) {
     return std::abs(operand1 - operand2);
   })>;

  const auto result = BinaryFnSeries<AbsDiffFn,
                                     std::decay_t<decltype(operand1_series)>,
                                     std::decay_t<decltype(operand2_series)>>{
   operand1_series, operand2_series};

  return SubSeries{PolySeries<double>{result},
                   static_cast<std::ptrdiff_t>(offset_)};
}

} // namespace pludux::screener