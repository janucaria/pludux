module;

#include <cstdint>
#include <utility>
#include <vector>

export module pludux.backtest:plots.line_plot_method;

import :plots.plot_method_contextable;
import :plots.any_plot_method_context;

export namespace pludux::backtest {

template<typename TPlotSourceMethod>
  requires requires(TPlotSourceMethod method, AnyPlotMethodContext context) {
    { method(context) };
  }
class LinePlotMethod {
public:
  LinePlotMethod(TPlotSourceMethod source)
  : LinePlotMethod(std::move(source), 0xFFFFFFFF)
  {
  }

  LinePlotMethod(TPlotSourceMethod source, std::uint32_t color)
  : source_{std::move(source)}
  , color_{color}
  {
  }

  auto operator==(const LinePlotMethod&) const noexcept -> bool = default;

  void operator()(this const LinePlotMethod& self,
                  PlotMethodContextable auto context)
  {
    const auto& data = self.source_(context);

    context.render_plot_line(data, self.color_);
  }

  auto source(this const LinePlotMethod& self) noexcept
   -> const TPlotSourceMethod&
  {
    return self.source_;
  }

  void source(this LinePlotMethod& self, TPlotSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto color(this const LinePlotMethod& self) noexcept -> std::uint32_t
  {
    return self.color_;
  }

  void color(this LinePlotMethod& self, std::uint32_t color) noexcept
  {
    self.color_ = color;
  }

private:
  TPlotSourceMethod source_;
  std::uint32_t color_;
};

} // namespace pludux::backtest
