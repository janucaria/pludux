#include <filesystem>
#include <fstream>
#include <string>
#include <utility>

#include <pludux/backtest.hpp>

#include "../app_state_data.hpp"
#include "./change_strategy_json_file_action.hpp"

namespace pludux::apps {

ChangeStrategyJsonFileAction::ChangeStrategyJsonFileAction(std::string path)
: path_{std::move(path)}
{
}

void ChangeStrategyJsonFileAction::operator()(AppStateData& state) const
{
  auto json_strategy_file = std::ifstream{path_};
  auto backtest = parse_backtest_strategy_json(json_strategy_file);

  state.strategy_name = get_strategy_name();
  state.backtest = backtest;
}

auto ChangeStrategyJsonFileAction::get_strategy_name() const noexcept
 -> std::string
{
  return std::filesystem::path{path_}.filename().string();
}

} // namespace pludux::apps
