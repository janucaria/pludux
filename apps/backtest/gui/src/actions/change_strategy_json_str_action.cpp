#include <filesystem>
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
  ChangeStrategyJsonAction<ChangeStrategyJsonStrAction>::operator()(
   json_strategy_stream, state);
}

auto ChangeStrategyJsonStrAction::get_strategy_name() const noexcept
 -> std::string
{
  return std::filesystem::path{config_name_}.stem().string();
}

} // namespace pludux::apps
