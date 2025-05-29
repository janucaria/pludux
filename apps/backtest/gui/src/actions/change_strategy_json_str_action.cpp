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
  const auto strategy_name = get_strategy_name();
  auto parsed_strategy = parse_backtest_strategy_json(
   strategy_name, json_strategy_stream, state.quote_access);
  auto strategy_ptr =
   std::make_unique<backtest::Strategy>(std::move(parsed_strategy));
  const auto& strategy =
   *state.strategies.emplace_back(std::move(strategy_ptr));

  state.selected_strategy_index = state.strategies.size() - 1;

  state.backtests.clear();
  for(const auto& asset_ptr : state.assets) {
    const auto& asset = *asset_ptr;
    state.backtests.emplace_back(strategy, asset, state.quote_access);
  }
}

auto ChangeStrategyJsonStrAction::get_strategy_name() const noexcept
 -> const std::string&
{
  return config_name_;
}

} // namespace pludux::apps
