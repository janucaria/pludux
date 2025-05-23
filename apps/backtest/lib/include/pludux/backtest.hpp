#ifndef PLUDUX_PLUDUX_BACKTEST_HPP
#define PLUDUX_PLUDUX_BACKTEST_HPP

#include <istream>
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
#include <pludux/quote_access.hpp>
#include <pludux/screener.hpp>

namespace pludux {

class Backtest {
public:
  Backtest(const backtest::Strategy& strategy,
           const backtest::Asset& asset,
           const QuoteAccess& quote_access);

  auto strategy() const noexcept -> const backtest::Strategy&;

  auto asset() const noexcept -> const backtest::Asset&;

  auto capital_risk() const noexcept -> double;

  auto trade_records() const noexcept
   -> const std::vector<backtest::TradeRecord>&;

  void reset();

  auto should_run() const noexcept -> bool;

  void run();

private:
  const backtest::Strategy& strategy_;
  const backtest::Asset& asset_;
  const QuoteAccess& quote_access_;

  std::optional<backtest::TradingSession> trading_session_;
  std::size_t current_index_;
  std::vector<backtest::TradeRecord> trade_records_;
};

auto get_env_var(std::string_view var_name) -> std::optional<std::string>;

auto parse_backtest_strategy_json(std::istream& json_strategy_stream,
                                  const QuoteAccess& quote_access)
 -> backtest::Strategy;

auto csv_daily_stock_data(std::istream& csv_stream)
 -> std::vector<std::pair<std::string, pludux::DataSeries<double>>>;

auto risk_reward_config_parser(QuoteAccess QuoteAccess) -> ConfigParser;

} // namespace pludux

#endif
