#ifndef PLUDUX_APPS_BACKTEST_APPLICATION_HPP
#define PLUDUX_APPS_BACKTEST_APPLICATION_HPP

#include <queue>

#include <imgui.h>

#include <pludux/asset_history.hpp>
#include <pludux/config_parser.hpp>
#include <pludux/screener.hpp>
#include <pludux/ta.hpp>

#include <pludux/backtest.hpp>

#include "./app_state.hpp"
#include "./windows.hpp"

namespace pludux::apps {

class Application {
public:
  Application();

  ~Application();

  void on_after_main_loop();

  void on_before_main_loop();

  void on_update();

  void set_window_size(int width, int height);

private:
  ImVec2 window_size_;

  DockspaceWindow dockspace_window_;
  PlotDataWindow plot_data_window_;
  BacktestsWindow backtests_window_;
  ProfilesWindow profiles_window_;

  AppStateData state_data_;
  std::queue<AppPolyAction> actions_;
};

} // namespace pludux::apps

#endif