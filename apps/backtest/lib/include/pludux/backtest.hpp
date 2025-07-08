#ifndef PLUDUX_PLUDUX_BACKTEST_HPP
#define PLUDUX_PLUDUX_BACKTEST_HPP

import pludux.config_parser;

#include <istream>
#include <memory>
#include <string>
#include <vector>

#include <pludux/backtest/asset.hpp>
#include <pludux/backtest/backtesting_summary.hpp>
#include <pludux/backtest/strategy.hpp>

namespace pludux {

class Backtest {
public:
  Backtest(std::string name,
           std::shared_ptr<backtest::Strategy> strategy_ptr,
           std::shared_ptr<backtest::Asset> asset_ptr);

  Backtest(std::string name,
           std::shared_ptr<backtest::Strategy> strategy_ptr,
           std::shared_ptr<backtest::Asset> asset_ptr,
           std::vector<backtest::BacktestingSummary> summaries);

  auto name() const noexcept -> const std::string&;

  auto strategy_ptr() const noexcept
   -> const std::shared_ptr<backtest::Strategy>;

  auto asset_ptr() const noexcept -> const std::shared_ptr<backtest::Asset>;

  auto strategy() const noexcept -> const backtest::Strategy&;

  auto asset() const noexcept -> const backtest::Asset&;

  void mark_as_failed() noexcept;

  auto is_failed() const noexcept -> bool;

  auto capital_risk() const noexcept -> double;

  auto summaries() const noexcept
   -> const std::vector<backtest::BacktestingSummary>&;

  void reset();

  auto should_run() const noexcept -> bool;

  void run();

private:
  std::string name_;

  std::shared_ptr<backtest::Strategy> strategy_ptr_;
  std::shared_ptr<backtest::Asset> asset_ptr_;

  bool is_failed_;
  std::vector<backtest::BacktestingSummary> summaries_;
};

auto get_env_var(std::string_view var_name) -> std::optional<std::string>;

auto parse_backtest_strategy_json(const std::string& strategy_name,
                                  std::istream& json_strategy_stream)
 -> backtest::Strategy;

auto csv_daily_stock_data(std::istream& csv_stream)
 -> std::vector<std::pair<std::string, pludux::DataSeries<double>>>;

auto risk_reward_config_parser() -> ConfigParser;

auto format_duration(std::size_t duration_in_seconds) -> std::string;

auto format_datetime(std::time_t timestamp) -> std::string;

auto format_currency(double value) -> std::string;

} // namespace pludux

#endif
