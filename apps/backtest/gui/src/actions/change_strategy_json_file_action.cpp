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
  auto json_strategy_stream = std::ifstream{path_};
  const auto strategy_name = get_strategy_name();
  auto parsed_strategy = parse_backtest_strategy_json(
   strategy_name, json_strategy_stream, state.quote_access);
  auto strategy_ptr =
   std::make_shared<backtest::Strategy>(std::move(parsed_strategy));
  const auto& strategy =
   *state.strategies.emplace_back(std::move(strategy_ptr));
}

auto ChangeStrategyJsonFileAction::get_strategy_name() const noexcept
 -> std::string
{
  return std::filesystem::path{path_}.filename().string();
}

} // namespace pludux::apps
