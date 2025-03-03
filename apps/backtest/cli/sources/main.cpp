#include <algorithm>
#include <chrono>
#include <cmath>
#include <format>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include <pludux/asset_history.hpp>
#include <pludux/asset_quote.hpp>
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

  const auto quote_access = pludux::QuoteAccess{};

  auto json_strategy_file = std::ifstream{json_strategy_path};
  auto backtest = pludux::parse_backtest_strategy_json(json_strategy_file);

  auto csv_stream = std::ifstream{asset_file};

  if(!csv_stream.is_open()) {
    std::cerr << "Could not open file: " << asset_file << std::endl;
    return 1;
  }

  const auto quotes = pludux::csv_daily_stock_data(csv_stream);

  auto asset = pludux::AssetHistory(quotes.begin(), quotes.end());
  auto open_trade = std::optional<pludux::backtest::TradeRecord>{};
  auto closed_trades = std::vector<pludux::backtest::TradeRecord>{};

  while(backtest.should_run(asset)) {
    backtest.run(asset);
  }

  auto& ostream = std::cout;

  auto summary = pludux::backtest::BacktestingSummary{};
  const auto& history = backtest.history();
  for(int i = 0, ii = history.size(); i < ii; ++i) {
    const auto& trade = history[i].trade_record();
    const auto is_last_trade = i == ii - 1;

    if(trade.has_value() &&
       (trade->is_closed() || is_last_trade && trade->is_open())) {
      summary.add_trade(*trade);
    }
  }

  ostream << std::format("Risk per trade: {:.2f}\n", backtest.capital_risk());
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
  ostream << std::format("Average duration: {} days\n", average_duration_days);

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
                         summary.expected_value() / backtest.capital_risk() *
                          100);

  ostream << std::format("Total closed trades: {}\n",
                         summary.closed_trades().size());
  ostream << std::format("Win rate: {:.2f}%\n", summary.win_rate() * 100);
  ostream << std::format("Loss rate: {:.2f}%\n", summary.loss_rate() * 100);
  ostream << std::format("Break even rate: {:.2f}%\n",
                         summary.break_even_rate() * 100);
  ostream << "\n\n";

  // iterate through the trades
  std::cout << "Trades: " << std::endl;
  const auto& backtest_history = backtest.history();
  // auto is_in_trade = false;
  for(int i = 0, ii = backtest_history.size(); i < ii; ++i) {
    const auto& trade = backtest_history[i].trade_record();
    const auto is_last_trade = i == ii - 1;

    if(trade && trade->is_closed() || is_last_trade) {
      const auto entry_timestamp = trade->entry_timestamp();
      const auto exit_timestamp = trade->exit_timestamp();
      std::cout << "Entry date: " << std::ctime(&entry_timestamp) // NOLINT
                << "Exit date: " << std::ctime(&exit_timestamp)   // NOLINT
                << "Position size: " << trade->position_size() << std::endl
                << "Reason: " << static_cast<int>(trade->status()) << std::endl
                << "Profit: " << trade->profit() << std::endl
                << std::endl;
    }
  }

  return 0;
}
