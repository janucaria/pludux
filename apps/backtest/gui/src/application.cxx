module;

#include <algorithm>
#include <chrono>
#include <queue>
#include <ranges>
#include <string>

#include <imgui.h>
#include <implot.h>
#include <nlohmann/json.hpp>

#include <pludux/asset_history.hpp>
#include <pludux/backtest.hpp>
#include <pludux/config_parser.hpp>
#include <pludux/screener.hpp>
#include <pludux/ta.hpp>

#include "./app_state.hpp"
#include "./windows.hpp"

export module pludux.apps.backtest.application;

namespace pludux::apps {

export class Application {
public:
  Application()
  : window_size_{0, 0}
  {
  }

  void on_before_main_loop(this auto& self)
  {
    auto& state_data = self.state_data_;
    auto& actions = self.actions_;

    ImPlot::GetStyle().UseISO8601 = true;
    ImPlot::GetStyle().UseLocalTime = true;
    ImPlot::GetStyle().Use24HourClock = true;

    {
      auto& profiles = state_data.profiles;
      if(profiles.empty()) {
        auto default_profile = std::make_shared<backtest::Profile>("Default");
        default_profile->initial_capital(100'000'000.0);
        default_profile->capital_risk(0.01);
        profiles.push_back(default_profile);
      }
    }

// run in debug mode and not in emscripten
#if !defined(__EMSCRIPTEN__) && !defined(NDEBUG) && 1

    const auto json_strategy_path =
     get_env_var("PLUDUX_BACKTEST_STRATEGY_JSON_PATH").value_or("");

    if(json_strategy_path.empty()) {
      return;
    }

    ChangeStrategyJsonFileAction{json_strategy_path}(state_data);

    {
      const auto csv_path =
       get_env_var("PLUDUX_BACKTEST_CSV_DATA_PATH_1").value_or("");

      if(!csv_path.empty()) {
        LoadAssetCsvFileAction{csv_path}(state_data);
      }
    }

    {
      const auto csv_path =
       get_env_var("PLUDUX_BACKTEST_CSV_DATA_PATH_2").value_or("");

      if(!csv_path.empty()) {
        LoadAssetCsvFileAction{csv_path}(state_data);
      }
    }

#endif
  }

  void on_after_main_loop(this auto& self)
  {
  }

  void set_window_size(this auto& self, int width, int height)
  {
    self.window_size_ =
     ImVec2{static_cast<float>(width), static_cast<float>(height)};
  }

  void on_update(this auto& self)
  {
    auto& state_data = self.state_data_;
    auto& actions = self.actions_;

    if(!state_data.backtests.empty()) {
      auto& backtests = state_data.backtests;

      // create a loop with timeout 60 fps
      auto last_update_time = std::chrono::high_resolution_clock::now();
      auto current_time = std::chrono::high_resolution_clock::now();
      auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                        current_time - last_update_time)
                        .count();

      do {
        for(auto& backtest : backtests) {
          try {
            if(backtest.should_run()) {
              backtest.run();
            }
          } catch(const std::exception& e) {
            backtest.mark_as_failed();

            // TODO: this line is buggy in emscripten release build.
            // The bug is failed to load/open the strategy, asset, or pludux
            // file. No error message is shown in the GUI nor in the console. No
            // crash, just no error message.
            /*
            const auto error_message =
             std::format("Backtest '{}' failed: {}", backtest.name(), e.what());
            */

            const auto error_message =
             "Backtest '" + backtest.name() + "' failed: " + e.what();
            state_data.alert_messages.push(error_message);
          }
        }

        current_time = std::chrono::high_resolution_clock::now();
        time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                     current_time - last_update_time)
                     .count();

      } while(time_diff < 1000 / 60);
    }

    auto app_state = AppState{state_data, actions};

    {
      if(!state_data.alert_messages.empty()) {
        ImGui::OpenPopup("Alerts");

        if(ImGui::BeginPopupModal(
            "Alerts", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
          const auto alert_message = state_data.alert_messages.front();
          ImGui::Text("%s", alert_message.c_str());

          if(ImGui::Button("OK", ImVec2(120, 0))) {
            state_data.alert_messages.pop();
            ImGui::CloseCurrentPopup();
          }

          ImGui::EndPopup();
        }
      }
    }

    try {
      self.dockspace_window_.render(app_state);
      self.plot_data_window_.render(app_state);

      auto backtesting_summary = BacktestSummaryWindow{};
      backtesting_summary.render(app_state);

      self.backtests_window_.render(app_state);

      auto assets_window = AssetsWindow{};
      assets_window.render(app_state);

      auto strategies_window = StrategiesWindow{};
      strategies_window.render(app_state);

      self.profiles_window_.render(app_state);

      auto trade_journal = TradeJournalWindow{};
      trade_journal.render(app_state);

    } catch(const std::exception& e) {
      const auto error_message = std::format("Error: {}", e.what());
      state_data.alert_messages.push(error_message);
    }

    while(!actions.empty()) {
      const auto action = std::move(actions.front());
      actions.pop();

      try {
        action(state_data);
      } catch(const std::exception& e) {
        // some error like failed in deserialization can cause addition of
        // nullptr assets, so we need to remove them from the assets vector
        state_data.assets.erase(
         std::remove_if(state_data.assets.begin(),
                        state_data.assets.end(),
                        [](const auto& asset) { return !asset; }),
         state_data.assets.end());

        const auto error_message = std::format("Error: {}", e.what());
        state_data.alert_messages.push(error_message);
      }
    }
  }

private:
  ImVec2 window_size_;

  DockspaceWindow dockspace_window_;
  PlotDataWindow plot_data_window_;
  BacktestsWindow backtests_window_;
  ProfilesWindow profiles_window_;

  AppStateData state_data_;
  std::queue<AppPolyAction> actions_;
};

} // namespace pludux::apps