module;

#include <any>
#include <concepts>
#include <functional>
#include <string>
#include <utility>

export module pludux.backtest:plots.erased_plot_method;

import :plots.plot_method_contextable;

export namespace pludux::backtest {

template<PlotMethodContextable TContext>
class ErasedPlotMethod {
public:
  template<typename TPlotMethod>
    requires std::equality_comparable<TPlotMethod> &&
              requires(TPlotMethod method, TContext context) {
                { method(context) };
              }
  ErasedPlotMethod(TPlotMethod plot_method)
  : impl_{std::move(plot_method)}
  , plot_func_{[](const std::any& impl, TContext context) {
    auto* method = std::any_cast<TPlotMethod>(&impl);
    if(method) {
      (*method)(context);
    }
  }}
  , equals_{[](const std::any& impl, const ErasedPlotMethod& other) {
    if(auto other_method = std::any_cast<TPlotMethod>(&other.impl_)) {
      const auto& method = std::any_cast<TPlotMethod>(impl);
      return method == *other_method;
    }
    return false;
  }}
  , not_equals_{[](const std::any& impl, const ErasedPlotMethod& other) {
    if(auto other_method = std::any_cast<TPlotMethod>(&other.impl_)) {
      const auto& method = std::any_cast<TPlotMethod>(impl);
      return method != *other_method;
    }
    return true;
  }}
  {
  }

  auto operator==(this const ErasedPlotMethod& self,
                  const ErasedPlotMethod& other) noexcept -> bool
  {
    return self.equals_(self.impl_, other);
  }

  auto operator!=(this const ErasedPlotMethod& self,
                  const ErasedPlotMethod& other) noexcept -> bool
  {
    return self.not_equals_(self.impl_, other);
  }

  void operator()(this const ErasedPlotMethod& self, TContext context)
  {
    self.plot_func_(self.impl_, context);
  }

  template<typename TPlotMethod>
  friend auto plot_method_cast(const ErasedPlotMethod& self)
   -> const TPlotMethod*
  {
    return std::any_cast<TPlotMethod>(&self.impl_);
  }

  template<typename TPlotMethod>
  friend auto plot_method_cast(ErasedPlotMethod& self) -> TPlotMethod*
  {
    return std::any_cast<TPlotMethod>(&self.impl_);
  }

private:
  std::any impl_;

  std::function<void(const std::any&, TContext context)> plot_func_;

  std::function<bool(const std::any&, const ErasedPlotMethod&)> equals_;

  std::function<bool(const std::any&, const ErasedPlotMethod&)> not_equals_;
};

} // namespace pludux::backtest
