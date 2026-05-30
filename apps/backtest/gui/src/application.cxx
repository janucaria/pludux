module;

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <list>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <imgui.h>
#include <implot.h>
#include <jsoncons/json.hpp>

export module pludux.apps.backtest;

export import :application_state;
export import :state_diff;
export import :window_context;
export import :serialization;
export import :actions;
export import :command_executor;
import :windows;

export namespace std {

template<>
struct hash<pludux::backtest::BacktestStoreHandle> {
  auto
  operator()(const pludux::backtest::BacktestStoreHandle& handle) const noexcept
   -> size_t
  {
    const auto slot_hash = std::hash<std::size_t>{}(handle.slot_index());
    const auto generation_hash = std::hash<std::size_t>{}(handle.generation());

    return slot_hash ^ (generation_hash << 1);
  }
};

} // namespace std

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

    ImPlot::GetStyle().UseISO8601 = true;
    ImPlot::GetStyle().UseLocalTime = true;
    ImPlot::GetStyle().Use24HourClock = true;

    if(app_state.get_market_handles().empty()) {
      auto default_market = backtest::Market{"Default"};
      app_state.add_market(std::move(default_market));
    }

    if(app_state.get_broker_handles().empty()) {
      auto default_broker = backtest::Broker{"Default"};
      app_state.add_broker(std::move(default_broker));
    }

    if(app_state.get_profile_handles().empty()) {
      auto default_profile = backtest::Profile{"Default"};
      default_profile.capital_risk(0.01);
      app_state.add_profile(std::move(default_profile));
    }

    {
      const auto json_ma_cross_sample =
       std::string{PLUDUX_SAMPLES_STRATEGY_moving_avg_cross};
      LoadStrategyJsonAction{std::string{"MA Cross"},
                             json_ma_cross_sample}(app_state);
    }
    {
      const auto json_ma_2_lines_cross_sample =
       std::string{PLUDUX_SAMPLES_STRATEGY_moving_avg_2_line_cross};
      LoadStrategyJsonAction{std::string{"MA 2 Lines Cross"},
                             json_ma_2_lines_cross_sample}(app_state);
    }
    {
      const auto json_bollinger_bands_sample =
       std::string{PLUDUX_SAMPLES_STRATEGY_bollinger_bands_L};
      LoadStrategyJsonAction{std::string{"Bollinger Bands"},
                             json_bollinger_bands_sample}(app_state);
    }
    {
      const auto json_rsi_sample = std::string{PLUDUX_SAMPLES_STRATEGY_rsi};
      LoadStrategyJsonAction{std::string{"RSI"}, json_rsi_sample}(app_state);
    }
    {
      const auto json_macd_sample = std::string{PLUDUX_SAMPLES_STRATEGY_macd};
      LoadStrategyJsonAction{std::string{"MACD"}, json_macd_sample}(app_state);
    }
    {
      const auto json_keltner_channels_sample =
       std::string{PLUDUX_SAMPLES_STRATEGY_keltner_channels};
      LoadStrategyJsonAction{std::string{"Keltner Channels"},
                             json_keltner_channels_sample}(app_state);
    }
    {
      const auto json_stochastic_sample =
       std::string{PLUDUX_SAMPLES_STRATEGY_stochastic};
      LoadStrategyJsonAction{std::string{"Stochastic"},
                             json_stochastic_sample}(app_state);
    }
    {
      const auto json_stochastic_rsi_sample =
       std::string{PLUDUX_SAMPLES_STRATEGY_stochastic_rsi};
      LoadStrategyJsonAction{std::string{"Stochastic RSI"},
                             json_stochastic_rsi_sample}(app_state);
    }

// run in debug mode and not in emscripten
#if !defined(__EMSCRIPTEN__) && !defined(NDEBUG) && 1

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

    const auto& backtest_handles = app_state.get_backtest_handles();
    for(const auto& backtest_handle : backtest_handles) {
      self.recreate_backtest_runner(app_state, backtest_handle);
    }
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
    auto& store = app_state.store();
    auto& alert_messages = self.alert_messages_;

    auto& backtest_handles = app_state.get_backtest_handles();

    // sync backtest runners with backtest handles in the app state, and run the
    // backtests
    for(auto it = self.running_backtests_.begin();
        it != self.running_backtests_.end();) {
      const auto& backtest_handle = it->first;

      if(std::ranges::find(backtest_handles, backtest_handle) ==
         backtest_handles.end()) {
        it = self.running_backtests_.erase(it);
      } else {
        ++it;
      }
    }

    if(!backtest_handles.empty()) {
      // create a loop with timeout 60 fps
      auto last_update_time = std::chrono::high_resolution_clock::now();
      auto current_time = std::chrono::high_resolution_clock::now();
      auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                        current_time - last_update_time)
                        .count();

      do {
        for(auto&& backtest_handle : backtest_handles) {
          auto& backtest = app_state.get_backtest(backtest_handle);

          if(!self.running_backtests_.contains(backtest_handle)) {
            continue;
          }

          auto& summaries =
           store.get_or_create_backtest_summaries(backtest_handle);
          auto& series_evaluation_results =
           store.get_or_create_series_results(backtest_handle);

          auto& backtest_runner = self.running_backtests_.at(backtest_handle);

          try {
            backtest_runner.run(series_evaluation_results, summaries);
          } catch(const std::exception& e) {
            backtest_runner.is_failed(true);

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
            alert_messages.push_back(error_message);
          }
        }

        current_time = std::chrono::high_resolution_clock::now();
        time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                     current_time - last_update_time)
                     .count();
      } while(time_diff < 1000 / 60);
    }

    auto window_context =
     WindowContext{app_state, alert_messages, self.command_executor_};

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
      alert_messages.push_back(error_message);
    }

    try {
      if(self.command_executor_.execute(app_state)) {
        // check if the backtest has reset, if so, remove the backtest runner
        // from the running backtests
        for(auto&& backtest_handle : backtest_handles) {
          const auto reset_needed =
           self.should_reset_backtest_runner(app_state, backtest_handle);

          if(reset_needed) {
            // backtest has reset, replace the backtest runner with a new one to
            // reset its state
            self.running_backtests_.erase(backtest_handle);
            self.recreate_backtest_runner(app_state, backtest_handle);
          }
        }
      }
    } catch(const std::exception& e) {
      const auto error_message = std::format("Error: {}", e.what());
      alert_messages.push_back(error_message);
    }

    if(!alert_messages.empty()) {
      const auto& alert_message = alert_messages.front();

      ImGui::OpenPopup("Alerts");

      if(ImGui::BeginPopupModal(
          "Alerts", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", alert_message.c_str());

        if(ImGui::Button("OK")) {
          alert_messages.pop_front();
          ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
      }
    }
  }

private:
  auto
  should_reset_backtest_runner(this const Application& self,
                               const ApplicationState& app_state,
                               backtest::BacktestStoreHandle backtest_handle)
   -> bool
  {
    const auto* backtest = app_state.get_backtest_if_present(backtest_handle);
    if(!backtest) {
      return true;
    }

    const auto& store = app_state.store();

    const auto* summaries =
     store.get_backtest_summaries_if_present(backtest_handle);
    if(!summaries) {
      return true;
    }

    const auto* series_evaluation_results =
     store.get_series_results_if_present(backtest_handle);
    if(!series_evaluation_results) {
      return true;
    }

    if(summaries->empty()) {
      return true;
    }

    if(series_evaluation_results->empty()) {
      return true;
    }

    return false;
  }

  void recreate_backtest_runner(this Application& self,
                                ApplicationState& app_state,
                                backtest::BacktestStoreHandle backtest_handle)
  {
    auto& backtest = app_state.get_backtest(backtest_handle);
    const auto* asset_ptr =
     app_state.get_asset_if_present(backtest.asset_handle());
    const auto* strategy_ptr =
     app_state.get_strategy_if_present(backtest.strategy_handle());
    const auto* market_ptr =
     app_state.get_market_if_present(backtest.market_handle());
    const auto* broker_ptr =
     app_state.get_broker_if_present(backtest.broker_handle());
    const auto* profile_ptr =
     app_state.get_profile_if_present(backtest.profile_handle());

    if(!asset_ptr || !strategy_ptr || !market_ptr || !broker_ptr ||
       !profile_ptr) {
      return;
    }

    self.running_backtests_.emplace(
     backtest_handle,
     backtest::BacktestRunner{*asset_ptr,
                              *strategy_ptr,
                              *market_ptr,
                              *broker_ptr,
                              *profile_ptr,
                              backtest.initial_capital()});
  }

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
  std::unordered_map<backtest::BacktestStoreHandle, backtest::BacktestRunner>
   running_backtests_;
  std::list<std::string> alert_messages_;
  CommandExecutor command_executor_{};
};

} // namespace pludux::apps