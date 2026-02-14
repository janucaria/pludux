module;

#include <memory>
#include <queue>
#include <vector>

export module pludux.apps.backtest:window_context;

import :actions;
import :application_state;

export namespace pludux::apps {

class WindowContext {
public:
  WindowContext(ApplicationState& app_state, std::queue<PolyAction>& actions)
  : app_state_{app_state}
  , actions_{actions}
  {
  }

  auto app_state(this const WindowContext& self) noexcept
   -> const ApplicationState&
  {
    return self.app_state_;
  }

  auto backtests(this const WindowContext& self) noexcept
   -> const std::vector<std::shared_ptr<backtest::Backtest>>&
  {
    return self.app_state_.backtests();
  }

  auto assets(this const WindowContext& self) noexcept
   -> const std::vector<std::shared_ptr<backtest::Asset>>&
  {
    return self.app_state_.assets();
  }

  auto strategies(this const WindowContext& self) noexcept
   -> const std::vector<std::shared_ptr<backtest::Strategy>>&
  {
    return self.app_state_.strategies();
  }

  auto markets(this const WindowContext& self) noexcept
   -> const std::vector<std::shared_ptr<backtest::Market>>&
  {
    return self.app_state_.markets();
  }

  auto brokers(this const WindowContext& self) noexcept
   -> const std::vector<std::shared_ptr<backtest::Broker>>&
  {
    return self.app_state_.brokers();
  }

  auto profiles(this const WindowContext& self) noexcept
   -> const std::vector<std::shared_ptr<backtest::Profile>>&
  {
    return self.app_state_.profiles();
  }

  template<typename TAppAction, typename... Args>
  void emplace_action(this WindowContext& self, Args&&... args)
  {
    self.actions_.emplace(TAppAction{std::forward<Args>(args)...});
  }

  template<typename TAppAction>
  void push_action(this WindowContext& self, TAppAction action)
  {
    self.actions_.push(std::move(action));
  }

  // TODO: compiler bug in emscripten build
  // void push_alert_action(this WindowContext& self, std::string alert_message)
  // {
  //   self.push_action(
  //    [alert_message = std::move(alert_message)](ApplicationState& app_state)
  //    {
  //      app_state.alert(alert_message);
  //    });
  // }

private:
  ApplicationState& app_state_;
  std::queue<PolyAction>& actions_;
};

} // namespace pludux::apps
