module;

#include <memory>
#include <queue>
#include <vector>

#include <imgui.h>

export module pludux.apps.backtest:window_context;

import :actions;
import :application_state;

export namespace pludux::apps {

class WindowContext {
public:
  WindowContext(ApplicationState& app_state,
                std::queue<std::string>& alert_messages,
                std::queue<PolyAction>& actions)
  : app_state_{app_state}
  , alert_messages_{alert_messages}
  , actions_{actions}
  {
  }

  auto app_state(this const WindowContext& self) noexcept
   -> const ApplicationState&
  {
    return self.app_state_;
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

  void update_imgui_ini_settings(this WindowContext& self)
  {
    auto ini_size = std::size_t{0};
    const auto ini_data = ImGui::SaveIniSettingsToMemory(&ini_size);
    self.app_state_.imgui_ini_settings(std::string(ini_data, ini_size));
  }

  void alert(this WindowContext& self, std::string alert_message)
  {
    self.alert_messages_.push(std::move(alert_message));
  }

  auto is_backtest_ready(this const WindowContext& self,
                         const backtest::Backtest& ready_backtest) noexcept
   -> bool
  {
    {
      const auto asset_handle = ready_backtest.asset_handle();
      const auto asset_ptr = self.app_state_.get_asset_if_present(asset_handle);

      if(!asset_ptr) {
        return false;
      }
    }
    {
      const auto strategy_handle = ready_backtest.strategy_handle();
      const auto strategy_ptr =
       self.app_state_.get_strategy_if_present(strategy_handle);

      if(!strategy_ptr) {
        return false;
      }
    }
    {
      const auto broker_handle = ready_backtest.broker_handle();
      const auto broker_ptr =
       self.app_state_.get_broker_if_present(broker_handle);

      if(!broker_ptr) {
        return false;
      }
    }
    {
      const auto market_handle = ready_backtest.market_handle();
      const auto market_ptr =
       self.app_state_.get_market_if_present(market_handle);

      if(!market_ptr) {
        return false;
      }
    }
    {
      const auto profile_handle = ready_backtest.profile_handle();
      const auto profile_ptr =
       self.app_state_.get_profile_if_present(profile_handle);

      if(!profile_ptr) {
        return false;
      }
    }

    return true;
  }

private:
  ApplicationState& app_state_;
  std::queue<std::string>& alert_messages_;
  std::queue<PolyAction>& actions_;
};

} // namespace pludux::apps
