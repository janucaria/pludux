#ifndef PLUDUX_PLUDUX_SCREENER_HIDDEN_BULLISH_DIVERGENCE_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_HIDDEN_BULLISH_DIVERGENCE_METHOD_HPP

#include <cstddef>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

import pludux.asset_snapshot;

namespace pludux::screener {

class HiddenBullishDivergenceMethod {
public:
  HiddenBullishDivergenceMethod();

  explicit HiddenBullishDivergenceMethod(ScreenerMethod signal,
                                         ScreenerMethod reference,
                                         std::size_t pivot_range,
                                         std::size_t lookback_range);

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>;

  auto operator==(const HiddenBullishDivergenceMethod& other) const noexcept
   -> bool;

  auto pivot_range() const noexcept -> std::size_t;

  void pivot_range(std::size_t pivot_range) noexcept;

  auto lookback_range() const noexcept -> std::size_t;

  void lookback_range(std::size_t lookback_range) noexcept;

  auto signal() const noexcept -> const ScreenerMethod&;

  void signal(ScreenerMethod signal) noexcept;

  auto reference() const noexcept -> const ScreenerMethod&;

  void reference(ScreenerMethod reference) noexcept;

private:
  std::size_t pivot_range_;
  std::size_t lookback_range_;

  ScreenerMethod signal_;
  ScreenerMethod reference_;
};

} // namespace pludux::screener

#endif
