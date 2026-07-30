module;

#include <any>
#include <concepts>
#include <functional>
#include <utility>
#include <vector>

export module pludux.backtest:plots.erased_plot_source_method;

import :plots.plot_method_contextable;

export namespace pludux::backtest {

template<PlotMethodContextable TContext>
class ErasedPlotSourceMethod {
public:
  template<typename TPlotSourceMethod>
    requires std::equality_comparable<TPlotSourceMethod> &&
              requires(TPlotSourceMethod method, TContext context) {
                { method(context) } -> std::same_as<const std::vector<double>&>;
              }
  ErasedPlotSourceMethod(TPlotSourceMethod plot_source_method)
  : impl_{std::move(plot_source_method)}
  , func_{[](const std::any& impl,
             TContext context) -> const std::vector<double>& {
    static const auto empty_result = std::vector<double>{};

    auto* method = std::any_cast<TPlotSourceMethod>(&impl);
    if(method) {
      return (*method)(context);
    }
    return empty_result;
  }}
  , equals_{[](const std::any& impl, const ErasedPlotSourceMethod& other) {
    if(auto other_method = std::any_cast<TPlotSourceMethod>(&other.impl_)) {
      const auto& method = std::any_cast<TPlotSourceMethod>(impl);
      return method == *other_method;
    }
    return false;
  }}
  , not_equals_{[](const std::any& impl, const ErasedPlotSourceMethod& other) {
    if(auto other_method = std::any_cast<TPlotSourceMethod>(&other.impl_)) {
      const auto& method = std::any_cast<TPlotSourceMethod>(impl);
      return method != *other_method;
    }
    return true;
  }}
  {
  }

  auto operator==(this const ErasedPlotSourceMethod& self,
                  const ErasedPlotSourceMethod& other) noexcept -> bool
  {
    return self.equals_(self.impl_, other);
  }

  auto operator!=(this const ErasedPlotSourceMethod& self,
                  const ErasedPlotSourceMethod& other) noexcept -> bool
  {
    return self.not_equals_(self.impl_, other);
  }

  auto operator()(this const ErasedPlotSourceMethod& self, TContext context)
   -> const std::vector<double>&
  {
    return self.func_(self.impl_, std::forward<decltype(context)>(context));
  }

  template<typename TPlotSourceMethod>
  friend auto plot_source_method_cast(const ErasedPlotSourceMethod& self)
   -> const TPlotSourceMethod*
  {
    return std::any_cast<TPlotSourceMethod>(&self.impl_);
  }

  template<typename TPlotSourceMethod>
  friend auto plot_source_method_cast(ErasedPlotSourceMethod& self)
   -> TPlotSourceMethod*
  {
    return std::any_cast<TPlotSourceMethod>(&self.impl_);
  }

private:
  std::any impl_;

  std::function<auto(const std::any&, TContext)->const std::vector<double>&>
   func_;

  std::function<auto(const std::any&, const ErasedPlotSourceMethod&)->bool>
   equals_;

  std::function<auto(const std::any&, const ErasedPlotSourceMethod&)->bool>
   not_equals_;
};

} // namespace pludux::backtest
