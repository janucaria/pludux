#include <sstream>
#include <string>
#include <utility>

#include <pludux/backtest.hpp>

#include "../app_state_data.hpp"
#include "./change_strategy_json_str_action.hpp"

namespace pludux::apps {

ChangeStrategyJsonStrAction::ChangeStrategyJsonStrAction(std::string config_name,
                                                     std::string content)
: config_name_{std::move(config_name)}
, content_{std::move(content)}
{
}

void ChangeStrategyJsonStrAction::operator()(AppStateData& state) const
{
  auto json_strategy_str = std::stringstream{content_};
  auto backtest = parse_backtest_strategy_json(json_strategy_str);

  state.strategy_name = get_strategy_name();
  state.backtest = std::move(backtest);
  state.resource_changed = true;
}

auto ChangeStrategyJsonStrAction::get_strategy_name() const noexcept
 -> const std::string&
{
  return config_name_;
}

} // namespace pludux::apps

