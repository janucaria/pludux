module;

#include <cstdint>
#include <utility>
#include <vector>

export module pludux.backtest:plots.histogram_plot_method;

import :plots.plot_method_contextable;
import :plots.any_plot_method_context;

export namespace pludux::backtest {

template<typename TPlotSourceMethod>
  requires requires(TPlotSourceMethod method, AnyPlotMethodContext context) {
    { method(context) };
  }
class HistogramPlotMethod {
public:
  HistogramPlotMethod(TPlotSourceMethod source)
  : HistogramPlotMethod(std::move(source), 0xFFFFFFFF)
  {
  }

  HistogramPlotMethod(TPlotSourceMethod source, std::uint32_t color)
  : source_{std::move(source)}
  , color_{color}
  {
  }

  auto operator==(const HistogramPlotMethod&) const noexcept -> bool = default;

  void operator()(this const HistogramPlotMethod& self,
                  PlotMethodContextable auto context)
  {
    const auto& data = self.source_(context);

    context.render_plot_histogram(data, self.color_);
  }

  auto source(this const HistogramPlotMethod& self) noexcept
   -> const TPlotSourceMethod&
  {
    return self.source_;
  }

  void source(this HistogramPlotMethod& self, TPlotSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto color(this const HistogramPlotMethod& self) noexcept -> std::uint32_t
  {
    return self.color_;
  }

  void color(this HistogramPlotMethod& self, std::uint32_t color) noexcept
  {
    self.color_ = color;
  }

private:
  TPlotSourceMethod source_;
  std::uint32_t color_;
};

} // namespace pludux::backtest
