module;

#include <string>
#include <utility>
#include <vector>

export module pludux.apps.backtest:series_node_editor;

import pludux.backtest;
import :window_context;
import :windows.strategies_window;

export namespace pludux::apps::ui {

enum class SeriesNodeCatalog { Full, RequestedOrder };

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
              ErasedNode<ErasedSeriesMethodContext>& node,
              WindowContext& context)
  {
    if(self.catalog_ == SeriesNodeCatalog::RequestedOrder) {
      self.renderer_.render_requested_order_expression(node, context);
    } else {
      self.renderer_.render_numeric_expression(
       node, context, self.available_series_names_, true);
    }
  }

private:
  SeriesNodeCatalog catalog_;
  std::vector<std::string> available_series_names_;
  StrategiesWindow renderer_;
};

} // namespace pludux::apps::ui
