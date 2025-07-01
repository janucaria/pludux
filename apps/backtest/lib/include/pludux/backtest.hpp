#ifndef PLUDUX_PLUDUX_BACKTEST_HPP
#define PLUDUX_PLUDUX_BACKTEST_HPP

#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <pludux/asset_history.hpp>
#include <pludux/backtest/asset.hpp>
#include <pludux/backtest/stop_loss.hpp>
#include <pludux/backtest/strategy.hpp>
#include <pludux/backtest/take_profit.hpp>
#include <pludux/backtest/trade_record.hpp>
#include <pludux/backtest/trading_session.hpp>
#include <pludux/backtest/trading_stop_loss.hpp>
#include <pludux/backtest/trading_take_profit.hpp>
#include <pludux/config_parser.hpp>
#include <pludux/screener.hpp>

namespace pludux {

class Backtest {
public:
  Backtest(std::string name,
           std::shared_ptr<backtest::Strategy> strategy_ptr,
           std::shared_ptr<backtest::Asset> asset_ptr);

  auto name() const noexcept -> const std::string&;

  auto strategy_ptr() const noexcept
   -> const std::shared_ptr<backtest::Strategy>;

  auto asset_ptr() const noexcept -> const std::shared_ptr<backtest::Asset>;

  auto strategy() const noexcept -> const backtest::Strategy&;

  auto asset() const noexcept -> const backtest::Asset&;

  void mark_as_failed() noexcept;

  auto is_failed() const noexcept -> bool;

  auto capital_risk() const noexcept -> double;

  auto trade_records() const noexcept
   -> const std::vector<backtest::TradeRecord>&;

  void reset();

  auto should_run() const noexcept -> bool;

  void run();

private:
  std::string name_;

  std::shared_ptr<backtest::Strategy> strategy_ptr_;
  std::shared_ptr<backtest::Asset> asset_ptr_;

  bool is_failed_;
  std::optional<backtest::TradingSession> trading_session_;
  std::size_t current_index_;
  std::vector<backtest::TradeRecord> trade_records_;
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

} // namespace pludux

#endif
