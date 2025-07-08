#ifndef PLUDUX_PLUDUX_SCREENER_CHANGES_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_CHANGES_METHOD_HPP

import pludux.asset_snapshot;
import pludux.series;

#include <cstddef>

#include <pludux/screener/screener_method.hpp>

namespace pludux::screener {

class ChangesMethod {
public:
  ChangesMethod();

  explicit ChangesMethod(ScreenerMethod input, std::size_t offset = 0);

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>;

  auto operator==(const ChangesMethod& other) const noexcept -> bool;

  auto offset() const noexcept -> std::size_t;

  void offset(std::size_t offset) noexcept;

  auto input() const noexcept -> const ScreenerMethod&;

  void input(ScreenerMethod input) noexcept;

private:
  ScreenerMethod input_;
  std::size_t offset_;
};

} // namespace pludux::screener

#endif
