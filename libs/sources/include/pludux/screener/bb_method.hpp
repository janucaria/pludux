#ifndef PLUDUX_PLUDUX_SCREENER_BB_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_BB_METHOD_HPP

#include <cstddef>

#include <pludux/screener/screener_method.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

class BbMethod {
public:
  enum class MaType { sma, ema, wma, rma, hma };

  BbMethod(MaType ma_type,
           ScreenerMethod input,
           std::size_t period,
           double stddev);

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>;

  auto operator==(const BbMethod& other) const noexcept -> bool;

  auto period() const noexcept -> std::size_t;

  void period(std::size_t period) noexcept;

  auto stddev() const noexcept -> double;

  void stddev(double stddev) noexcept;

  auto input() const noexcept -> ScreenerMethod;

  void input(ScreenerMethod input) noexcept;

  auto ma_type() const noexcept -> MaType;

  void ma_type(MaType ma_type) noexcept;

private:
  MaType ma_type_;
  ScreenerMethod input_;
  std::size_t period_;
  double stddev_;
};

} // namespace pludux::screener

#endif
