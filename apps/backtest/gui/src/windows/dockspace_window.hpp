#ifndef PLUDUX_APPS_BACKTEST_WINDOWS_DOCKSPACE_WINDOW_HPP
#define PLUDUX_APPS_BACKTEST_WINDOWS_DOCKSPACE_WINDOW_HPP

#include "../app_state.hpp"

namespace pludux::apps {

class DockspaceWindow {
public:
  DockspaceWindow();

  void render(AppState& app_state);

private:
  bool open_about_popup_;
};

} // namespace pludux::apps

#endif
