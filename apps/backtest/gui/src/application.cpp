#include <algorithm>
#include <chrono>
#include <ranges>
#include <string>

#include <imgui.h>
#include <implot.h>

#include <nlohmann/json.hpp>

#include <pludux/backtest.hpp>

#include "./application.hpp"

namespace pludux::apps {

Application::Application()
: window_size_{0, 0}
{
}

Application::~Application()
{
}

void Application::on_before_main_loop()
{
  ImPlot::GetStyle().UseISO8601 = true;
  ImPlot::GetStyle().UseLocalTime = true;
  ImPlot::GetStyle().Use24HourClock = true;

  auto date_method = screener::DataMethod{"Date"};
  auto open_method = screener::DataMethod{"Open"};
  auto high_method = screener::DataMethod{"High"};
  auto low_method = screener::DataMethod{"Low"};
  auto close_method = screener::DataMethod{"Close"};
  auto volume_method = screener::DataMethod{"Volume"};

  state_data_.quote_access = QuoteAccess{std::move(date_method),
                                         std::move(open_method),
                                         std::move(high_method),
                                         std::move(low_method),
                                         std::move(close_method),
                                         std::move(volume_method)};

// run in debug mode and not in emscripten
#if !defined(__EMSCRIPTEN__) && !defined(NDEBUG) && 1

  const auto json_strategy_path =
   get_env_var("PLUDUX_BACKTEST_STRATEGY_JSON_PATH").value_or("");

  if(json_strategy_path.empty()) {
    return;
  }

  ChangeStrategyJsonFileAction{json_strategy_path}(state_data_);

  {
    const auto csv_path =
     get_env_var("PLUDUX_BACKTEST_CSV_DATA_PATH_1").value_or("");

    if(!csv_path.empty()) {
      LoadAssetCsvFileAction{csv_path}(state_data_);
    }
  }

  {
    const auto csv_path =
     get_env_var("PLUDUX_BACKTEST_CSV_DATA_PATH_2").value_or("");

    if(!csv_path.empty()) {
      LoadAssetCsvFileAction{csv_path}(state_data_);
    }
  }

#endif
}

void Application::on_after_main_loop()
{
}

void Application::set_window_size(int width, int height)
{
  window_size_ = ImVec2{static_cast<float>(width), static_cast<float>(height)};
}

void Application::on_update()
{
  if(!state_data_.backtests.empty()) {
    auto& backtests = state_data_.backtests;

    // create a loop with timeout 60 fps
    auto last_update_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                      current_time - last_update_time)
                      .count();

    try {
      do {
        for(auto& backtest : backtests) {
          if(backtest.should_run()) {
            backtest.run();
          }
        }

        current_time = std::chrono::high_resolution_clock::now();
        time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                     current_time - last_update_time)
                     .count();

      } while(time_diff < 1000 / 60);

    } catch(const std::exception& e) {
      state_data_.backtests.clear();

      const auto error_message = std::format("Error: {}", e.what());
      state_data_.alert_messages.push(error_message);
    }
  }

  auto app_state = AppState{state_data_, actions_};

  {
    if(!state_data_.alert_messages.empty()) {
      ImGui::OpenPopup("Alerts");

      if(ImGui::BeginPopupModal(
          "Alerts", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        const auto alert_message = state_data_.alert_messages.front();
        ImGui::Text("%s", alert_message.c_str());

        if(ImGui::Button("OK", ImVec2(120, 0))) {
          state_data_.alert_messages.pop();
          ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
      }
    }
  }

  try {
    dockspace_window_.render(app_state);
    plot_data_window_.render(app_state);

    auto backtesting_summary = BacktestSummaryWindow{};
    backtesting_summary.render(app_state);

    backtests_window_.render(app_state);

    auto assets_window = AssetsWindow{};
    assets_window.render(app_state);

    auto strategies_window = StrategiesWindow{};
    strategies_window.render(app_state);

    auto trade_journal = TradeJournalWindow{};
    trade_journal.render(app_state);

  } catch(const std::exception& e) {
    const auto error_message = std::format("Error: {}", e.what());
    state_data_.alert_messages.push(error_message);
  }

  while(!actions_.empty()) {
    const auto action = std::move(actions_.front());
    actions_.pop();

    try {
      action(state_data_);
    } catch(const std::exception& e) {
      const auto error_message = std::format("Error: {}", e.what());
      state_data_.alert_messages.push(error_message);
    }
  }
}

} // namespace pludux::apps