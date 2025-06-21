#ifndef PLUDUX_PLUDUX_SCREENER_ABS_DIFF_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_ABS_DIFF_METHOD_HPP

#include <cstddef>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class AbsDiffMethod {
public:
  explicit AbsDiffMethod(ScreenerMethod operand1,
                         ScreenerMethod operand2,
                         std::size_t offset = 0);

  auto
  operator()(AssetSnapshot asset_data) const -> SubSeries<PolySeries<double>>;

  auto operator==(const AbsDiffMethod& other) const noexcept -> bool;

private:
  ScreenerMethod operand1_;
  ScreenerMethod operand2_;
  std::size_t offset_;
};

} // namespace pludux::screener

#endif
