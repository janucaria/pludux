module;

#include <chrono>
#include <format>
#include <iostream>
#include <sstream>

#include <imgui.h>

export module pludux.apps.backtest:windows.backtest_summary_window;

import :app_state;

export namespace pludux::apps {

class BacktestSummaryWindow {
public:
  void render(this const auto self, AppState& app_state)
  {
    auto& state = app_state.state();
    auto& backtests = state.backtests;

    ImGui::Begin("Summary", nullptr);

    if(!state.backtests.empty() && state.selected_backtest_index >= 0) {
      const auto& backtest = backtests[state.selected_backtest_index];
      const auto& profile = backtest.get_profile();

      const auto& backtest_name = backtest.name();
      ImGui::Text("%s", backtest_name.c_str());
      ImGui::Separator();

      const auto& backtesting_summaries = backtest.summaries();
      if(!backtesting_summaries.empty() &&
         ImGui::BeginTable(
          "TradeSummaryTable", 2, ImGuiTableFlags_BordersInnerH)) {
        const auto& summary = backtesting_summaries.back();

        self.draw_row("Asset", backtest.asset().name());
        self.draw_row("Strategy", backtest.strategy().name());
        self.draw_row("Profile", profile.name());
        self.draw_count_row("Total trades", summary.trade_count());
        self.draw_duration_row("Total duration",
                               summary.cumulative_durations());

        self.draw_spacer_row();

        self.draw_currency_with_rate_row("Expected value",
                                         summary.expected_value(),
                                         summary.expected_value() /
                                          summary.average_investment());
        self.draw_float_row("Profit factor", summary.profit_factor());

        self.draw_spacer_row();

        self.draw_currency_with_rate_row("Avg P&L",
                                         summary.average_pnl(),
                                         summary.average_pnl() /
                                          summary.average_investment());
        self.draw_currency_with_rate_row("Avg profit",
                                         summary.average_profit(),
                                         summary.average_profit() /
                                          summary.average_investment());
        self.draw_currency_with_rate_row("Avg loss",
                                         summary.average_loss(),
                                         summary.average_loss() /
                                          summary.average_investment());
        self.draw_currency_row("Avg investment", summary.average_investment());
        self.draw_duration_row("Avg duration", summary.average_duration());

        self.draw_spacer_row();

        self.draw_count_row_with_rate(
         "Winning trades", summary.profit_count(), summary.profit_rate());
        self.draw_count_row_with_rate(
         "Losing trades", summary.loss_count(), summary.loss_rate());
        self.draw_count_row_with_rate("Break-even trades",
                                      summary.break_even_count(),
                                      summary.break_even_rate());

        self.draw_spacer_row();

        self.draw_currency_row("Initial capital", summary.initial_capital());
        self.draw_currency_with_percent_row("Total profits",
                                            summary.cumulative_profits(),
                                            summary.initial_capital());
        self.draw_currency_with_percent_row("Total losses",
                                            summary.cumulative_losses(),
                                            summary.initial_capital());
        self.draw_currency_with_percent_row(
         "Net P&L", summary.cumulative_pnls(), summary.initial_capital());
        self.draw_currency_with_percent_row(
         "Total Capital", summary.capital(), summary.initial_capital());

        self.draw_spacer_row();
        self.draw_currency_with_percent_row(
         "Equity", summary.equity(), summary.initial_capital());
        self.draw_currency_with_percent_row(
         "Peak equity", summary.peak_equity(), summary.initial_capital());
        self.draw_row("Drawdown", std::format("{:.2f}%", summary.drawdown()));
        self.draw_row("Max drawdown",
                      std::format("{:.2f}%", summary.max_drawdown()));

        self.draw_spacer_row();

        self.draw_count_row("Total open trades", summary.open_trade_count());
        self.draw_currency_with_percent_row("Unrealized P&L",
                                            summary.unrealized_pnl(),
                                            summary.unrealized_investment());
        self.draw_duration_row("Ongoing trade duration",
                               summary.unrealized_duration());
      }

      ImGui::EndTable();
    }

    ImGui::End();
  }

private:
  void draw_row(this const auto, std::string_view label, std::string_view value)
  {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("%s", label.data());
    ImGui::TableNextColumn();
    ImGui::Text("%s", value.data());
  }

  void draw_empty_row(this const auto)
  {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
  }

  void draw_spacer_row(this const auto self)
  {
    self.draw_row("", "");
  }

  void
  draw_float_row(this const auto self, std::string_view label, double value)
  {
    self.draw_row(label, std::format("{:.2f}", value));
  }

  void draw_count_row(this const auto self,
                      std::string_view label,
                      std::size_t value)
  {
    self.draw_row(label, std::to_string(value));
  }

  void draw_count_row_with_rate(this const auto self,
                                std::string_view label,
                                std::size_t value,
                                double rate)
  {
    self.draw_row(label, std::format("{} ({:.2f}%)", value, rate * 100.0));
  }

  void
  draw_currency_row(this const auto self, std::string_view label, double value)
  {
    self.draw_row(label, format_currency(value));
  }

  void draw_currency_with_rate_row(this const auto self,
                                   std::string_view label,
                                   double value,
                                   double rate)
  {
    self.draw_row(
     label, std::format("{} ({:.2f}%)", format_currency(value), rate * 100.0));
  }

  void draw_currency_with_percent_row(this const auto self,
                                      std::string_view label,
                                      double value,
                                      double total)
  {
    const auto percentage = total != 0.0 ? value / total : 0.0;
    self.draw_row(
     label,
     std::format("{} ({:.2f}%)", format_currency(value), percentage * 100.0));
  }

  void draw_duration_row(this const auto self,
                         std::string_view label,
                         std::size_t duration)
  {
    self.draw_row(label, format_duration(duration));
  }

  void draw_datetime_row(this const auto self,
                         std::string_view label,
                         std::time_t timestamp)
  {
    self.draw_row(label, format_datetime(timestamp));
  }
};

} // namespace pludux::apps
