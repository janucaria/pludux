module;

#include <algorithm>
#include <chrono>
#include <fstream>
#include <queue>
#include <ranges>
#include <stdexcept>
#include <string>

#include <imgui.h>
#include <implot.h>
#include <jsoncons/json.hpp>

export module pludux.apps.backtest;

export import :application_state;
export import :window_context;
export import :serialization;
export import :actions;
import :windows;

export namespace pludux::apps {

class Application {
public:
  Application()
  : window_size_{0, 0}
  {
  }

  void on_before_main_loop(this Application& self)
  {
    auto& app_state = self.app_state_;
    auto& actions = self.actions_;

    ImPlot::GetStyle().UseISO8601 = true;
    ImPlot::GetStyle().UseLocalTime = true;
    ImPlot::GetStyle().Use24HourClock = true;

    {
      const auto& markets = app_state.markets();
      if(markets.empty()) {
        auto default_market = std::make_shared<backtest::Market>("Default");
        app_state.add_market(std::move(default_market));
      }
    }

    {
      const auto& brokers = app_state.brokers();
      if(brokers.empty()) {
        auto default_broker = std::make_shared<backtest::Broker>("Default");
        app_state.add_broker(std::move(default_broker));
      }
    }

    {
      const auto& profiles = app_state.profiles();
      if(profiles.empty()) {
        auto default_profile = std::make_shared<backtest::Profile>("Default");
        default_profile->initial_capital(100'000'000.0);
        default_profile->capital_risk(0.01);
        app_state.add_profile(std::move(default_profile));
      }
    }

// run in debug mode and not in emscripten
#if !defined(__EMSCRIPTEN__) && !defined(NDEBUG) && 1

    const auto json_strategy_path =
     get_env_var("PLUDUX_BACKTEST_STRATEGY_JSON_PATH").value_or("");

    if(json_strategy_path.empty()) {
      return;
    }

    LoadStrategyJsonAction{json_strategy_path}(app_state);

    {
      const auto csv_path =
       get_env_var("PLUDUX_BACKTEST_CSV_DATA_PATH_1").value_or("");

      if(!csv_path.empty()) {
        LoadAssetCsvAction{csv_path}(app_state);
      }
    }

    {
      const auto csv_path =
       get_env_var("PLUDUX_BACKTEST_CSV_DATA_PATH_2").value_or("");

      if(!csv_path.empty()) {
        LoadAssetCsvAction{csv_path}(app_state);
      }
    }

#endif
  }

  void on_after_main_loop(this Application& self)
  {
  }

  void set_window_size(this Application& self, int width, int height)
  {
    self.window_size_ =
     ImVec2{static_cast<float>(width), static_cast<float>(height)};
  }

  void on_update(this Application& self)
  {
    auto& app_state = self.app_state_;
    auto& actions = self.actions_;

    if(!app_state.backtests().empty()) {
      auto& backtests = app_state.backtests();

      // create a loop with timeout 60 fps
      auto last_update_time = std::chrono::high_resolution_clock::now();
      auto current_time = std::chrono::high_resolution_clock::now();
      auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                        current_time - last_update_time)
                        .count();

      do {
        for(auto& backtest : backtests) {
          try {
            if(backtest->should_run()) {
              backtest->run();
            }
          } catch(const std::exception& e) {
            backtest->mark_as_failed();

            // TODO: this line is buggy in emscripten release build.
            // The bug is failed to load/open the strategy, asset, or pludux
            // file. No error message is shown in the GUI nor in the console. No
            // crash, just no error message.
            /*
            const auto error_message =
             std::format("Backtest '{}' failed: {}", backtest.name(), e.what());
            */

            const auto error_message =
             "Backtest '" + backtest->name() + "' failed: " + e.what();
            app_state.alert(error_message);
          }
        }

        current_time = std::chrono::high_resolution_clock::now();
        time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                     current_time - last_update_time)
                     .count();

      } while(time_diff < 1000 / 60);
    }

    {
      const auto alert_message = app_state.top_alert_message();

      if(alert_message) {
        ImGui::OpenPopup("Alerts");

        if(ImGui::BeginPopupModal(
            "Alerts", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
          ImGui::Text("%s", alert_message->c_str());

          if(ImGui::Button("OK")) {
            app_state.pop_alert_messages();
            ImGui::CloseCurrentPopup();
          }

          ImGui::EndPopup();
        }
      }
    }

    auto window_context = WindowContext{app_state, actions};

    try {
      self.dockspace_window_.render(window_context);
      self.plot_data_window_.render(window_context);

      auto backtesting_summary = BacktestSummaryWindow{};
      backtesting_summary.render(window_context);

      self.backtests_window_.render(window_context);

      self.assets_window_.render(window_context);

      self.strategies_window_.render(window_context);

      self.markets_window_.render(window_context);

      self.brokers_window_.render(window_context);

      self.profiles_window_.render(window_context);

      auto trade_journal = TradeJournalWindow{};
      trade_journal.render(window_context);

    } catch(const std::exception& e) {
      const auto error_message = std::format("Error: {}", e.what());
      app_state.alert(error_message);
    }

    while(!actions.empty()) {
      auto action = std::move(actions.front());
      actions.pop();

      try {
        action(app_state);
      } catch(const std::exception& e) {
        const auto error_message = std::format("Error: {}", e.what());
        app_state.alert(error_message);
      }
    }
  }

private:
  ImVec2 window_size_;

  DockspaceWindow dockspace_window_;
  PlotDataWindow plot_data_window_;
  BacktestsWindow backtests_window_;
  AssetsWindow assets_window_;
  StrategiesWindow strategies_window_;
  MarketsWindow markets_window_;
  BrokersWindow brokers_window_;
  ProfilesWindow profiles_window_;

  ApplicationState app_state_;
  std::queue<PolyAction> actions_;
};

} // namespace pludux::apps