module;

#include <chrono>
#include <format>
#include <iostream>
#include <sstream>

#include <imgui.h>

export module pludux.apps.backtest:windows.backtest_summary_window;

import :window_context;

export namespace pludux::apps {

class BacktestSummaryWindow {
public:
  void render(this const BacktestSummaryWindow self, WindowContext& context)
  {
    auto& app_state = context.app_state();

    ImGui::Begin("Summary", nullptr);

    const auto backtest_handle = app_state.selected_backtest_handle();
    const auto backtest_ptr =
     app_state.get_backtest_if_present(backtest_handle);
    if(backtest_ptr && app_state.is_backtest_ready(*backtest_ptr)) {
      const auto& backtest = *backtest_ptr;

      const auto asset_handle = backtest.asset_handle();
      const auto& asset = app_state.get_asset(asset_handle);

      const auto strategy_handle = backtest.strategy_handle();
      const auto& strategy = app_state.get_strategy(strategy_handle);

      const auto profile_handle = backtest.profile_handle();
      const auto& profile = app_state.get_profile(profile_handle);

      const auto& market_handle = backtest.market_handle();
      const auto& market = app_state.get_market(market_handle);

      const auto& broker_handle = backtest.broker_handle();
      const auto& broker = app_state.get_broker(broker_handle);

      const auto& timeline = app_state.get_backtest_timelines(backtest_handle);

      const auto& backtest_name = backtest.name();
      ImGui::Text("%s", backtest_name.c_str());
      ImGui::Separator();

      if(!timeline.empty() &&
         ImGui::BeginTable(
          "TradeSummaryTable", 2, ImGuiTableFlags_BordersInnerH)) {
        const auto timeline_i = timeline.size() - 1;

        self.draw_row("Asset", asset.name());
        self.draw_row("Strategy", strategy.name());
        self.draw_row("Market", market.name());
        self.draw_row("Broker", broker.name());
        self.draw_row("Profile", profile.name());

        self.draw_spacer_row();

        self.draw_count_row("Total trades", timeline.trade_count(timeline_i));
        self.draw_duration_row("Total duration",
                               timeline.cumulative_durations(timeline_i));

        self.draw_spacer_row();

        self.draw_float_row("Profit factor",
                            timeline.profit_factor(timeline_i));
        self.draw_currency_with_rate_row(
         "Expectancy",
         timeline.expected_value(timeline_i),
         timeline.expected_value(timeline_i) /
          timeline.average_investment(timeline_i));

        self.draw_count_row_with_rate("Winning trades",
                                      timeline.profit_count(timeline_i),
                                      timeline.profit_rate(timeline_i));
        self.draw_count_row_with_rate("Losing trades",
                                      timeline.loss_count(timeline_i),
                                      timeline.loss_rate(timeline_i));
        self.draw_count_row_with_rate("Break-even trades",
                                      timeline.break_even_count(timeline_i),
                                      timeline.break_even_rate(timeline_i));

        self.draw_spacer_row();

        self.draw_currency_with_rate_row(
         "Avg P&L",
         timeline.average_pnl(timeline_i),
         timeline.average_pnl(timeline_i) /
          timeline.average_investment(timeline_i));
        self.draw_currency_with_rate_row(
         "Avg profit",
         timeline.average_profit(timeline_i),
         timeline.average_profit(timeline_i) /
          timeline.average_investment(timeline_i));
        self.draw_currency_with_rate_row(
         "Avg loss",
         timeline.average_loss(timeline_i),
         timeline.average_loss(timeline_i) /
          timeline.average_investment(timeline_i));
        self.draw_currency_row("Avg investment",
                               timeline.average_investment(timeline_i));
        self.draw_duration_row("Avg duration",
                               timeline.average_duration(timeline_i));

        self.draw_spacer_row();

        self.draw_currency_row("Initial capital",
                               timeline.initial_capital(timeline_i));
        self.draw_currency_with_percent_row(
         "Total profits",
         timeline.cumulative_profits(timeline_i),
         timeline.initial_capital(timeline_i));
        self.draw_currency_with_percent_row(
         "Total losses",
         timeline.cumulative_losses(timeline_i),
         timeline.initial_capital(timeline_i));
        self.draw_currency_with_percent_row(
         "Net P&L",
         timeline.cumulative_pnls(timeline_i),
         timeline.initial_capital(timeline_i));
        self.draw_currency_with_percent_row(
         "Total Capital",
         timeline.capital(timeline_i),
         timeline.initial_capital(timeline_i));

        self.draw_spacer_row();
        self.draw_currency_with_percent_row(
         "Equity",
         timeline.equity(timeline_i),
         timeline.initial_capital(timeline_i));
        self.draw_currency_with_percent_row(
         "Peak equity",
         timeline.peak_equity(timeline_i),
         timeline.initial_capital(timeline_i));
        self.draw_row("Drawdown",
                      std::format("{:.2f}%", timeline.drawdown(timeline_i)));
        self.draw_row(
         "Max drawdown",
         std::format("{:.2f}%", timeline.max_drawdown(timeline_i)));

        self.draw_spacer_row();

        self.draw_count_row("Total open trades",
                            timeline.open_trade_count(timeline_i));
        self.draw_currency_with_percent_row(
         "Unrealized P&L",
         timeline.unrealized_pnl(timeline_i),
         timeline.unrealized_investment(timeline_i));
        self.draw_duration_row("Ongoing trade duration",
                               timeline.unrealized_duration(timeline_i));

        ImGui::EndTable();
      }
    }

    ImGui::End();
  }

private:
  void draw_row(this const BacktestSummaryWindow,
                std::string_view label,
                std::string_view value)
  {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::Text("%s", label.data());
    ImGui::TableNextColumn();
    ImGui::Text("%s", value.data());
  }

  void draw_empty_row(this const BacktestSummaryWindow)
  {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::TableNextColumn();
  }

  void draw_spacer_row(this const BacktestSummaryWindow self)
  {
    self.draw_row("", "");
  }

  void draw_float_row(this const BacktestSummaryWindow self,
                      std::string_view label,
                      double value)
  {
    self.draw_row(label, std::format("{:.2f}", value));
  }

  void draw_count_row(this const BacktestSummaryWindow self,
                      std::string_view label,
                      std::size_t value)
  {
    self.draw_row(label, std::to_string(value));
  }

  void draw_count_row_with_rate(this const BacktestSummaryWindow self,
                                std::string_view label,
                                std::size_t value,
                                double rate)
  {
    self.draw_row(label, std::format("{} ({:.2f}%)", value, rate * 100.0));
  }

  void draw_currency_row(this const BacktestSummaryWindow self,
                         std::string_view label,
                         double value)
  {
    self.draw_row(label, format_currency(value));
  }

  void draw_currency_with_rate_row(this const BacktestSummaryWindow self,
                                   std::string_view label,
                                   double value,
                                   double rate)
  {
    self.draw_row(
     label, std::format("{} ({:.2f}%)", format_currency(value), rate * 100.0));
  }

  void draw_currency_with_percent_row(this const BacktestSummaryWindow self,
                                      std::string_view label,
                                      double value,
                                      double total)
  {
    const auto percentage = total != 0.0 ? value / total : 0.0;
    self.draw_row(
     label,
     std::format("{} ({:.2f}%)", format_currency(value), percentage * 100.0));
  }

  void draw_duration_row(this const BacktestSummaryWindow self,
                         std::string_view label,
                         std::size_t duration)
  {
    self.draw_row(label, format_duration(duration));
  }

  void draw_datetime_row(this const BacktestSummaryWindow self,
                         std::string_view label,
                         std::time_t timestamp)
  {
    self.draw_row(label, format_datetime(timestamp));
  }
};

} // namespace pludux::apps
