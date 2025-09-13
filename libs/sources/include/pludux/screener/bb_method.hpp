#ifndef PLUDUX_PLUDUX_SCREENER_BB_METHOD_HPP
#define PLUDUX_PLUDUX_SCREENER_BB_METHOD_HPP

#include <cstddef>

#include <pludux/asset_snapshot.hpp>
#include <pludux/screener/screener_method.hpp>
#include <pludux/series.hpp>

namespace pludux::screener {

class BbMethod {
public:
  enum class MaType { sma, ema, wma, rma, hma };

  BbMethod(OutputName output,
           MaType ma_type,
           ScreenerMethod input,
           std::size_t period,
           double stddev,
           std::size_t offset = 0);

  auto operator()(AssetSnapshot asset_data) const
   -> SubSeries<PolySeries<double>>;

  auto operator==(const BbMethod& other) const noexcept -> bool;

  auto offset() const noexcept -> std::size_t;

  void offset(std::size_t offset) noexcept;

  auto output() const noexcept -> OutputName;

  void output(OutputName output) noexcept;

  auto period() const noexcept -> std::size_t;

  void period(std::size_t period) noexcept;

  auto stddev() const noexcept -> double;

  void stddev(double stddev) noexcept;

  auto input() const noexcept -> ScreenerMethod;

  void input(ScreenerMethod input) noexcept;

  auto ma_type() const noexcept -> MaType;

  void ma_type(MaType ma_type) noexcept;

private:
  OutputName output_;
  MaType ma_type_;
  ScreenerMethod input_;
  std::size_t offset_;
  std::size_t period_;
  double stddev_;
};

} // namespace pludux::screener

#endif
