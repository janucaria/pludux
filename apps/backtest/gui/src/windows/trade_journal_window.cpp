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
                             ImGuiTableFlags_Hideable;

    constexpr auto headers = std::array{"Entry Date",
                                        "Exit Date",
                                        "Position Size",
                                        "Entry Price",
                                        "Stop Loss",
                                        "Take Profit",
                                        "Status",
                                        "Exit Price",
                                        "Profit",
                                        "Duration"};

    if(ImGui::BeginTable("TradesTable", headers.size(), table_flags)) {
      for(const auto& header : headers) {
        ImGui::TableSetupColumn(header);
      }

      ImGui::TableHeadersRow();

      const auto& trade_records = backtest.trade_records();
      const auto total_trades = trade_records.size();
      for(int i = total_trades - 1; i >= 0; --i) {
        const auto& trade = trade_records[i];

        if(trade.is_summary_session()) {
          draw_trade_row(trade);
        }
      }

      ImGui::EndTable();
    }
  }

  ImGui::End();
}

void TradeJournalWindow::draw_trade_row(const backtest::TradeRecord& trade)
{
  const auto entry_timestamp = trade.entry_timestamp();
  char entry_date[32];
  ImPlot::FormatDate(ImPlotTime::FromDouble(entry_timestamp),
                     entry_date,
                     32,
                     ImPlotDateFmt_DayMoYr,
                     ImPlot::GetStyle().UseISO8601);

  const auto exit_timestamp = trade.exit_timestamp();
  char exit_date[32];
  ImPlot::FormatDate(ImPlotTime::FromDouble(exit_timestamp),
                     exit_date,
                     32,
                     ImPlotDateFmt_DayMoYr,
                     ImPlot::GetStyle().UseISO8601);

  const auto duration = trade.duration();

  ImGui::TableNextRow();
  ImGui::TableNextColumn();
  ImGui::Text("%s", entry_date);
  ImGui::TableNextColumn();
  ImGui::Text("%s", exit_date);
  ImGui::TableNextColumn();
  ImGui::Text("%.0f", trade.position_size());
  ImGui::TableNextColumn();
  ImGui::Text("%.0f", trade.entry_price());
  ImGui::TableNextColumn();
  ImGui::Text("%.0f", trade.stop_loss_price());
  ImGui::TableNextColumn();
  ImGui::Text("%.0f", trade.take_profit_price());
  ImGui::TableNextColumn();
  ImGui::Text("%s", format_trade_status(trade.status()).c_str());
  ImGui::TableNextColumn();
  if(trade.is_open()) {
    ImGui::Text("N/A");
  } else {
    ImGui::Text("%.0f", trade.exit_price());
  }
  ImGui::TableNextColumn();

  ImGui::Text("%.2f%s", trade.profit(), trade.is_open() ? " (Floating)" : "");
  ImGui::TableNextColumn();
  ImGui::Text("%s", format_duration(trade.duration()).c_str());
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

auto TradeJournalWindow::format_duration(std::time_t time_duration) noexcept
 -> std::string
{
  // get the duration in days
  const auto days = time_duration / (60 * 60 * 24);

  // if the duration is greater than 1 month then return the duration in
  // months and days. if the duration is greater than 1 year then return the
  // duration in years, months and days.
  if(days > 30) {
    const auto months = days / 30;
    const auto remaining_days = days % 30;
    if(months > 12) {
      const auto years = months / 12;
      const auto remaining_months = months % 12;
      return std::format(
       "{} years, {} months, {} days", years, remaining_months, remaining_days);
    }
    return std::format("{} months, {} days", months, remaining_days);
  }

  return std::format("{} days", days);
}

} // namespace pludux::apps
