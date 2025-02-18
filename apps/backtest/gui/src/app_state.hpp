#ifndef PLUDUX_APPS_BACKTEST_APP_STATE_HPP
#define PLUDUX_APPS_BACKTEST_APP_STATE_HPP

#include <queue>
#include <vector>

#include "./actions.hpp"
#include "./app_state_data.hpp"

namespace pludux::apps {

class AppState {
public:
  AppState(AppStateData& state, std::queue<AppPolyAction>& actions);

  auto state() const noexcept -> const AppStateData&;

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
  std::queue<AppPolyAction>& actions_;
};

} // namespace pludux::apps

#endif
