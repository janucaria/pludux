#ifndef PLUDUX_PLUDUX_SCREENER_ATR_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_ATR_METHOD_HPP

#include <cstddef>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_method.hpp>

namespace pludux::screener {

class AtrMethod {
public:
  explicit AtrMethod(ScreenerMethod high,
                     ScreenerMethod low,
                     ScreenerMethod close,
                     std::size_t period,
                     double multiplier = 1.0,
                     std::size_t offset = 0);

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>;

private:
  std::size_t period_;
  double multiplier_;
  std::size_t offset_;

  ScreenerMethod high_;
  ScreenerMethod low_;
  ScreenerMethod close_;
};

} // namespace pludux::screener

#endif
