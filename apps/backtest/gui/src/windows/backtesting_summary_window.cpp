#include <chrono>
#include <format>
#include <iostream>
#include <sstream>

#include <imgui.h>

#include "../app_state.hpp"

#include "./backtesting_summary_window.hpp"

namespace pludux::apps {

BacktestSummaryWindow::BacktestSummaryWindow()
{
}

void BacktestSummaryWindow::render(AppState& app_state)
{
  auto& state = app_state.state();
  auto& backtest = state.backtest;

  ImGui::Begin("Summary", nullptr);

  auto ostream = std::stringstream{};

  ostream << "Strategy: " << state.strategy_name << std::endl;
  ostream << "Asset: " << state.asset_name << std::endl;
  ostream << std::endl;

  if(backtest.has_value() &&
     backtest->backtesting_summary().total_trades() > 0) {
    const auto& summary = backtest->backtesting_summary();
    ostream << std::format("Risk per trade: {:.2f}\n",
                           backtest->capital_risk());
    ostream << std::format("Total profit: {:.2f}\n", summary.total_profit());
    auto total_duration_days = std::chrono::duration_cast<std::chrono::days>(
                                std::chrono::seconds(summary.total_duration()))
                                .count();
    ostream << std::format("Total duration: {} days\n", total_duration_days);
    ostream << std::format("Total trades: {}\n", summary.total_trades());

    ostream << std::format("Average profit: {:.2f}\n", summary.average_win());
    ostream << std::format("Average loss: {:.2f}\n", -summary.average_loss());

    auto average_duration_days =
     std::chrono::duration_cast<std::chrono::days>(
      std::chrono::seconds(summary.average_duration()))
      .count();
    ostream << std::format("Average duration: {} days\n",
                           average_duration_days);

    ostream << "\n\n";
    ostream << "OPEN TRADE\n";
    ostream << "----------\n";
    ostream << std::format("Unrealized profit: {:.2f}\n",
                           summary.unrealized_profit());

    auto ongoing_trade_duration_days =
     std::chrono::duration_cast<std::chrono::days>(
      std::chrono::seconds(summary.ongoing_trade_duration()))
      .count();
    ostream << std::format("Ongoing duration: {} days\n",
                           ongoing_trade_duration_days);

    ostream << "\n\n";
    ostream << "CLOSED TRADES\n";
    ostream << "-------------\n";
    ostream << std::format("Exit signal rate: {:.2f}%\n",
                           summary.exit_signal_rate() * 100);
    ostream << std::format("Average exit signal: {:.2f}\n",
                           summary.average_exit_signal());
    ostream << std::format("Exit signal EV: {:.2f}\n",
                           summary.exit_signal_expected_value());

    ostream << std::format("Take profit rate: {:.2f}%\n",
                           summary.take_profit_rate() * 100);
    ostream << std::format("Average take profit: {:.2f}\n",
                           summary.average_take_profit());
    ostream << std::format("Take profit EV: {:.2f}\n",
                           summary.take_profit_expected_value());

    ostream << std::format("Stop loss rate: {:.2f}%\n",
                           summary.stop_loss_rate() * 100);
    ostream << std::format("Average stop loss: {:.2f}\n",
                           -summary.average_stop_loss());
    ostream << std::format("Stop loss EV: {:.2f}\n",
                           summary.stop_loss_expected_value());
    ostream << "\n";

    ostream << std::format("Expected value (EV): {:.2f}\n",
                           summary.expected_value());
    ostream << std::format("EV to risk rate: {:.2f}%\n",
                           summary.expected_value() / backtest->capital_risk() *
                            100);

    ostream << std::format("Total closed trades: {}\n",
                           summary.closed_trades().size());
    ostream << std::format("Win rate: {:.2f}%\n", summary.win_rate() * 100);
    ostream << std::format("Loss rate: {:.2f}%\n", summary.loss_rate() * 100);
    ostream << std::format("Break even rate: {:.2f}%\n",
                           summary.break_even_rate() * 100);
    ostream << "\n\n";
  }

  const auto str = ostream.str();
  ImGui::TextWrapped("%s", str.c_str());

  ImGui::End();
}

} // namespace pludux::apps
