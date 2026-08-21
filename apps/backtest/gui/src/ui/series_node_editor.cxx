module;

#include <string>
#include <utility>
#include <vector>

export module pludux.apps.backtest:series_node_editor;

import pludux.backtest;
import :window_context;
import :windows.models_window;

export namespace pludux::apps::ui {

enum class SeriesNodeCatalog { Full, PortfolioComparator };

class SeriesNodeEditor {
public:
  explicit SeriesNodeEditor(
   SeriesNodeCatalog catalog = SeriesNodeCatalog::Full,
   std::vector<std::string> available_series_names = {})
  : catalog_{catalog}
  , available_series_names_{std::move(available_series_names)}
  {
  }

  void available_series_names(this SeriesNodeEditor& self,
                              std::vector<std::string> value)
  {
    self.available_series_names_ = std::move(value);
  }

  void render(this SeriesNodeEditor& self,
              backtest::ComparatorNode& node,
              WindowContext& context)
  {
    self.renderer_.render_portfolio_comparator_expression(node, context);
  }

private:
  SeriesNodeCatalog catalog_;
  std::vector<std::string> available_series_names_;
  ModelsWindow renderer_;
};

} // namespace pludux::apps::ui
