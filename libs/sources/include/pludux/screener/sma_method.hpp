#ifndef PLUDUX_PLUDUX_SCREENER_SMA_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_SMA_METHOD_HPP

#include <vector>

#include <pludux/asset.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class SmaMethod {
public:
  SmaMethod(int period, ScreenerMethod target, int offset);

  auto run_one(const Asset& asset) const -> double;

  auto run_all(const Asset& asset) const -> Series;

private:
  int period_{};
  int offset_{};
  ScreenerMethod target_;
};

} // namespace pludux::screener

#endif // PLUDUX_PLUDUX_SCREENER_SMA_METHOD_HPP
