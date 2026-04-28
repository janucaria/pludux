module;

#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:plot_group;

import :plots;

export namespace pludux::backtest {

class PlotGroup {
public:
  PlotGroup()
  : PlotGroup{"", true, {}}
  {
  }

  PlotGroup(std::string name)
  : PlotGroup{std::move(name), true, {}}
  {
  }

  PlotGroup(std::string name, bool is_overlay, std::vector<AnyPlotMethod> items)
  : name_{std::move(name)}
  , is_overlay_{is_overlay}
  , items_{std::move(items)}
  {
  }

  auto operator==(const PlotGroup&) const noexcept -> bool = default;

  auto name(this const PlotGroup& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this PlotGroup& self, std::string name) noexcept
  {
    self.name_ = std::move(name);
  }

  auto is_overlay(this const PlotGroup& self) noexcept -> bool
  {
    return self.is_overlay_;
  }

  void is_overlay(this PlotGroup& self, bool is_overlay) noexcept
  {
    self.is_overlay_ = is_overlay;
  }

  auto items(this const PlotGroup& self) noexcept
   -> const std::vector<AnyPlotMethod>&
  {
    return self.items_;
  }

  void items(this PlotGroup& self, std::vector<AnyPlotMethod> items) noexcept
  {
    self.items_ = std::move(items);
  }

private:
  std::string name_;

  bool is_overlay_;

  std::vector<AnyPlotMethod> items_;
};

} // namespace pludux::backtest
