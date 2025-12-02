module;

#include <queue>
#include <vector>

export module pludux.apps.backtest:app_state;

import :actions;
import :app_state_data;

export namespace pludux::apps {

class AppState {
public:
  AppState(AppStateData& state, std::queue<PolyAction>& actions)
  : state_data_{state}
  , actions_{actions}
  {
  }

  auto state(this const AppState& self) noexcept -> const AppStateData&
  {
    return self.state_data_;
  }

  template<typename TAppAction, typename... Args>
  void emplace_action(this AppState& self, Args&&... args)
  {
    self.actions_.emplace(TAppAction{std::forward<Args>(args)...});
  }

  template<typename TAppAction>
  void push_action(this AppState& self, TAppAction action)
  {
    self.actions_.push(std::move(action));
  }

private:
  AppStateData& state_data_;
  std::queue<PolyAction>& actions_;
};

} // namespace pludux::apps
