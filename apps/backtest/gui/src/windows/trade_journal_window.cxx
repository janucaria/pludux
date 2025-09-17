module;

#include <chrono>
#include <ctime>
#include <format>
#include <ranges>
#include <string>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include "../app_state_data.hpp"

export module pludux.apps.backtest.windows:trade_journal_window;

import pludux.apps.backtest.app_state;

export namespace pludux::apps {

class TradeJournalWindow {
public:
  void render(this const auto, AppState& app_state)
  {
    const auto& state = app_state.state();
    const auto& backtests = state.backtests;

    ImGui::Begin("Trades", nullptr);

    if(!backtests.empty()) {
      const auto backtest = backtests[state.selected_backtest_index];

      const auto table_flags =
       ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
       ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
       ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY;

      constexpr auto headers = std::array{"Trade #",
                                          "Type",
                                          "Status",
                                          "Entry Date",
                                          "Exit Date",
                                          "Entry Price",
                                          "Exit Price",
                                          "Take Profit",
                                          "Stop Loss",
                                          "Avg Price",
                                          "Investment (Size)",
                                          "P&L",
                                          "Duration"};

      if(ImGui::BeginTable("TradesTable", headers.size(), table_flags)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        for(const auto& header : headers) {
          if(header == std::string{"Trade #"}) {
            const auto text_size = ImGui::CalcTextSize(header);
            ImGui::TableSetupColumn(
             header, ImGuiTableColumnFlags_WidthFixed, text_size.x);
          } else {
            ImGui::TableSetupColumn(header);
          }
        }

        ImGui::TableHeadersRow();

        const auto& backtest_summaries = backtest.summaries();
        const auto backtest_summaries_size = backtest_summaries.size();
        for(int i = backtest_summaries_size - 1; i >= 0; --i) {
          const auto& summary = backtest_summaries.at(i);
          const auto& trade_session = summary.trade_session();
          const auto asset_index = backtest_summaries_size - i - 1;

          auto id_counter = 0;
          for(const auto& trade_record : trade_session.trade_record_range()) {
            if(!trade_session.is_open() || trade_record.exit_index() == 0) {
              const auto trade_count =
               summary.trade_count() + (trade_session.is_open() ? 1 : 0);

              ImGui::PushID(id_counter++);
              draw_trade_row(trade_count, trade_record);
              ImGui::PopID();
            }
          }
        }

        ImGui::EndTable();
      }
    }

    ImGui::End();
  }

private:
  static void draw_trade_row(int trade_count,
                             const backtest::TradeRecord& trade)
  {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Selectable(std::to_string(trade_count).c_str(),
                      false,
                      ImGuiSelectableFlags_SpanAllColumns |
                       ImGuiSelectableFlags_AllowOverlap);
    ImGui::TableNextColumn();
    ImGui::Text("%s", trade.is_long_position() ? "Long" : "Short");
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_trade_status(trade.status()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_datetime(trade.entry_timestamp()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text(
     "%s",
     trade.is_open() ? "N/A" : format_datetime(trade.exit_timestamp()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.entry_price()).c_str());
    ImGui::TableNextColumn();
    if(trade.is_open()) {
      ImGui::Text("N/A");
    } else {
      ImGui::Text("%s", format_currency(trade.exit_price()).c_str());
    }
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.take_profit_price()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.stop_loss_price()).c_str());
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_currency(trade.average_price()).c_str());
    ImGui::TableNextColumn();
    const auto investment = trade.investment();
    ImGui::Text(
     "%s (%.0f)", format_currency(investment).c_str(), trade.position_size());
    ImGui::TableNextColumn();

    ImGui::Text("%s (%.2f%%)",
                format_currency(trade.pnl()).c_str(),
                trade.pnl() / trade.investment() * 100.0);
    ImGui::TableNextColumn();
    ImGui::Text("%s", format_duration(trade.duration()).c_str());
  }

  static auto format_trade_status(backtest::TradeRecord::Status status) noexcept
   -> std::string
  {
    switch(status) {
    case backtest::TradeRecord::Status::open:
      return "Open";
    case backtest::TradeRecord::Status::closed_exit_signal:
      return "Exit Signal";
    case backtest::TradeRecord::Status::closed_take_profit:
      return "Take Profit";
    case backtest::TradeRecord::Status::closed_stop_loss:
      return "Stop Loss";
    case backtest::TradeRecord::Status::scaled_in:
      return "Scaled In";
    case backtest::TradeRecord::Status::scaled_out:
      return "Scaled Out";
    }

    [[unlikely]] {
      return "Unknown";
    }
  }
};

} // namespace pludux::apps
