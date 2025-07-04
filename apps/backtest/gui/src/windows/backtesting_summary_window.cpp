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

  auto ostream = std::stringstream{};
  if(!state.backtests.empty() && state.selected_backtest_index >= 0) {
    const auto& backtest = backtests[state.selected_backtest_index];

    const auto& backtest_name = backtest.name();
    const auto& asset_name = backtest.asset().name();
    const auto& strategy_name = backtest.strategy().name();

    ostream << "Backtest: " << backtest_name << std::endl;
    ostream << "Asset: " << asset_name << std::endl;
    ostream << "Strategy: " << strategy_name << std::endl;
    ostream << std::endl;

    auto summary = backtest::BacktestingSummary{};

    const auto& trade_records = backtest.trade_records();
    for(int i = 0, ii = trade_records.size(); i < ii; ++i) {
      const auto& trade = trade_records[i];

      if(trade.is_summary_session()) {
        summary.add_trade(trade);
      }
    }

    ostream << std::format("Risk per trade: {:.2f}\n", backtest.capital_risk());
    ostream << std::format("Total profit: {:.2f}\n", summary.total_profit());

    const auto total_duration = format_duration(summary.total_duration());
    ostream << std::format("Total duration: {}\n", total_duration);
    ostream << std::format("Total trades: {}\n", summary.total_trades());

    ostream << std::format("Average profit: {:.2f}\n", summary.average_win());
    ostream << std::format("Average loss: {:.2f}\n", -summary.average_loss());

    const auto average_duration = format_duration(summary.average_duration());
    ostream << std::format("Average duration: {}\n", average_duration);

    ostream << "\n\n";
    ostream << "OPEN TRADE\n";
    ostream << "----------\n";
    ostream << std::format("Unrealized profit: {:.2f}\n",
                           summary.unrealized_profit());

    const auto ongoing_trade_duration =
     format_duration(summary.ongoing_trade_duration());
    ostream << std::format("Ongoing duration: {}\n", ongoing_trade_duration);

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
                           summary.expected_value() / backtest.capital_risk() *
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
