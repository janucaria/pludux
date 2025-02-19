#ifndef PLUDUX_PLUDUX_BACKTEST_HPP
#define PLUDUX_PLUDUX_BACKTEST_HPP

#include <istream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <pludux/asset_history.hpp>
#include <pludux/backtest/backtesting_summary.hpp>
#include <pludux/backtest/stop_loss.hpp>
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
  Backtest(double capital_risk_,
           QuoteAccess quote_access,
           screener::ScreenerFilter entry_filter,
           screener::ScreenerFilter exit_filter,
           backtest::StopLoss stop_loss,
           backtest::TakeProfit take_profit);

  auto capital_risk() const noexcept -> double;

  auto quote_access() const noexcept -> const QuoteAccess&;

  auto backtesting_summary() const noexcept
   -> const backtest::BacktestingSummary&;

  void reset();

  auto run(const AssetHistory& asset) -> bool;

private:
  double capital_risk_;
  QuoteAccess quote_access_;
  screener::ScreenerFilter entry_filter_;
  screener::ScreenerFilter exit_filter_;
  backtest::StopLoss stop_loss_;
  backtest::TakeProfit take_profit_;

  std::optional<backtest::TradingSession> trading_session_;
  std::size_t current_index_;
  std::size_t asset_size_;
  backtest::BacktestingSummary backtesting_summary_;
};

auto get_env_var(std::string_view var_name) -> std::optional<std::string>;

auto parse_backtest_strategy_json(std::istream& json_strategy_stream)
 -> Backtest;

auto csv_daily_stock_data(std::istream& csv_stream)
 -> std::vector<std::pair<std::string, pludux::DataSeries<double>>>;

auto risk_reward_config_parser(QuoteAccess QuoteAccess) -> ConfigParser;

} // namespace pludux

#endif
