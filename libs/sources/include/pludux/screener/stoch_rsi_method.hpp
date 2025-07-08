#ifndef PLUDUX_PLUDUX_SCREENER_STOCH_RSI_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_STOCH_RSI_METHOD_HPP

import pludux.asset_snapshot;
import pludux.series;

#include <cstddef>

#include <pludux/screener/screener_method.hpp>

namespace pludux::screener {

class StochRsiMethod {
public:
  StochRsiMethod(StochOutput output,
                 ScreenerMethod rsi_input,
                 std::size_t rsi_period,
                 std::size_t k_period,
                 std::size_t k_smooth,
                 std::size_t d_period,
                 std::size_t offset = 0);

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>;

  auto operator==(const StochRsiMethod& other) const noexcept -> bool;

  auto rsi_input() const noexcept -> ScreenerMethod;

  auto rsi_period() const noexcept -> std::size_t;

  auto offset() const noexcept -> std::size_t;

  void offset(std::size_t offset) noexcept;

  auto output() const noexcept -> StochOutput;

  void output(StochOutput output) noexcept;

  auto k_period() const noexcept -> std::size_t;

  void k_period(std::size_t k_period) noexcept;

  auto k_smooth() const noexcept -> std::size_t;

  void k_smooth(std::size_t k_smooth) noexcept;

  auto d_period() const noexcept -> std::size_t;

  void d_period(std::size_t d_period) noexcept;

private:
  ScreenerMethod rsi_input_;
  StochOutput output_;
  std::size_t rsi_period_;
  std::size_t k_period_;
  std::size_t k_smooth_;
  std::size_t d_period_;
  std::size_t offset_;
};

} // namespace pludux::screener

#endif
