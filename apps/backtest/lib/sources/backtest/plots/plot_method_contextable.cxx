module;

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

export module pludux.backtest:plots.plot_method_contextable;

export namespace pludux::backtest {

template<typename TContext>
concept PlotMethodContextable = requires(TContext context,
                                         std::vector<double> data,
                                         std::string series_name,
                                         std::uint32_t color) {
  context.render_plot_line(data, color);

  {
    context.series_results(series_name)
  } -> std::same_as<std::optional<std::reference_wrapper<const std::vector<double>>>>;

  { context.results_size() } -> std::convertible_to<std::size_t>;
};

} // namespace pludux::backtest
