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
  if(!json_strategy_stream.is_open()) {
    const auto error_message =
     std::format("Failed to open '{}' for reading.", path_);
    throw std::runtime_error(error_message);
  }

  ChangeStrategyJsonAction<ChangeStrategyJsonFileAction>::operator()(
   json_strategy_stream, state);
}

auto ChangeStrategyJsonFileAction::get_strategy_name() const noexcept
 -> std::string
{
  return std::filesystem::path{path_}.stem().string();
}

} // namespace pludux::apps
