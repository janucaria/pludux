#ifndef PLUDUX_PLUDUX_SCREENER_CHANGES_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_CHANGES_METHOD_HPP

#include <cstddef>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class ChangesMethod {
public:
  explicit ChangesMethod(ScreenerMethod operand, std::size_t offset = 0);

  auto
  operator()(AssetSnapshot asset_data) const -> SubSeries<PolySeries<double>>;

private:
  ScreenerMethod operand_;
  std::size_t offset_;
};

} // namespace pludux::screener

#endif
