module;

#include <any>
#include <functional>
#include <string>
#include <utility>

export module pludux.backtest:plots.any_plot_method;

import :plots.plot_method_contextable;
import :plots.any_plot_method_context;

export namespace pludux::backtest {

class AnyPlotMethod {
public:
  template<typename TPlotMethod>
    requires requires(TPlotMethod method, AnyPlotMethodContext context) {
      { method(context) };
    }
  AnyPlotMethod(TPlotMethod plot_method)
  : impl_{std::move(plot_method)}
  , plot_func_{[](const std::any& impl, AnyPlotMethodContext context) {
    auto* method = std::any_cast<TPlotMethod>(&impl);
    if(method) {
      (*method)(context);
    }
  }}
  , equals_{[](const std::any& impl, const AnyPlotMethod& other) {
    if(auto other_method = std::any_cast<TPlotMethod>(&other.impl_)) {
      const auto& method = std::any_cast<TPlotMethod>(impl);
      return method == *other_method;
    }
    return false;
  }}
  , not_equals_{[](const std::any& impl, const AnyPlotMethod& other) {
    if(auto other_method = std::any_cast<TPlotMethod>(&other.impl_)) {
      const auto& method = std::any_cast<TPlotMethod>(impl);
      return method != *other_method;
    }
    return true;
  }}
  {
  }

  auto operator==(this const AnyPlotMethod& self,
                  const AnyPlotMethod& other) noexcept -> bool
  {
    return self.equals_(self.impl_, other);
  }

  auto operator!=(this const AnyPlotMethod& self,
                  const AnyPlotMethod& other) noexcept -> bool
  {
    return self.not_equals_(self.impl_, other);
  }

  void operator()(this const AnyPlotMethod& self,
                  PlotMethodContextable auto context)
  {
    self.plot_func_(self.impl_, context);
  }

  template<typename TPlotMethod>
  friend auto plot_method_cast(const AnyPlotMethod& self) -> const TPlotMethod*
  {
    return std::any_cast<TPlotMethod>(&self.impl_);
  }

  template<typename TPlotMethod>
  friend auto plot_method_cast(AnyPlotMethod& self) -> TPlotMethod*
  {
    return std::any_cast<TPlotMethod>(&self.impl_);
  }

private:
  std::any impl_;

  std::function<void(const std::any&, AnyPlotMethodContext context)> plot_func_;

  std::function<bool(const std::any&, const AnyPlotMethod&)> equals_;

  std::function<bool(const std::any&, const AnyPlotMethod&)> not_equals_;
};

} // namespace pludux::backtest
