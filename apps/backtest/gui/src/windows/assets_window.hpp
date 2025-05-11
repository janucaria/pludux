#ifndef PLUDUX_APPS_BACKTEST_ASSETS_WINDOW_HPP
#define PLUDUX_APPS_BACKTEST_ASSETS_WINDOW_HPP

#include "../app_state.hpp"

namespace pludux::apps {

class AssetsWindow {
public:
  AssetsWindow();

  void render(AppState& app_state);
};

} // namespace pludux::apps

#endif
