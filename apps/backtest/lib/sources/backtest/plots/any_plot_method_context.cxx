module;

#include <any>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:plots.any_plot_method_context;

import :plots.plot_method_contextable;

export namespace pludux::backtest {

class AnyPlotMethodContext {
public:
  template<typename TPlotMethodContext>
    requires PlotMethodContextable<TPlotMethodContext>
  AnyPlotMethodContext(TPlotMethodContext plot_method_context)
  : impl_{std::move(plot_method_context)}
  , render_plot_line_{[](const std::any& impl,
                         const std::vector<double>& data,
                         std::uint32_t color) {
    std::any_cast<TPlotMethodContext>(impl).render_plot_line(data, color);
  }}
  , render_plot_histogram_{[](const std::any& impl,
                              const std::vector<double>& data,
                              std::uint32_t color) {
    std::any_cast<TPlotMethodContext>(impl).render_plot_histogram(data, color);
  }}
  , render_plot_momentum_histogram_{[](
                                     const std::any& impl,
                                     const std::vector<double>& data,
                                     const std::vector<std::uint32_t>& colors) {
    std::any_cast<TPlotMethodContext>(impl).render_plot_momentum_histogram(
     data, colors);
  }}
  , series_results_{[](const std::any& impl, const std::string& series_name)
                     -> std::optional<
                      std::reference_wrapper<const std::vector<double>>> {
    auto* context = std::any_cast<TPlotMethodContext>(&impl);
    if(context) {
      return context->series_results(series_name);
    }
    return std::nullopt;
  }}
  , results_size_{[](const std::any& impl) {
    auto* context = std::any_cast<TPlotMethodContext>(&impl);
    if(context) {
      return context->results_size();
    }
    return std::size_t{0};
  }}
  {
  }

  void render_plot_line(this const AnyPlotMethodContext& self,
                        const std::vector<double>& data,
                        std::uint32_t color)
  {
    self.render_plot_line_(self.impl_, data, color);
  }

  void render_plot_histogram(this const AnyPlotMethodContext& self,
                             const std::vector<double>& data,
                             std::uint32_t color)
  {
    self.render_plot_histogram_(self.impl_, data, color);
  }

  void render_plot_momentum_histogram(this const AnyPlotMethodContext& self,
                                      const std::vector<double>& data,
                                      const std::vector<std::uint32_t>& colors)
  {
    self.render_plot_momentum_histogram_(self.impl_, data, colors);
  }

  auto series_results(this const AnyPlotMethodContext& self,
                      const std::string& series_name)
   -> std::optional<std::reference_wrapper<const std::vector<double>>>
  {
    return self.series_results_(self.impl_, series_name);
  }

  auto results_size(this const AnyPlotMethodContext& self) -> std::size_t
  {
    return self.results_size_(self.impl_);
  }

  template<typename TPlotMethodContext>
    requires PlotMethodContextable<TPlotMethodContext>
  friend auto plot_method_context_cast(const AnyPlotMethodContext& self)
   -> const TPlotMethodContext*
  {
    return std::any_cast<TPlotMethodContext>(&self.impl_);
  }

  template<typename TPlotMethodContext>
    requires PlotMethodContextable<TPlotMethodContext>
  friend auto plot_method_context_cast(AnyPlotMethodContext& self)
   -> TPlotMethodContext*
  {
    return std::any_cast<TPlotMethodContext>(&self.impl_);
  }

private:
  std::any impl_;

  std::function<void(
   const std::any&, const std::vector<double>&, std::uint32_t)>
   render_plot_line_;

  std::function<void(
   const std::any&, const std::vector<double>&, std::uint32_t)>
   render_plot_histogram_;

  std::function<void(const std::any&,
                     const std::vector<double>&,
                     const std::vector<std::uint32_t>&)>
   render_plot_momentum_histogram_;

  std::function<
   auto(const std::any&, const std::string&)
    ->std::optional<std::reference_wrapper<const std::vector<double>>>>
   series_results_;

  std::function<auto(const std::any&)->std::size_t> results_size_;
};

static_assert(PlotMethodContextable<AnyPlotMethodContext>);

} // namespace pludux::backtest
