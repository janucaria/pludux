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

  while(backtest.run(asset)) {
    // Nothing to do here
  }

  auto result = backtest.backtesting_summary();

  std::cout << "Asset: " << asset_file << std::endl;
  std::cout << "Total profit: " << result.total_profit() << std::endl;
  std::cout << "Total trades: " << result.closed_trades().size() << std::endl;
  std::cout << "Average profit: " << result.average_win() << std::endl;
  std::cout << "Average loss: " << result.average_loss() << std::endl;

  std::cout << std::endl;
  std::cout << "exit signal rate: " << result.exit_signal_rate() << std::endl;
  std::cout << "Average exit signal: " << result.average_exit_signal()
            << std::endl;
  std::cout << "exit signal ev: " << result.exit_signal_expected_value()
            << std::endl;

  std::cout << "take profit rate: " << result.take_profit_rate() << std::endl;
  std::cout << "Average take profit: " << result.average_take_profit()
            << std::endl;
  std::cout << "take profit ev: " << result.take_profit_expected_value()
            << std::endl;

  std::cout << "stop loss rate: " << result.stop_loss_rate() << std::endl;
  std::cout << "Average stop loss: " << result.average_stop_loss() << std::endl;
  std::cout << "stop loss ev: " << result.stop_loss_expected_value()
            << std::endl;
  std::cout << std::endl;

  std::cout << "Expected value: " << result.expected_value() << std::endl;

  // get the duration in days
  std::cout << "Total duration: " << result.total_duration() / (60 * 60 * 24)
            << " days" << std::endl;
  std::cout << "Average duration: "
            << result.average_duration() / (60 * 60 * 24) << " days"
            << std::endl;

  std::cout << "Win rate: " << result.win_rate() << std::endl;
  std::cout << "Loss Rate: " << result.loss_rate() << std::endl;
  std::cout << "BE Rate: " << result.break_even_rate() << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;

  // iterate through the trades
  std::cout << "Trade: " << std::endl;
  for(const pludux::backtest::TradeRecord& trade : result.closed_trades()) {
    const auto entry_timestamp = trade.entry_timestamp();
    const auto exit_timestamp = trade.exit_timestamp();
    std::cout << "Entry date: " << std::ctime(&entry_timestamp) // NOLINT
              << "Exit date: " << std::ctime(&exit_timestamp)   // NOLINT
              << "Reason: " << static_cast<int>(trade.status()) << std::endl
              << "Profit: " << trade.profit() << std::endl
              << std::endl;
  }

  return 0;
}
