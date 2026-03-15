module;

#include <vector>

export module pludux.backtest:plots.constant_plot_source_method;

import :plots.plot_method_contextable;

export namespace pludux::backtest {

class ConstantPlotSourceMethod {
public:
  ConstantPlotSourceMethod(double value = 0.0)
  : value_{value}
  {
  }

  auto operator==(const ConstantPlotSourceMethod&) const noexcept
   -> bool = default;

  // TODO: use std::generator<double> when it's available
  auto operator()(this const ConstantPlotSourceMethod& self,
                  PlotMethodContextable auto context)
   -> const std::vector<double>&
  {
    const auto size = context.results_size();
    if(self.cached_data_.size() != size) {
      self.cached_data_.assign(size, self.value_);
    }
    return self.cached_data_;
  }

  auto value(this const ConstantPlotSourceMethod& self) noexcept -> double
  {
    return self.value_;
  }

  void value(this ConstantPlotSourceMethod& self, double value) noexcept
  {
    self.value_ = value;
  }

private:
  double value_;

  // TODO: remove cached_data_ and generate the vector on the fly when
  // std::generator is available
  mutable std::vector<double> cached_data_;
};

} // namespace pludux::backtest
