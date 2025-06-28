#ifndef PLUDUX_APPS_BACKTEST_ACTIONS_CHANGE_CONFIG_JSON_STR_ACTION_HPP
#define PLUDUX_APPS_BACKTEST_ACTIONS_CHANGE_CONFIG_JSON_STR_ACTION_HPP

#include <string>

#include "../app_state_data.hpp"
#include "./change_strategy_json_action.hpp"

namespace pludux::apps {

class ChangeStrategyJsonStrAction
: public ChangeStrategyJsonAction<ChangeStrategyJsonStrAction> {
public:
  ChangeStrategyJsonStrAction(std::string config_name, std::string content);

  void operator()(AppStateData& state) const;

  auto get_strategy_name() const noexcept -> std::string;

private:
  std::string config_name_;
  std::string content_;
};

} // namespace pludux::apps

#endif
