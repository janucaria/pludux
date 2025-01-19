#ifndef PLUDUX_PLUDUX_SCREENER_ATR_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_ATR_METHOD_HPP

#include <cstddef>

#include <pludux/asset_snapshot.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class AtrMethod {
public:
  explicit AtrMethod(std::size_t period, std::size_t offset = 0);

  auto
  operator()(AssetSnapshot asset_data) const -> SubSeries<PolySeries<double>>;

private:
  std::size_t period_;
  std::size_t offset_;
};

} // namespace pludux::screener

#endif
