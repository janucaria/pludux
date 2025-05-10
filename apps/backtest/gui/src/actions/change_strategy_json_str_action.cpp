#include <sstream>
#include <string>
#include <utility>

#include <pludux/backtest.hpp>

#include "../app_state_data.hpp"
#include "./change_strategy_json_str_action.hpp"

namespace pludux::apps {

ChangeStrategyJsonStrAction::ChangeStrategyJsonStrAction(
 std::string config_name, std::string content)
: config_name_{std::move(config_name)}
, content_{std::move(content)}
{
}

void ChangeStrategyJsonStrAction::operator()(AppStateData& state) const
{
  auto json_strategy_stream = std::stringstream{content_};
  auto strategy = parse_backtest_strategy_json(json_strategy_stream);

  state.strategy_name = get_strategy_name();
  state.strategy = strategy;

  state.backtests.clear();
  for(const auto& asset : state.assets) {
    state.backtests.emplace_back(state.strategy.value(), asset);
  }
}

auto ChangeStrategyJsonStrAction::get_strategy_name() const noexcept
 -> const std::string&
{
  return config_name_;
}

} // namespace pludux::apps
