#ifndef PLUDUX_PLUDUX_SCREENER_HIDDEN_BULLISH_DIVERGENCE_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_HIDDEN_BULLISH_DIVERGENCE_METHOD_HPP

#include <cstddef>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class HiddenBullishDivergenceMethod {
public:
  explicit HiddenBullishDivergenceMethod(ScreenerMethod signal,
                                         ScreenerMethod reference,
                                         std::size_t pivot_range,
                                         std::size_t lookback_range,
                                         std::size_t offset = 0);

  auto
  operator()(AssetSnapshot asset_data) const -> SubSeries<PolySeries<double>>;

  auto operator==(const HiddenBullishDivergenceMethod& other) const noexcept
   -> bool;

private:
  std::size_t offset_;
  std::size_t pivot_range_;
  std::size_t lookback_range_;

  ScreenerMethod signal_;
  ScreenerMethod reference_;
};

} // namespace pludux::screener

#endif
