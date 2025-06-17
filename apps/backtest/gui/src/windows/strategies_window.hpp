#ifndef PLUDUX_APPS_BACKTEST_STRATEGIES_WINDOW_HPP
#define PLUDUX_APPS_BACKTEST_STRATEGIES_WINDOW_HPP

#include "../app_state.hpp"

namespace pludux::apps {

class StrategiesWindow {
public:
  StrategiesWindow();

  void render(AppState& app_state);
};

} // namespace pludux::apps

#endif
