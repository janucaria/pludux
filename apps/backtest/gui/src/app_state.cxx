module;

#include <queue>
#include <vector>

#include "./app_state_data.hpp"

export module pludux.apps.backtest.app_state;

export import pludux.apps.backtest.actions;

export namespace pludux::apps {

class AppState {
public:
  AppState(AppStateData& state, std::queue<PolyAction>& actions)
  : state_data_{state}
  , actions_{actions}
  {
  }

  auto state() const noexcept -> const AppStateData&
  {
    return state_data_;
  }

  template<typename TAppAction, typename... Args>
  void emplace_action(Args&&... args)
  {
    actions_.emplace(TAppAction{std::forward<Args>(args)...});
  }

  template<typename TAppAction>
  void push_action(TAppAction action)
  {
    actions_.push(std::move(action));
  }

private:
  AppStateData& state_data_;
  std::queue<PolyAction>& actions_;
};

} // namespace pludux::apps
