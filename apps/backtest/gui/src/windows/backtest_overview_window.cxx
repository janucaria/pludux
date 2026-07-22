module;

#include <array>
#include <cmath>
#include <cstddef>
#include <format>
#include <limits>
#include <optional>
#include <string>
#include <string_view>

#include "../ui/pludux_icons.hpp"

#include <imgui.h>

export module pludux.apps.backtest:windows.backtest_overview_window;

import pludux.backtest;

import :ui.summary_widgets;
import :window_context;

export namespace pludux::apps {

class BacktestOverviewWindow {
public:
  void render(this const BacktestOverviewWindow, WindowContext& context)
  {
    ImGui::Begin("Overview", nullptr);

    const auto& app_state = context.app_state();
    const auto backtest_handle = app_state.selected_backtest_handle();
    const auto backtest_ptr =
     app_state.get_backtest_if_present(backtest_handle);

    if(!backtest_ptr) {
      ui::summary_status(PLUDUX_ICON_SUMMARY,
                         "No backtest selected",
                         "Select a backtest to review its performance, trade "
                         "quality, capital, and risk metrics.");
      ImGui::End();
      return;
    }

    const auto& backtest = *backtest_ptr;
    if(!app_state.is_backtest_ready(backtest)) {
      ui::summary_status(PLUDUX_ICON_WARNING,
                         "Backtest setup is incomplete",
                         "Complete the asset, strategy, market, broker, and "
                         "profile selections before results can be shown.");
      ImGui::End();
      return;
    }

    const auto& timeline = app_state.get_backtest_timelines(backtest_handle);
    if(timeline.empty()) {
      ui::summary_status(PLUDUX_ICON_PERFORMANCE,
                         "Results are not available yet",
                         "The summary will populate when this backtest has "
                         "produced its first timeline result.");
      ImGui::End();
      return;
    }

    const auto& asset = app_state.get_asset(backtest.asset_handle());
    const auto& strategy = app_state.get_strategy(backtest.strategy_handle());
    const auto& market = app_state.get_market(backtest.market_handle());
    const auto& broker = app_state.get_broker(backtest.broker_handle());
    const auto& profile = app_state.get_profile(backtest.profile_handle());
    const auto timeline_index = timeline.size() - 1;

    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() +
                           ImGui::GetContentRegionAvail().x);
    ImGui::SetWindowFontScale(1.18f);
    ImGui::TextUnformatted(backtest.name().c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopTextWrapPos();
    ImGui::TextDisabled("Latest timeline result | %zu closed trades",
                        timeline.trade_count(timeline_index));
    ImGui::Spacing();

    const auto metadata =
     std::array{ui::SummaryMetric{"Asset", asset.name(), {}, {}, {}},
                ui::SummaryMetric{"Strategy", strategy.name(), {}, {}, {}},
                ui::SummaryMetric{"Market", market.name(), {}, {}, {}},
                ui::SummaryMetric{"Broker", broker.name(), {}, {}, {}},
                ui::SummaryMetric{"Profile", profile.name(), {}, {}, {}}};
    ui::summary_metric_section("##summary_setup",
                               PLUDUX_ICON_SUMMARY,
                               "Configuration",
                               "Inputs used for this result.",
                               metadata);
    ImGui::Spacing();

    const auto initial_capital = timeline.initial_capital(timeline_index);
    const auto net_pnl = timeline.cumulative_pnls(timeline_index);
    const auto trade_count = timeline.trade_count(timeline_index);
    const auto win_rate = timeline.profit_rate(timeline_index) * 100.0;
    const auto net_return = percentage_of(net_pnl, initial_capital);

    const auto available_width = ImGui::GetContentRegionAvail().x;
    const auto card_spacing = ImGui::GetStyle().ItemSpacing.x;
    const auto column_count = available_width >= 520.0f ? 2 : 1;
    const auto card_width =
     column_count == 2 ? (available_width - card_spacing) * 0.5f : -1.0f;

    ui::summary_kpi_card("##net_pnl_card",
                         "NET P&L",
                         format_currency(net_pnl),
                         format_optional_percent(net_return),
                         outcome_tone(net_pnl),
                         card_width);
    if(column_count == 2) {
      ImGui::SameLine();
    }
    ui::summary_kpi_card("##ending_capital_card",
                         "ENDING CAPITAL",
                         format_currency(timeline.capital(timeline_index)),
                         "Realized balance",
                         outcome_tone(net_pnl),
                         card_width);
    if(column_count == 2) {
      ImGui::Spacing();
    }
    ui::summary_kpi_card("##win_rate_card",
                         "WIN RATE",
                         format_percent(win_rate),
                         std::format("{} closed trades", trade_count),
                         win_rate >= 50.0 ? ui::MetricTone::Positive
                                          : ui::MetricTone::Neutral,
                         card_width);
    if(column_count == 2) {
      ImGui::SameLine();
    }
    ui::summary_kpi_card("##max_drawdown_card",
                         "MAX DRAWDOWN",
                         format_percent(timeline.max_drawdown(timeline_index)),
                         "Peak-to-trough risk",
                         timeline.max_drawdown(timeline_index) > 0.0
                          ? ui::MetricTone::Warning
                          : ui::MetricTone::Neutral,
                         card_width);
    ImGui::Spacing();

    const auto average_investment = timeline.average_investment(timeline_index);
    const auto average_profit = timeline.average_profit(timeline_index);
    const auto average_loss = timeline.average_loss(timeline_index);
    const auto profit_factor = timeline.profit_factor(timeline_index);
    const auto payoff = payoff_ratio(average_profit,
                                     average_loss,
                                     timeline.profit_count(timeline_index),
                                     timeline.loss_count(timeline_index));
    const auto return_to_drawdown =
     recovery_ratio(net_return, timeline.max_drawdown(timeline_index));
    const auto performance_metrics = std::array{
     ui::SummaryMetric{"Total return",
                       format_optional_percent(net_return),
                       {},
                       "Net realized P&L relative to initial capital.",
                       outcome_tone(net_pnl)},
     ui::SummaryMetric{
      "Profit factor",
      trade_count > 0 ? format_ratio(profit_factor) : "—",
      {},
      "Gross profit divided by the absolute value of gross loss.",
      trade_count > 0 && profit_factor >= 1.0
       ? ui::MetricTone::Positive
       : (trade_count > 0 ? ui::MetricTone::Negative
                          : ui::MetricTone::Neutral)},
     ui::SummaryMetric{
      "Payoff ratio",
      format_optional_ratio(payoff),
      {},
      "Average winning trade divided by the absolute average losing trade.",
      payoff && *payoff >= 1.0 ? ui::MetricTone::Positive
                               : ui::MetricTone::Neutral},
     ui::SummaryMetric{
      "Return / max drawdown",
      format_optional_ratio(return_to_drawdown),
      {},
      "Total return divided by maximum drawdown; higher values indicate more "
      "return per unit of observed drawdown.",
      return_to_drawdown && *return_to_drawdown > 0.0
       ? ui::MetricTone::Positive
       : ui::MetricTone::Neutral},
     ui::SummaryMetric{
      "Expectancy",
      format_currency(timeline.expected_value(timeline_index)),
      format_percent(timeline.expected_return(timeline_index)),
      "Average expected P&L per closed trade and its return on average "
      "investment.",
      outcome_tone(timeline.expected_value(timeline_index))},
     ui::SummaryMetric{
      "Average P&L",
      format_currency(timeline.average_pnl(timeline_index)),
      format_optional_percent(
       percentage_of(timeline.average_pnl(timeline_index), average_investment)),
      "Average realized P&L per closed trade.",
      outcome_tone(timeline.average_pnl(timeline_index))},
     ui::SummaryMetric{
      "Gross profit",
      format_currency(timeline.cumulative_profits(timeline_index)),
      format_optional_percent(percentage_of(
       timeline.cumulative_profits(timeline_index), initial_capital)),
      "Sum of profitable trades; the percentage is relative to initial "
      "capital.",
      ui::MetricTone::Positive},
     ui::SummaryMetric{
      "Gross loss",
      format_currency(timeline.cumulative_losses(timeline_index)),
      format_optional_percent(percentage_of(
       timeline.cumulative_losses(timeline_index), initial_capital)),
      "Sum of losing trades; the percentage is relative to initial capital.",
      ui::MetricTone::Negative}};
    ui::summary_metric_section("##performance",
                               PLUDUX_ICON_PERFORMANCE,
                               "Performance",
                               "Profitability and return quality.",
                               performance_metrics);
    ImGui::Spacing();

    const auto current_winning_streak = timeline.current_winning_streak();
    const auto current_losing_streak = timeline.current_losing_streak();
    const auto current_streak_value =
     current_winning_streak > 0
      ? format_streak(current_winning_streak, "win")
      : (current_losing_streak > 0
          ? format_streak(current_losing_streak, "loss", "losses")
          : std::string{"—"});
    const auto current_streak_tone =
     current_winning_streak > 0
      ? ui::MetricTone::Positive
      : (current_losing_streak > 0 ? ui::MetricTone::Negative
                                   : ui::MetricTone::Neutral);
    const auto trade_metrics = std::array{
     ui::SummaryMetric{"Closed trades",
                       std::to_string(trade_count),
                       {},
                       "Completed winning, losing, and break-even trades.",
                       {}},
     ui::SummaryMetric{"Winners",
                       std::to_string(timeline.profit_count(timeline_index)),
                       format_percent(win_rate),
                       {},
                       ui::MetricTone::Positive},
     ui::SummaryMetric{
      "Losers",
      std::to_string(timeline.loss_count(timeline_index)),
      format_percent(timeline.loss_rate(timeline_index) * 100.0),
      {},
      ui::MetricTone::Negative},
     ui::SummaryMetric{
      "Break-even",
      std::to_string(timeline.break_even_count(timeline_index)),
      format_percent(timeline.break_even_rate(timeline_index) * 100.0),
      {},
      {}},
     ui::SummaryMetric{
      "Current streak",
      current_streak_value,
      {},
      "Consecutive outcomes ending at the most recently closed trade; a "
      "break-even trade resets the streak.",
      current_streak_tone},
     ui::SummaryMetric{"Maximum winning streak",
                       format_streak(timeline.maximum_winning_streak(), "win"),
                       {},
                       "Largest number of consecutive profitable trades.",
                       ui::MetricTone::Positive},
     ui::SummaryMetric{
      "Maximum losing streak",
      format_streak(timeline.maximum_losing_streak(), "loss", "losses"),
      {},
      "Largest number of consecutive losing trades.",
      timeline.maximum_losing_streak() > 0 ? ui::MetricTone::Negative
                                           : ui::MetricTone::Neutral},
     ui::SummaryMetric{"Average winner",
                       format_currency(timeline.average_profit(timeline_index)),
                       format_optional_percent(
                        percentage_of(average_profit, average_investment)),
                       "Average P&L among profitable trades.",
                       ui::MetricTone::Positive},
     ui::SummaryMetric{
      "Average loser",
      format_currency(timeline.average_loss(timeline_index)),
      format_optional_percent(percentage_of(average_loss, average_investment)),
      "Average P&L among losing trades.",
      ui::MetricTone::Negative},
     ui::SummaryMetric{"Average investment",
                       format_currency(average_investment),
                       {},
                       "Average capital committed per closed trade.",
                       {}},
     ui::SummaryMetric{
      "Total invested capital",
      format_currency(timeline.cumulative_investments(timeline_index)),
      format_optional_multiple(division_of(
       timeline.cumulative_investments(timeline_index), initial_capital)),
      "Capital committed across all closed trades; the multiple is relative "
      "to initial capital.",
      {}},
     ui::SummaryMetric{
      "Total trade duration",
      format_duration(timeline.cumulative_durations(timeline_index)),
      {},
      "Combined duration of all closed trades.",
      {}},
     ui::SummaryMetric{
      "Average trade duration",
      format_duration(timeline.average_duration(timeline_index)),
      {},
      {},
      {}}};
    ui::summary_metric_section("##trades",
                               PLUDUX_ICON_TRADES,
                               "Trades",
                               "Outcome distribution and typical trade size.",
                               trade_metrics);
    ImGui::Spacing();

    const auto capital_metrics = std::array{
     ui::SummaryMetric{
      "Initial capital", format_currency(initial_capital), {}, {}, {}},
     ui::SummaryMetric{"Ending capital",
                       format_currency(timeline.capital(timeline_index)),
                       format_optional_percent(net_return),
                       "Realized balance after closed trades.",
                       outcome_tone(net_pnl)},
     ui::SummaryMetric{
      "Equity",
      format_currency(timeline.equity(timeline_index)),
      format_optional_percent(percentage_of(
       timeline.equity(timeline_index) - initial_capital, initial_capital)),
      "Current balance including unrealized P&L.",
      outcome_tone(timeline.equity(timeline_index) - initial_capital)},
     ui::SummaryMetric{"Peak equity",
                       format_currency(timeline.peak_equity(timeline_index)),
                       format_optional_percent(percentage_of(
                        timeline.peak_equity(timeline_index) - initial_capital,
                        initial_capital)),
                       "Highest equity reached during the backtest.",
                       ui::MetricTone::Positive},
     ui::SummaryMetric{"Current drawdown",
                       format_percent(timeline.drawdown(timeline_index)),
                       {},
                       "Current decline from peak equity.",
                       timeline.drawdown(timeline_index) > 0.0
                        ? ui::MetricTone::Warning
                        : ui::MetricTone::Neutral},
     ui::SummaryMetric{"Maximum drawdown",
                       format_percent(timeline.max_drawdown(timeline_index)),
                       {},
                       "Largest peak-to-trough decline during the backtest.",
                       timeline.max_drawdown(timeline_index) > 0.0
                        ? ui::MetricTone::Warning
                        : ui::MetricTone::Neutral}};
    ui::summary_metric_section("##capital_risk",
                               PLUDUX_ICON_CAPITAL,
                               "Capital & risk",
                               "Balance, equity, and downside exposure.",
                               capital_metrics);
    ImGui::Spacing();

    const auto open_investment = timeline.unrealized_investment(timeline_index);
    const auto open_pnl = timeline.unrealized_pnl(timeline_index);
    const auto equity = timeline.equity(timeline_index);
    const auto exposure_metrics = std::array{
     ui::SummaryMetric{
      "Open trades",
      std::to_string(timeline.open_trade_count(timeline_index)),
      {},
      "Trades that remain open at the latest timeline result.",
      timeline.open_trade_count(timeline_index) > 0 ? ui::MetricTone::Warning
                                                    : ui::MetricTone::Neutral},
     ui::SummaryMetric{
      "Open investment",
      format_currency(open_investment),
      format_optional_percent(percentage_of(open_investment, equity)),
      "Capital currently committed to the open position; "
      "the percentage is relative to current equity.",
      {}},
     ui::SummaryMetric{
      "Unrealized P&L",
      format_currency(open_pnl),
      format_optional_percent(percentage_of(open_pnl, open_investment)),
      "Open-position P&L and return on currently invested capital.",
      outcome_tone(open_pnl)},
     ui::SummaryMetric{
      "Open trade duration",
      format_duration(timeline.unrealized_duration(timeline_index)),
      {},
      {},
      {}}};
    ui::summary_metric_section("##open_exposure",
                               PLUDUX_ICON_EXPOSURE,
                               "Open exposure",
                               timeline.open_trade_count(timeline_index) > 0
                                ? "Current open-position status."
                                : "No trade is open at the latest result.",
                               exposure_metrics);

    ImGui::End();
  }

private:
  static auto percentage_of(double value, double total) -> std::optional<double>
  {
    if(total == 0.0 || !std::isfinite(value) || !std::isfinite(total)) {
      return std::nullopt;
    }
    return (value / total) * 100.0;
  }

  static auto division_of(double value, double total) -> std::optional<double>
  {
    if(total == 0.0 || !std::isfinite(value) || !std::isfinite(total)) {
      return std::nullopt;
    }
    return value / total;
  }

  static auto payoff_ratio(double average_profit,
                           double average_loss,
                           std::size_t profit_count,
                           std::size_t loss_count) -> std::optional<double>
  {
    if(profit_count == 0) {
      return std::nullopt;
    }
    if(loss_count == 0 || average_loss == 0.0) {
      return std::numeric_limits<double>::infinity();
    }
    return average_profit / std::abs(average_loss);
  }

  static auto recovery_ratio(std::optional<double> total_return,
                             double maximum_drawdown) -> std::optional<double>
  {
    if(!total_return || !std::isfinite(maximum_drawdown)) {
      return std::nullopt;
    }
    if(maximum_drawdown == 0.0) {
      return *total_return > 0.0
              ? std::optional{std::numeric_limits<double>::infinity()}
              : std::nullopt;
    }
    return *total_return / maximum_drawdown;
  }

  static auto format_percent(double value) -> std::string
  {
    return std::isfinite(value) ? std::format("{:.2f}%", value) : "—";
  }

  static auto format_optional_percent(std::optional<double> value)
   -> std::string
  {
    return value ? format_percent(*value) : "—";
  }

  static auto format_ratio(double value) -> std::string
  {
    if(std::isinf(value) && value > 0.0) {
      return "∞";
    }
    return std::isfinite(value) ? std::format("{:.2f}", value) : "—";
  }

  static auto format_optional_ratio(std::optional<double> value) -> std::string
  {
    return value ? format_ratio(*value) : "—";
  }

  static auto format_optional_multiple(std::optional<double> value)
   -> std::string
  {
    return value ? std::format("{}x", format_ratio(*value)) : "—";
  }

  static auto format_streak(std::size_t count,
                            std::string_view singular,
                            std::string_view plural = {}) -> std::string
  {
    const auto unit = count == 1 || plural.empty() ? singular : plural;
    return std::format(
     "{} {}{}", count, unit, count == 1 || !plural.empty() ? "" : "s");
  }

  static auto outcome_tone(double value) -> ui::MetricTone
  {
    if(value > 0.0) {
      return ui::MetricTone::Positive;
    }
    if(value < 0.0) {
      return ui::MetricTone::Negative;
    }
    return ui::MetricTone::Neutral;
  }
};

} // namespace pludux::apps
