#ifndef PLUDUX_APPS_BACKTEST_ACTIONS_CHANGE_CONFIG_JSON_FILE_ACTION_HPP
#define PLUDUX_APPS_BACKTEST_ACTIONS_CHANGE_CONFIG_JSON_FILE_ACTION_HPP

#include <string>

#include "../app_state_data.hpp"

namespace pludux::apps {

class ChangeStrategyJsonFileAction {
public:
  ChangeStrategyJsonFileAction(std::string path);

  void operator()(AppStateData& state) const;

  auto get_strategy_name() const noexcept -> std::string;

private:
  std::string path_;
};

} // namespace pludux::apps

#endif
