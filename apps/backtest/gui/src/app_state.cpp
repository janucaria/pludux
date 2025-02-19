#include <queue>
#include <vector>

#include "./actions.hpp"
#include "./app_state_data.hpp"

#include "./app_state.hpp"

namespace pludux::apps {

AppState::AppState(AppStateData& state, std::queue<AppPolyAction>& actions)
: state_data_{state}
, actions_{actions}
{
}

auto AppState::state() const noexcept -> const AppStateData&
{
  return state_data_;
}

} // namespace pludux::apps
