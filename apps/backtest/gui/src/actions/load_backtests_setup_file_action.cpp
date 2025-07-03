#include <filesystem>
#include <fstream>
#include <utility>

#include "../app_state_data.hpp"
#include "./load_backtests_setup_file_action.hpp"

namespace pludux::apps {

LoadBacktestsSetupFileAction::LoadBacktestsSetupFileAction(
 std::filesystem::path file_path)
: file_path_(std::move(file_path))
{
}

void LoadBacktestsSetupFileAction::operator()(AppStateData& state) const
{
  auto csv_stream = std::ifstream{file_path_};
  if(!csv_stream.is_open()) {
    throw std::runtime_error("Failed to open CSV file: " + file_path_.string());
  }

  details::LoadBacktestsSetupAction<LoadBacktestsSetupFileAction>::operator()(
   csv_stream, state);
}

} // namespace pludux::apps