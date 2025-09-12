#ifndef PLUDUX_APPS_BACKTEST_STRATEGIES_WINDOW_HPP
#define PLUDUX_APPS_BACKTEST_STRATEGIES_WINDOW_HPP

#include <imgui.h>
#include <jsoncons/json.hpp>

#include "../app_state.hpp"

namespace pludux::apps {

class StrategiesWindow {
public:
  StrategiesWindow();

  void render(AppState& app_state);

private:
  enum class Page { List, AddNew } current_page_{Page::AddNew};

  jsoncons::ojson data_json;

  void render_strategies_list(AppState& app_state);

  void render_add_new_strategy(AppState& app_state);
};

} // namespace pludux::apps

#endif
