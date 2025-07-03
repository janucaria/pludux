#ifndef PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_BACKTESTS_SETUP_STRING_ACTION_HPP
#define PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_BACKTESTS_SETUP_STRING_ACTION_HPP

#include <filesystem>
#include <string>
#include <utility>

#include "../app_state_data.hpp"
#include "./load_backtests_setup_action.hpp"

namespace pludux::apps {

class LoadBacktestsSetupStingAction
: public details::LoadBacktestsSetupAction<LoadBacktestsSetupStingAction> {
public:
  LoadBacktestsSetupStingAction(std::string csv_string);

  void operator()(AppStateData& state) const;

private:
  std::string csv_string_;
};

} // namespace pludux::apps

#endif