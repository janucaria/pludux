#include <chrono>
#include <ctime>
#include <format>
#include <ranges>
#include <string>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include "../app_state.hpp"
#include "./trade_journal_window.hpp"

namespace pludux::apps {

TradeJournalWindow::TradeJournalWindow()
{
}

void TradeJournalWindow::render(AppState& app_state)
{
  const auto& state = app_state.state();
  const auto& backtests = state.backtests;

  ImGui::Begin("Trades", nullptr);

  if(!backtests.empty()) {
    const auto backtest = backtests[state.selected_backtest_index];

    const auto table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                             ImGuiTableFlags_Resizable |
                             ImGuiTableFlags_Reorderable |
                             ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY;

    constexpr auto headers = std::array{"Trade #",
                                        "Entry Date",
                                        "Exit Date",
                                        "Position Value (Size)",
                                        "Entry Price",
                                        "Stop Loss",
                                        "Take Profit",
                                        "Status",
                                        "Exit Price",
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
        const auto& summary = backtest_summaries[i];
        const auto& trade_record = summary.trade_record();

        if(trade_record.is_summary_session()) {
          auto trade_count = summary.trade_count();
          if(trade_record.is_open()) {
            trade_count++;
          }

          draw_trade_row(trade_count, trade_record);
        }
      }

      ImGui::EndTable();
    }
  }

  ImGui::End();
}

void TradeJournalWindow::draw_trade_row(int trade_count,
                                        const backtest::TradeRecord& trade)
{
  ImGui::PushID(trade_count);

  ImGui::TableNextRow();
  ImGui::TableNextColumn();
  ImGui::Selectable(std::to_string(trade_count).c_str(),
                    false,
                    ImGuiSelectableFlags_SpanAllColumns |
                     ImGuiSelectableFlags_AllowOverlap);
  ImGui::TableNextColumn();
  ImGui::Text("%s", format_datetime(trade.entry_timestamp()).c_str());
  ImGui::TableNextColumn();
  ImGui::Text(
   "%s",
   trade.is_open() ? "N/A" : format_datetime(trade.exit_timestamp()).c_str());
  ImGui::TableNextColumn();
  const auto position_value = trade.position_value();
  ImGui::Text(
   "%s (%.0f)", format_currency(position_value).c_str(), trade.position_size());
  ImGui::TableNextColumn();
  ImGui::Text("%s", format_currency(trade.entry_price()).c_str());
  ImGui::TableNextColumn();
  ImGui::Text("%s", format_currency(trade.stop_loss_price()).c_str());
  ImGui::TableNextColumn();
  ImGui::Text("%s", format_currency(trade.take_profit_price()).c_str());
  ImGui::TableNextColumn();
  ImGui::Text("%s", format_trade_status(trade.status()).c_str());
  ImGui::TableNextColumn();
  if(trade.is_open()) {
    ImGui::Text("N/A");
  } else {
    ImGui::Text("%s", format_currency(trade.exit_price()).c_str());
  }
  ImGui::TableNextColumn();

  ImGui::Text("%s (%.2f%%)",
              format_currency(trade.pnl()).c_str(),
              trade.pnl() / trade.position_value() * 100.0);
  ImGui::TableNextColumn();
  ImGui::Text("%s", format_duration(trade.duration()).c_str());

  ImGui::PopID();
}

auto TradeJournalWindow::format_trade_status(
 backtest::TradeRecord::Status status) noexcept -> std::string
{
  switch(status) {
  case backtest::TradeRecord::Status::flat:
    return "Flat";
  case backtest::TradeRecord::Status::open:
    return "Open";
  case backtest::TradeRecord::Status::closed_exit_signal:
    return "Exit Signal";
  case backtest::TradeRecord::Status::closed_take_profit:
    return "Take Profit";
  case backtest::TradeRecord::Status::closed_stop_loss:
    return "Stop Loss";
  }

  return "Unknown";
}

} // namespace pludux::apps
