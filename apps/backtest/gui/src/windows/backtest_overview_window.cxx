module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <ctime>
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
    const auto portfolio_handle = app_state.selected_portfolio_handle();
    const auto portfolio_ptr =
     app_state.get_portfolio_if_present(portfolio_handle);

    if(!portfolio_ptr) {
      ui::summary_status(PLUDUX_ICON_SUMMARY,
                         "No portfolio selected",
                         "Select a portfolio to review its performance, trade "
                         "quality, capital, and risk metrics.");
      ImGui::End();
      return;
    }

    const auto& portfolio = *portfolio_ptr;
    if(!app_state.is_portfolio_ready(portfolio)) {
      ui::summary_status(PLUDUX_ICON_WARNING,
                         "Portfolio setup is incomplete",
                         "Complete the shared market, broker, and backtest "
                         "backtests before results can be shown.");
      ImGui::End();
      return;
    }

    const auto& results = app_state.get_portfolio_results(portfolio_handle);
    if(results.timeline().empty() || results.backtests().empty()) {
      ui::summary_status(PLUDUX_ICON_PERFORMANCE,
                         "Results are not available yet",
                         "The summary will populate when this portfolio has "
                         "produced its first timeline result.");
      ImGui::End();
      return;
    }

    const auto& market = app_state.get_market(portfolio.market_handle());
    const auto& broker = app_state.get_broker(portfolio.broker_handle());
    const auto& portfolio_row =
     results.timeline().row(results.timeline().size() - 1);
    auto trade_count = std::size_t{};
    auto profit_count = std::size_t{};
    auto loss_count = std::size_t{};
    auto break_even_count = std::size_t{};
    auto open_trade_count = std::size_t{};
    auto cumulative_profit = 0.0;
    auto cumulative_loss = 0.0;
    auto cumulative_investment = 0.0;
    auto open_investment = 0.0;
    auto open_pnl = 0.0;
    auto cumulative_duration = std::time_t{};
    auto open_duration = std::time_t{};
    auto current_winning_streak = std::size_t{};
    auto current_losing_streak = std::size_t{};
    auto maximum_winning_streak = std::size_t{};
    auto maximum_losing_streak = std::size_t{};
    for(const auto& backtest : results.backtests()) {
      const auto& backtest_timeline = backtest.timeline();
      if(backtest_timeline.empty()) {
        continue;
      }
      const auto index = backtest_timeline.size() - 1;
      trade_count += backtest_timeline.trade_count(index);
      profit_count += backtest_timeline.profit_count(index);
      loss_count += backtest_timeline.loss_count(index);
      break_even_count += backtest_timeline.break_even_count(index);
      open_trade_count += backtest_timeline.open_trade_count(index);
      cumulative_profit += backtest_timeline.cumulative_profits(index);
      cumulative_loss += backtest_timeline.cumulative_losses(index);
      cumulative_investment += backtest_timeline.cumulative_investments(index);
      open_investment += backtest_timeline.unrealized_investment(index);
      open_pnl += backtest_timeline.unrealized_pnl(index);
      cumulative_duration += backtest_timeline.cumulative_durations(index);
      open_duration += backtest_timeline.unrealized_duration(index);
      current_winning_streak = std::max(
       current_winning_streak, backtest_timeline.current_winning_streak());
      current_losing_streak = std::max(
       current_losing_streak, backtest_timeline.current_losing_streak());
      maximum_winning_streak = std::max(
       maximum_winning_streak, backtest_timeline.maximum_winning_streak());
      maximum_losing_streak = std::max(
       maximum_losing_streak, backtest_timeline.maximum_losing_streak());
    }

    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() +
                           ImGui::GetContentRegionAvail().x);
    ImGui::SetWindowFontScale(1.18f);
    ImGui::TextUnformatted(portfolio.name().c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopTextWrapPos();
    ImGui::TextDisabled("Latest timeline result | %zu closed trades",
                        trade_count);
    ImGui::Spacing();

    const auto metadata = std::array{
     ui::SummaryMetric{"Backtests",
                       std::to_string(results.backtests().size()),
                       {},
                       "Backtests share capital in their displayed order.",
                       {}},
     ui::SummaryMetric{"Market", market.name(), {}, {}, {}},
     ui::SummaryMetric{"Broker", broker.name(), {}, {}, {}}};
    ui::summary_metric_section("##summary_setup",
                               PLUDUX_ICON_SUMMARY,
                               "Configuration",
                               "Inputs used for this result.",
                               metadata);
    ImGui::Spacing();

    const auto initial_capital = portfolio.initial_capital();
    const auto net_pnl = portfolio_row.realized_pnl;
    const auto win_rate = trade_count > 0
                           ? static_cast<double>(profit_count) /
                              static_cast<double>(trade_count) * 100.0
                           : 0.0;
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
                         format_currency(portfolio_row.capital),
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
                         format_percent(portfolio_row.max_drawdown),
                         "Peak-to-trough risk",
                         portfolio_row.max_drawdown > 0.0
                          ? ui::MetricTone::Warning
                          : ui::MetricTone::Neutral,
                         card_width);
    ImGui::Spacing();

    const auto average_investment =
     trade_count > 0 ? cumulative_investment / trade_count : 0.0;
    const auto average_profit =
     profit_count > 0 ? cumulative_profit / profit_count : 0.0;
    const auto average_loss =
     loss_count > 0 ? cumulative_loss / loss_count : 0.0;
    const auto average_pnl =
     trade_count > 0 ? (cumulative_profit + cumulative_loss) / trade_count
                     : 0.0;
    const auto profit_factor = cumulative_loss != 0.0
                                ? cumulative_profit / std::abs(cumulative_loss)
                                : 0.0;
    const auto payoff =
     payoff_ratio(average_profit, average_loss, profit_count, loss_count);
    const auto return_to_drawdown =
     recovery_ratio(net_return, portfolio_row.max_drawdown);
    const auto performance_metrics = std::array{
     ui::SummaryMetric{"Total return",
                       format_optional_percent(net_return),
                       {},
                       "Net realized P&L relative to initial capital.",
                       outcome_tone(net_pnl)},
     ui::SummaryMetric{
      "Profit factor",
      trade_count > 0 ? format_ratio(profit_factor) : "â€”",
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
      format_currency(average_pnl),
      format_optional_percent(percentage_of(average_pnl, average_investment)),
      "Average expected P&L per closed trade and its return on average "
      "investment.",
      outcome_tone(average_pnl)},
     ui::SummaryMetric{
      "Average P&L",
      format_currency(average_pnl),
      format_optional_percent(percentage_of(average_pnl, average_investment)),
      "Average realized P&L per closed trade.",
      outcome_tone(average_pnl)},
     ui::SummaryMetric{
      "Gross profit",
      format_currency(cumulative_profit),
      format_optional_percent(
       percentage_of(cumulative_profit, initial_capital)),
      "Sum of profitable trades; the percentage is relative to initial "
      "capital.",
      ui::MetricTone::Positive},
     ui::SummaryMetric{
      "Gross loss",
      format_currency(cumulative_loss),
      format_optional_percent(percentage_of(cumulative_loss, initial_capital)),
      "Sum of losing trades; the percentage is relative to initial capital.",
      ui::MetricTone::Negative}};
    ui::summary_metric_section("##performance",
                               PLUDUX_ICON_PERFORMANCE,
                               "Performance",
                               "Profitability and return quality.",
                               performance_metrics);
    ImGui::Spacing();

    const auto current_streak_value =
     current_winning_streak > 0
      ? format_streak(current_winning_streak, "win")
      : (current_losing_streak > 0
          ? format_streak(current_losing_streak, "loss", "losses")
          : std::string{"â€”"});
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
                       std::to_string(profit_count),
                       format_percent(win_rate),
                       {},
                       ui::MetricTone::Positive},
     ui::SummaryMetric{
      "Losers",
      std::to_string(loss_count),
      format_percent(trade_count > 0
                      ? static_cast<double>(loss_count) / trade_count * 100.0
                      : 0.0),
      {},
      ui::MetricTone::Negative},
     ui::SummaryMetric{"Break-even",
                       std::to_string(break_even_count),
                       format_percent(trade_count > 0
                                       ? static_cast<double>(break_even_count) /
                                          trade_count * 100.0
                                       : 0.0),
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
                       format_streak(maximum_winning_streak, "win"),
                       {},
                       "Largest number of consecutive profitable trades.",
                       ui::MetricTone::Positive},
     ui::SummaryMetric{"Maximum losing streak",
                       format_streak(maximum_losing_streak, "loss", "losses"),
                       {},
                       "Largest number of consecutive losing trades.",
                       maximum_losing_streak > 0 ? ui::MetricTone::Negative
                                                 : ui::MetricTone::Neutral},
     ui::SummaryMetric{"Average winner",
                       format_currency(average_profit),
                       format_optional_percent(
                        percentage_of(average_profit, average_investment)),
                       "Average P&L among profitable trades.",
                       ui::MetricTone::Positive},
     ui::SummaryMetric{
      "Average loser",
      format_currency(average_loss),
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
      format_currency(cumulative_investment),
      format_optional_multiple(
       division_of(cumulative_investment, initial_capital)),
      "Capital committed across all closed trades; the multiple is relative "
      "to initial capital.",
      {}},
     ui::SummaryMetric{"Total trade duration",
                       format_duration(cumulative_duration),
                       {},
                       "Combined duration of all closed trades.",
                       {}},
     ui::SummaryMetric{"Average trade duration",
                       format_duration(trade_count > 0
                                        ? cumulative_duration /
                                           static_cast<std::time_t>(trade_count)
                                        : std::time_t{}),
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
                       format_currency(portfolio_row.capital),
                       format_optional_percent(net_return),
                       "Realized balance after closed trades.",
                       outcome_tone(net_pnl)},
     ui::SummaryMetric{
      "Equity",
      format_currency(portfolio_row.equity),
      format_optional_percent(
       percentage_of(portfolio_row.equity - initial_capital, initial_capital)),
      "Current balance including unrealized P&L.",
      outcome_tone(portfolio_row.equity - initial_capital)},
     ui::SummaryMetric{
      "Peak equity",
      format_currency(portfolio_row.peak_equity),
      format_optional_percent(percentage_of(
       portfolio_row.peak_equity - initial_capital, initial_capital)),
      "Highest equity reached during the backtest.",
      ui::MetricTone::Positive},
     ui::SummaryMetric{"Current drawdown",
                       format_percent(portfolio_row.drawdown),
                       {},
                       "Current decline from peak equity.",
                       portfolio_row.drawdown > 0.0 ? ui::MetricTone::Warning
                                                    : ui::MetricTone::Neutral},
     ui::SummaryMetric{"Maximum drawdown",
                       format_percent(portfolio_row.max_drawdown),
                       {},
                       "Largest peak-to-trough decline during the backtest.",
                       portfolio_row.max_drawdown > 0.0
                        ? ui::MetricTone::Warning
                        : ui::MetricTone::Neutral}};
    ui::summary_metric_section("##capital_risk",
                               PLUDUX_ICON_CAPITAL,
                               "Capital & risk",
                               "Balance, equity, and downside exposure.",
                               capital_metrics);
    ImGui::Spacing();

    open_investment = portfolio_row.reserved_notional;
    open_pnl = portfolio_row.unrealized_pnl;
    const auto equity = portfolio_row.equity;
    const auto exposure_metrics = std::array{
     ui::SummaryMetric{"Open trades",
                       std::to_string(open_trade_count),
                       {},
                       "Trades that remain open at the latest timeline result.",
                       open_trade_count > 0 ? ui::MetricTone::Warning
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
      "Open trade duration", format_duration(open_duration), {}, {}, {}}};
    ui::summary_metric_section("##open_exposure",
                               PLUDUX_ICON_EXPOSURE,
                               "Open exposure",
                               open_trade_count > 0
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
    return std::isfinite(value) ? std::format("{:.2f}%", value) : "â€”";
  }

  static auto format_optional_percent(std::optional<double> value)
   -> std::string
  {
    return value ? format_percent(*value) : "â€”";
  }

  static auto format_ratio(double value) -> std::string
  {
    if(std::isinf(value) && value > 0.0) {
      return "âˆž";
    }
    return std::isfinite(value) ? std::format("{:.2f}", value) : "â€”";
  }

  static auto format_optional_ratio(std::optional<double> value) -> std::string
  {
    return value ? format_ratio(*value) : "â€”";
  }

  static auto format_optional_multiple(std::optional<double> value)
   -> std::string
  {
    return value ? std::format("{}x", format_ratio(*value)) : "â€”";
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
