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
    auto* context = std::any_cast<TPlotMethodContext>(&impl);
    if(context) {
      context->render_plot_line(data, color);
    }
    // TODO: handle error case (e.g., log an error message)
  }}
  , render_plot_histogram_{[](const std::any& impl,
                              const std::vector<double>& data,
                              std::uint32_t color) {
    auto* context = std::any_cast<TPlotMethodContext>(&impl);
    if(context) {
      context->render_plot_histogram(data, color);
    }
    // TODO: handle error case (e.g., log an error message)
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

  std::function<
   auto(const std::any&, const std::string&)
    ->std::optional<std::reference_wrapper<const std::vector<double>>>>
   series_results_;

  std::function<auto(const std::any&)->std::size_t> results_size_;
};

static_assert(PlotMethodContextable<AnyPlotMethodContext>);

} // namespace pludux::backtest
