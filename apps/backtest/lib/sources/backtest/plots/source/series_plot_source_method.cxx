module;

#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:plots.series_plot_source_method;

import :plots.plot_method_contextable;

export namespace pludux::backtest {

class SeriesPlotSourceMethod {
public:
  SeriesPlotSourceMethod(std::string series_name = "")
  : series_name_{std::move(series_name)}
  {
  }

  auto operator==(const SeriesPlotSourceMethod&) const noexcept
   -> bool = default;

  auto operator()(this const SeriesPlotSourceMethod& self,
                  PlotMethodContextable auto context)
   -> const std::vector<double>&
  {
    static const auto empty_result = std::vector<double>{};
    const auto series_results_opt = context.series_results(self.series_name_);
    return series_results_opt ? series_results_opt->get() : empty_result;
  }

  auto series_name(this const SeriesPlotSourceMethod& self) noexcept
   -> const std::string&
  {
    return self.series_name_;
  }

  void series_name(this SeriesPlotSourceMethod& self,
                   std::string series_name) noexcept
  {
    self.series_name_ = std::move(series_name);
  }

private:
  std::string series_name_;
};

} // namespace pludux::backtest
