#ifndef PLUDUX_PLUDUX_SCREENER_MACD_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_MACD_METHOD_HPP

#include <cstddef>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class MacdMethod {
public:
  MacdMethod(MacdOutput output,
             ScreenerMethod input,
             std::size_t fast_period,
             std::size_t slow_period,
             std::size_t signal_period,
             std::size_t offset = 0);

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>;

  auto operator==(const MacdMethod& other) const noexcept -> bool;

  auto offset() const noexcept -> std::size_t;

  void offset(std::size_t offset) noexcept;

  auto output() const noexcept -> MacdOutput;

  void output(MacdOutput output) noexcept;

  auto input() const noexcept -> ScreenerMethod;

  void input(ScreenerMethod input) noexcept;

  auto fast_period() const noexcept -> std::size_t;

  void fast_period(std::size_t fast_period) noexcept;

  auto slow_period() const noexcept -> std::size_t;

  void slow_period(std::size_t slow_period) noexcept;

  auto signal_period() const noexcept -> std::size_t;

  void signal_period(std::size_t signal_period) noexcept;

private:
  MacdOutput output_;
  ScreenerMethod input_;
  std::size_t offset_;
  std::size_t fast_period_;
  std::size_t slow_period_;
  std::size_t signal_period_;
};

} // namespace pludux::screener

#endif
