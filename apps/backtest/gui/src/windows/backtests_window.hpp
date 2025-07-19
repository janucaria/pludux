#ifndef PLUDUX_APPS_BACKTEST_BACKTESTS_WINDOW_HPP
#define PLUDUX_APPS_BACKTEST_BACKTESTS_WINDOW_HPP

#include "../app_state.hpp"

namespace pludux::apps {

class BacktestsWindow {
public:
  BacktestsWindow();

  void render(AppState& app_state);

private:
  bool is_adding_new_backtest_{false};

  std::string new_backtest_name_{""};
  std::ptrdiff_t new_backtest_strategy_index_{0};
  std::ptrdiff_t new_backtest_asset_index_{0};
  std::ptrdiff_t new_backtest_profile_index_{0};

  void render_backtests_list(AppState& app_state);

  void render_add_new_backtest(AppState& app_state);
};

} // namespace pludux::apps

#endif
