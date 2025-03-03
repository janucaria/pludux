#include <algorithm>
#include <iterator>
#include <vector>

#include <pludux/asset_snapshot.hpp>
#include <pludux/ta.hpp>

#include <pludux/screener/changes_method.hpp>

namespace pludux::screener {

ChangesMethod::ChangesMethod(ScreenerMethod operand, std::size_t offset)
: operand_{operand}
, offset_{offset}
{
}

auto ChangesMethod::operator()(AssetSnapshot asset_data) const
 -> SubSeries<PolySeries<double>>
{
  const auto operand_series = operand_(asset_data);
  const auto result = ta::changes(operand_series);
  return SubSeries{PolySeries<double>{result},
                   static_cast<std::ptrdiff_t>(offset_)};
}

} // namespace pludux::screener