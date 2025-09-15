#ifndef PLUDUX_PLUDUX_SCREENER_KC_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_KC_METHOD_HPP

#include <cstddef>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class KcMethod {
public:
  KcMethod(OutputName output,
           ScreenerMethod ma,
           ScreenerMethod range,
           double multiplier,
           std::size_t offset = 0);

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>;

  auto operator==(const KcMethod& other) const noexcept -> bool;

  auto offset() const noexcept -> std::size_t;

  void offset(std::size_t offset) noexcept;

  auto output() const noexcept -> OutputName;

  void output(OutputName output) noexcept;

  auto multiplier() const noexcept -> double;

  void multiplier(double multiplier) noexcept;

  auto ma() const noexcept -> ScreenerMethod;

  void ma(ScreenerMethod ma) noexcept;

  auto range() const noexcept -> ScreenerMethod;

  void range(ScreenerMethod range) noexcept;

private:
  std::size_t offset_;
  OutputName output_;
  double multiplier_;
  ScreenerMethod ma_;
  ScreenerMethod range_;
};

} // namespace pludux::screener

#endif
