#include <algorithm>
#include <chrono>
#include <cmath>
#include <format>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include <pludux/asset_history.hpp>
#include <pludux/screener.hpp>

#include <nlohmann/json.hpp>

#include <pludux/backtest.hpp>
#include <pludux/backtest/backtesting_summary.hpp>

auto main(int, const char**) -> int
{
  using json = nlohmann::json;

  const auto json_strategy_path =
   pludux::get_env_var("PLUDUX_BACKTEST_STRATEGY_JSON_PATH").value_or("");
  const auto asset_file =
   pludux::get_env_var("PLUDUX_BACKTEST_CSV_DATA_PATH").value_or("");

  auto json_strategy_file = std::ifstream{json_strategy_path};
  auto strategy =
   pludux::parse_backtest_strategy_json("Strategy", json_strategy_file);
  auto strategy_ptr = std::make_shared<pludux::backtest::Strategy>(strategy);

  auto csv_stream = std::ifstream{asset_file};

  if(!csv_stream.is_open()) {
    std::cerr << "Could not open file: " << asset_file << std::endl;
    return 1;
  }

  const auto quotes = pludux::csv_daily_stock_data(csv_stream);

  auto asset_history = pludux::AssetHistory(quotes.begin(), quotes.end());
  auto asset_ptr = std::make_shared<pludux::backtest::Asset>(
   asset_file, std::move(asset_history));

  auto backtest = pludux::Backtest{"no name", strategy_ptr, asset_ptr};
  while(backtest.should_run()) {
    backtest.run();
  }

  const auto& backtest_summaries = backtest.summaries();
  const auto& summary = !backtest_summaries.empty()
                         ? backtest_summaries.back()
                         : pludux::backtest::BacktestingSummary{};

  auto& ostream = std::cout;

  ostream << std::format("Risk per trade: {:.2f}\n", backtest.capital_risk());
  ostream << std::format("Total profit: {:.2f}\n", summary.cumulative_pnls());

  const auto total_duration =
   pludux::format_duration(summary.cumulative_durations());
  ostream << std::format("Total duration: {}\n", total_duration);
  ostream << std::format("Total trades: {}\n",
                         summary.trade_count() + summary.open_trade_count());

  ostream << std::format("Average profit: {:.2f}\n", summary.average_profit());
  ostream << std::format("Average loss: {:.2f}\n", -summary.average_loss());

  const auto average_duration =
   pludux::format_duration(summary.average_duration());
  ostream << std::format("Average duration: {}\n", average_duration);

  ostream << "\n\n";
  ostream << "OPEN TRADE\n";
  ostream << "----------\n";
  ostream << std::format("Unrealized profit: {:.2f}\n",
                         summary.unrealized_pnl());

  const auto ongoing_trade_duration =
   pludux::format_duration(summary.ongoing_trade_duration());
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

  ostream << std::format("Total closed trades: {}\n", summary.trade_count());
  ostream << std::format("Win rate: {:.2f}%\n", summary.profit_rate() * 100);
  ostream << std::format("Loss rate: {:.2f}%\n", summary.loss_rate() * 100);
  ostream << std::format("Break even rate: {:.2f}%\n",
                         summary.break_even_rate() * 100);
  ostream << "\n\n";

  // iterate through the trades
  std::cout << "Trades: " << std::endl;
  // auto is_in_trade = false;

  for(int i = 0, ii = backtest_summaries.size(); i < ii; ++i) {
    const auto& trade = backtest_summaries[i].trade_record();

    if(trade.is_summary_session()) {
      const auto entry_timestamp = trade.entry_timestamp();
      const auto exit_timestamp = trade.exit_timestamp();
      std::cout << "Entry date: " << pludux::format_datetime(entry_timestamp)
                << "Exit date: "
                << (trade.is_open() ? "N/A"
                                    : pludux::format_datetime(exit_timestamp))
                << "Position size: " << trade.position_size() << std::endl
                << "Reason: " << static_cast<int>(trade.status()) << std::endl
                << "Profit: " << trade.pnl() << std::endl
                << std::endl;
    }
  }

  return 0;
}
