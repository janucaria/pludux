#include <sstream>
#include <string>
#include <utility>

#include "../app_state_data.hpp"
#include "./load_backtests_setup_string_action.hpp"

namespace pludux::apps {

LoadBacktestsSetupStingAction::LoadBacktestsSetupStingAction(
 std::string csv_string)
: csv_string_(std::move(csv_string))
{
}

void LoadBacktestsSetupStingAction::operator()(AppStateData& state) const
{
  auto csv_stream = std::istringstream{csv_string_};
  if(!csv_stream) {
    throw std::runtime_error("Failed to create input stream from CSV string.");
  }

  details::LoadBacktestsSetupAction<LoadBacktestsSetupStingAction>::operator()(
   csv_stream, state);
}

} // namespace pludux::apps
