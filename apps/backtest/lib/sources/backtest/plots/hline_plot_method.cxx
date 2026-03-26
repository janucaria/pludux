module;

#include <cstdint>
#include <utility>
#include <vector>

export module pludux.backtest:plots.hline_plot_method;

import :plots.plot_method_contextable;

export namespace pludux::backtest {

class HLinePlotMethod {
public:
  HLinePlotMethod(double level)
  : HLinePlotMethod(level, 0xFFFFFFFF)
  {
  }

  HLinePlotMethod(double level, std::uint32_t color)
  : level_{level}
  , color_{color}
  {
  }

  auto operator==(const HLinePlotMethod&) const noexcept -> bool = default;

  void operator()(this const HLinePlotMethod& self,
                  PlotMethodContextable auto context)
  {
    const auto data = std::vector<double>(context.results_size(), self.level_);
    context.render_plot_line(data, self.color_);
  }

  auto level(this const HLinePlotMethod& self) noexcept -> double
  {
    return self.level_;
  }

  void level(this HLinePlotMethod& self, double level) noexcept
  {
    self.level_ = level;
  }

  auto color(this const HLinePlotMethod& self) noexcept -> std::uint32_t
  {
    return self.color_;
  }

  void color(this HLinePlotMethod& self, std::uint32_t color) noexcept
  {
    self.color_ = color;
  }

private:
  double level_;
  std::uint32_t color_;
};

} // namespace pludux::backtest
