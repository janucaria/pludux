module;

#include <any>
#include <functional>
#include <vector>

export module pludux.backtest:plots.any_plot_source_method;

import :plots.plot_method_contextable;
import :plots.any_plot_method_context;

export namespace pludux::backtest {

class AnyPlotSourceContext {
public:
private:
};

class AnyPlotSourceMethod {
public:
  template<typename TPlotSourceMethod>
  AnyPlotSourceMethod(TPlotSourceMethod plot_source_method)
  : impl_{std::move(plot_source_method)}
  , func_{[](const std::any& impl,
             AnyPlotMethodContext context) -> const std::vector<double>& {
    static const auto empty_result = std::vector<double>{};

    auto* method = std::any_cast<TPlotSourceMethod>(&impl);
    if(method) {
      return (*method)(context);
    }
    return empty_result;
  }}
  , equals_{[](const std::any& impl, const AnyPlotSourceMethod& other) {
    if(auto other_method = std::any_cast<TPlotSourceMethod>(&other.impl_)) {
      const auto& method = std::any_cast<TPlotSourceMethod>(impl);
      return method == *other_method;
    }
    return false;
  }}
  , not_equals_{[](const std::any& impl, const AnyPlotSourceMethod& other) {
    if(auto other_method = std::any_cast<TPlotSourceMethod>(&other.impl_)) {
      const auto& method = std::any_cast<TPlotSourceMethod>(impl);
      return method != *other_method;
    }
    return true;
  }}
  {
  }

  auto operator==(this const AnyPlotSourceMethod& self,
                  const AnyPlotSourceMethod& other) noexcept -> bool
  {
    return self.equals_(self.impl_, other);
  }

  auto operator!=(this const AnyPlotSourceMethod& self,
                  const AnyPlotSourceMethod& other) noexcept -> bool
  {
    return self.not_equals_(self.impl_, other);
  }

  auto operator()(this const AnyPlotSourceMethod& self,
                  PlotMethodContextable auto context)
   -> const std::vector<double>&
  {
    return self.func_(self.impl_, std::forward<decltype(context)>(context));
  }

  template<typename TPlotSourceMethod>
  friend auto plot_source_method_cast(const AnyPlotSourceMethod& self)
   -> const TPlotSourceMethod*
  {
    return std::any_cast<TPlotSourceMethod>(&self.impl_);
  }

  template<typename TPlotSourceMethod>
  friend auto plot_source_method_cast(AnyPlotSourceMethod& self)
   -> TPlotSourceMethod*
  {
    return std::any_cast<TPlotSourceMethod>(&self.impl_);
  }

private:
  std::any impl_;

  std::function<
   auto(const std::any&, AnyPlotMethodContext)->const std::vector<double>&>
   func_;

  std::function<auto(const std::any&, const AnyPlotSourceMethod&)->bool>
   equals_;

  std::function<auto(const std::any&, const AnyPlotSourceMethod&)->bool>
   not_equals_;
};

} // namespace pludux::backtest
