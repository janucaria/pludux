#include <algorithm>
#include <chrono>
#include <cmath>
#include <format>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include <jsoncons/json.hpp>

import pludux.backtest;

auto main(int, const char**) -> int
{
  using json = jsoncons::ojson;

  const auto json_strategy_path =
   pludux::get_env_var("PLUDUX_BACKTEST_STRATEGY_JSON_PATH").value_or("");
  const auto asset_file =
   pludux::get_env_var("PLUDUX_BACKTEST_CSV_DATA_PATH").value_or("");

  auto json_strategy_file = std::ifstream{json_strategy_path};
  auto strategy = pludux::backtest::parse_backtest_strategy_json(
   "Strategy", json_strategy_file);

  auto csv_stream = std::ifstream{asset_file};

  if(!csv_stream.is_open()) {
    std::cerr << "Could not open file: " << asset_file << std::endl;
    return 1;
  }

  auto store = pludux::backtest::Store{};

  auto asset = pludux::backtest::Asset{asset_file};
  pludux::update_asset_from_csv(asset, csv_stream);

  auto broker = pludux::backtest::Broker{"Default"};

  auto market = pludux::backtest::Market{"Default"};

  auto profile = pludux::backtest::Profile{"Default"};
  profile.capital_risk(0.01);

  const auto asset_handle = store.add_asset(std::move(asset)).value();
  const auto strategy_handle = store.add_strategy(std::move(strategy)).value();
  const auto market_handle = store.add_market(std::move(market)).value();
  const auto broker_handle = store.add_broker(std::move(broker)).value();
  const auto profile_handle = store.add_profile(std::move(profile)).value();

  const auto intial_capital = 1'000'000;

  auto backtest = pludux::backtest::Backtest{"no name",
                                             intial_capital,
                                             asset_handle,
                                             strategy_handle,
                                             market_handle,
                                             broker_handle,
                                             profile_handle};

  auto backtest_handle = store.add_backtest(std::move(backtest)).value();

  backtest = store.get_backtest(backtest_handle);
  asset = store.get_asset(backtest.asset_handle());
  strategy = store.get_strategy(backtest.strategy_handle());
  market = store.get_market(backtest.market_handle());
  broker = store.get_broker(backtest.broker_handle());
  profile = store.get_profile(backtest.profile_handle());

  auto& backtest_summaries = store.get_backtest_summaries(backtest_handle);
  auto& series_results = store.get_series_results(backtest_handle);

  auto backtest_runner =
   pludux::backtest::BacktestRunner{backtest.initial_capital(),
                                    asset,
                                    strategy,
                                    market,
                                    broker,
                                    profile,
                                    backtest_summaries,
                                    series_results};

  while(!backtest.is_failed() && backtest_summaries.size() < asset.size()) {
    backtest_runner.run();
  }

  const auto& summary = !backtest_summaries.empty()
                         ? backtest_summaries.back()
                         : pludux::backtest::BacktestSummary{};

  const auto risk_value = profile.capital_risk() * backtest.initial_capital();

  auto& ostream = std::cout;

  ostream << std::format("Risk per trade: {:.2f}\n", risk_value);
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

  const auto unrealized_duration =
   pludux::format_duration(summary.unrealized_duration());
  ostream << std::format("Ongoing duration: {}\n", unrealized_duration);

  ostream << "\n\n";
  ostream << "CLOSED TRADES\n";
  ostream << "-------------\n";
  ostream << std::format("Expected value (EV): {:.2f}\n",
                         summary.expected_value());
  ostream << std::format("EV to risk rate: {:.2f}%\n",
                         summary.expected_value() / risk_value * 100);

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
    const auto& session = backtest_summaries[i].trade_session();
    for(const auto& record : session.trade_record_range()) {
      if(!record.is_open() || i == ii - 1) {
        const auto entry_timestamp = record.entry_timestamp();
        const auto exit_timestamp = record.exit_timestamp();
        std::cout << "Entry date: " << pludux::format_datetime(entry_timestamp)
                  << std::endl
                  << "Exit date: "
                  << (record.is_open()
                       ? "N/A"
                       : pludux::format_datetime(exit_timestamp))
                  << std::endl
                  << "Position size: " << record.position_size() << std::endl
                  << "Reason: " << static_cast<int>(record.status())
                  << std::endl
                  << "Profit: " << record.pnl() << std::endl
                  << std::endl;
      }
    }
  }

  return 0;
}
