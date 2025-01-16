#ifndef PLUDUX_PLUDUX_SCREENER_ABS_DIFF_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_ABS_DIFF_METHOD_HPP

#include <pludux/asset_data_provider.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class AbsDiffMethod {
public:
  explicit AbsDiffMethod(ScreenerMethod operand1, ScreenerMethod operand2);

  auto operator()(const AssetDataProvider& asset_data) const -> PolySeries<double>;

private:
  ScreenerMethod operand1_;
  ScreenerMethod operand2_;
};

} // namespace pludux::screener

#endif
