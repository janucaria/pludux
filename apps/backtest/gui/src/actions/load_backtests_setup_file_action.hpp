#ifndef PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_BACKTESTS_SETUP_FILE_ACTION_HPP
#define PLUDUX_APPS_BACKTEST_ACTIONS_LOAD_BACKTESTS_SETUP_FILE_ACTION_HPP

#include <filesystem>
#include <string>
#include <utility>

#include "../app_state_data.hpp"
#include "./load_backtests_setup_action.hpp"

namespace pludux::apps {

class LoadBacktestsSetupFileAction
: public details::LoadBacktestsSetupAction<LoadBacktestsSetupFileAction> {
public:
  LoadBacktestsSetupFileAction(std::filesystem::path file_path);

  void operator()(AppStateData& state) const;

private:
  std::filesystem::path file_path_;
};

} // namespace pludux::apps

#endif