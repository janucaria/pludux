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

private:
  ApplicationState& app_state_;
  std::queue<std::string>& alert_messages_;
  std::queue<PolyAction>& actions_;
};

} // namespace pludux::apps
