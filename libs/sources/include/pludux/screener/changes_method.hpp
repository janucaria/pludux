#ifndef PLUDUX_PLUDUX_SCREENER_CHANGES_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_CHANGES_METHOD_HPP

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class ChangesMethod {
public:
  explicit ChangesMethod(ScreenerMethod operand);

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>;

private:
  ScreenerMethod operand_;
};

} // namespace pludux::screener

#endif
