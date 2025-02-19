#ifndef PLUDUX_APPS_BACKTEST_WINDOWS_BACKTEST_SUMMARY_WINDOW_HPP
#define PLUDUX_APPS_BACKTEST_WINDOWS_BACKTEST_SUMMARY_WINDOW_HPP

#include "../app_state.hpp"

namespace pludux::apps {

class BacktestSummaryWindow {
public:
  BacktestSummaryWindow();

  void render(AppState& app_state);

private:
};

} // namespace pludux::apps

#endif
