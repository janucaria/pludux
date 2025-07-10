#include <chrono>
#include <format>
#include <iostream>
#include <sstream>

#include <imgui.h>

#include <pludux/backtest/backtesting_summary.hpp>

#include "../app_state.hpp"

#include "./backtesting_summary_window.hpp"

namespace pludux::apps {

BacktestSummaryWindow::BacktestSummaryWindow()
{
}

void BacktestSummaryWindow::render(AppState& app_state)
{
  auto& state = app_state.state();
  auto& backtests = state.backtests;

  ImGui::Begin("Summary", nullptr);

  if(!state.backtests.empty() && state.selected_backtest_index >= 0) {
    const auto& backtest = backtests[state.selected_backtest_index];
    const auto& backtest_name = backtest.name();
    ImGui::Text("%s", backtest_name.c_str());
    ImGui::Separator();

    const auto& backtesting_summaries = backtest.summaries();
    if(!backtesting_summaries.empty() &&
       ImGui::BeginTable(
        "TradeSummaryTable", 2, ImGuiTableFlags_BordersInnerH)) {
      const auto& summary = backtesting_summaries.back();

      draw_row("Asset", backtest.asset().name());
      draw_row("Strategy", backtest.strategy().name());
      draw_count_row("Total trades", summary.trade_count());
      draw_duration_row("Total duration", summary.cumulative_durations());

      draw_spacer_row();

      draw_currency_with_rate_row("Expected value",
                                  summary.expected_value(),
                                  summary.expected_value() /
                                   summary.average_investment());
      draw_float_row("Profit factor", summary.profit_factor());

      draw_spacer_row();

      draw_currency_with_rate_row("Avg P&L",
                                  summary.average_pnl(),
                                  summary.average_pnl() /
                                   summary.average_investment());
      draw_currency_with_rate_row("Avg profit",
                                  summary.average_profit(),
                                  summary.average_profit() /
                                   summary.average_investment());
      draw_currency_with_rate_row("Avg loss",
                                  summary.average_loss(),
                                  summary.average_loss() /
                                   summary.average_investment());
      draw_currency_row("Avg investment", summary.average_investment());
      draw_duration_row("Avg duration", summary.average_duration());

      draw_spacer_row();

      draw_count_row_with_rate(
       "Winning trades", summary.profit_count(), summary.profit_rate());
      draw_count_row_with_rate(
       "Losing trades", summary.loss_count(), summary.loss_rate());
      draw_count_row_with_rate("Break-even trades",
                               summary.break_even_count(),
                               summary.break_even_rate());

      draw_spacer_row();

      draw_count_row_with_rate("Take profit hits",
                               summary.take_profit_count(),
                               summary.take_profit_rate());
      draw_currency_with_rate_row("Avg take profit",
                                  summary.average_take_profit(),
                                  summary.average_take_profit() /
                                   summary.average_investment());
      draw_currency_with_rate_row("Take profit EV",
                                  summary.take_profit_expected_value(),
                                  summary.take_profit_expected_value() /
                                   summary.average_investment());

      draw_empty_row();

      draw_count_row_with_rate(
       "Stop loss hits", summary.stop_loss_count(), summary.stop_loss_rate());
      draw_currency_with_rate_row("Avg stop loss",
                                  summary.average_stop_loss(),
                                  summary.average_stop_loss() /
                                   summary.average_investment());
      draw_currency_with_rate_row("Stop loss EV",
                                  summary.stop_loss_expected_value(),
                                  summary.stop_loss_expected_value() /
                                   summary.average_investment());

      draw_empty_row();

      draw_count_row_with_rate("Exit signal hits",
                               summary.exit_signal_count(),
                               summary.exit_signal_rate());
      draw_currency_with_rate_row("Avg exit signal",
                                  summary.average_exit_signal(),
                                  summary.average_exit_signal() /
                                   summary.average_investment());
      draw_currency_with_rate_row("Exit signal EV",
                                  summary.exit_signal_expected_value(),
                                  summary.exit_signal_expected_value() /
                                   summary.average_investment());

      draw_spacer_row();

      draw_currency_row("Total investments", summary.cumulative_investments());
      draw_currency_with_percentage_row("Total profits",
                                        summary.cumulative_profits(),
                                        summary.cumulative_profit_percent());
      draw_currency_with_percentage_row("Total losses",
                                        summary.cumulative_losses(),
                                        summary.cumulative_loss_percent());
      draw_currency_with_rate_row("Net P&L",
                                  summary.cumulative_pnls(),
                                  summary.cumulative_pnls() /
                                   summary.cumulative_investments());

      draw_spacer_row();

      draw_count_row("Total open trades", summary.open_trade_count());
      draw_currency_with_rate_row("Unrealized P&L",
                                  summary.unrealized_pnl(),
                                  summary.unrealized_pnl() /
                                   summary.cumulative_investments());
      draw_duration_row("Ongoing trade duration",
                        summary.ongoing_trade_duration());
    }

    ImGui::EndTable();
  }

  ImGui::End();
}

void BacktestSummaryWindow::draw_row(std::string_view label,
                                     std::string_view value) const
{
  ImGui::TableNextRow();
  ImGui::TableNextColumn();
  ImGui::Text("%s", label.data());
  ImGui::TableNextColumn();
  ImGui::Text("%s", value.data());
}

void BacktestSummaryWindow::draw_empty_row() const
{
  ImGui::TableNextRow();
  ImGui::TableNextColumn();
  ImGui::TableNextColumn();
}

void BacktestSummaryWindow::draw_spacer_row() const
{
  draw_row("", "");
}

void BacktestSummaryWindow::draw_float_row(std::string_view label,
                                           double value) const
{
  draw_row(label, std::format("{:.2f}", value));
}

void BacktestSummaryWindow::draw_count_row(std::string_view label,
                                           std::size_t value) const
{
  draw_row(label, std::to_string(value));
}

void BacktestSummaryWindow::draw_count_row_with_rate(std::string_view label,
                                                     std::size_t value,
                                                     double rate) const
{
  draw_row(label, std::format("{} ({:.2f}%)", value, rate * 100.0));
}

void BacktestSummaryWindow::draw_currency_row(std::string_view label,
                                              double value) const
{
  draw_row(label, format_currency(value));
}

void BacktestSummaryWindow::draw_currency_with_rate_row(std::string_view label,
                                                        double value,
                                                        double rate) const
{
  draw_currency_with_percentage_row(label, value, rate * 100.0);
}

void BacktestSummaryWindow::draw_currency_with_percentage_row(
 std::string_view label, double value, double percentage) const
{
  draw_row(label,
           std::format("{} ({:.2f}%)", format_currency(value), percentage));
}

void BacktestSummaryWindow::draw_duration_row(std::string_view label,
                                              std::size_t duration) const
{
  draw_row(label, format_duration(duration));
}

void BacktestSummaryWindow::draw_datetime_row(std::string_view label,
                                              std::time_t timestamp) const
{
  draw_row(label, format_datetime(timestamp));
}

} // namespace pludux::apps
