#ifndef PLUDUX_PLUDUX_SCREENER_ATR_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_ATR_METHOD_HPP

#include <cstddef>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_method.hpp>

namespace pludux::screener {

class AtrMethod {
public:
  AtrMethod();

  explicit AtrMethod(ScreenerMethod high,
                     ScreenerMethod low,
                     ScreenerMethod close,
                     std::size_t period,
                     double multiplier = 1.0,
                     std::size_t offset = 0);

  auto operator()(AssetSnapshot asset_data) const -> PolySeries<double>;

  auto operator==(const AtrMethod& other) const noexcept -> bool;

  auto period() const noexcept -> std::size_t;

  void period(std::size_t period) noexcept;

  auto multiplier() const noexcept -> double;

  void multiplier(double multiplier) noexcept;

  auto offset() const noexcept -> std::size_t;

  void offset(std::size_t offset) noexcept;

  auto high() const noexcept -> const ScreenerMethod&;

  void high(ScreenerMethod high) noexcept;

  auto low() const noexcept -> const ScreenerMethod&;

  void low(ScreenerMethod low) noexcept;

  auto close() const noexcept -> const ScreenerMethod&;

  void close(ScreenerMethod close) noexcept;

private:
  std::size_t period_;
  double multiplier_;
  std::size_t offset_;

  ScreenerMethod high_;
  ScreenerMethod low_;
  ScreenerMethod close_;
};

} // namespace pludux::screener

#endif
