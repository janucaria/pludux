module;

#include <functional>
#include <list>
#include <memory>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include <imgui.h>

export module pludux.apps.backtest:window_context;

import :application_state;
import :actions;
import :backtest_execution_status;
import :command_executor;

export namespace pludux::apps {

class WindowContext {
public:
  WindowContext(
   ApplicationState& app_state,
   std::list<std::string>& alert_messages,
   CommandExecutor& command_executor,
   bool& discard_all_drafts_requested,
   std::function<const BacktestExecutionStatus*(
    const backtest::PortfolioStoreHandle&)> backtest_execution_status_lookup)
  : app_state_{app_state}
  , alert_messages_{alert_messages}
  , command_executor_{command_executor}
  , discard_all_drafts_requested_{discard_all_drafts_requested}
  , backtest_execution_status_lookup_{
     std::move(backtest_execution_status_lookup)}
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
    self.command_executor_.push(TAppAction{std::forward<Args>(args)...});
  }

  template<typename TAppAction>
  void push_action(this WindowContext& self, TAppAction action)
  {
    self.command_executor_.push(std::move(action));
  }

  void update_imgui_ini_settings(this WindowContext& self)
  {
    auto ini_size = std::size_t{0};
    const auto ini_data = ImGui::SaveIniSettingsToMemory(&ini_size);
    self.app_state_.imgui_ini_settings(std::string(ini_data, ini_size));
  }

  void request_discard_all_drafts(this WindowContext& self) noexcept
  {
    self.discard_all_drafts_requested_ = true;
  }

  void alert(this WindowContext& self, std::string alert_message)
  {
    self.alert_messages_.push_back(std::move(alert_message));
  }

  void push_undo(this WindowContext& self)
  {
    self.command_executor_.push(UndoCommand{});
  }

  auto has_undo(this const WindowContext& self) -> bool
  {
    return self.command_executor_.can_undo();
  }

  void push_redo(this WindowContext& self)
  {
    self.command_executor_.push(RedoCommand{});
  }

  auto has_redo(this const WindowContext& self) -> bool
  {
    return self.command_executor_.can_redo();
  }

  auto backtest_execution_status(
   this const WindowContext& self,
   const backtest::PortfolioStoreHandle& handle) noexcept
   -> const BacktestExecutionStatus*
  {
    return self.backtest_execution_status_lookup_(handle);
  }

private:
  ApplicationState& app_state_;
  std::list<std::string>& alert_messages_;
  CommandExecutor& command_executor_;
  bool& discard_all_drafts_requested_;
  std::function<const BacktestExecutionStatus*(
   const backtest::PortfolioStoreHandle&)>
   backtest_execution_status_lookup_;
};

} // namespace pludux::apps
