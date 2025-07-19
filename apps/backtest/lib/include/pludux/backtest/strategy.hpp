#ifndef PLUDUX_PLUDUX_BACKTEST_STRATEGY_HPP
#define PLUDUX_PLUDUX_BACKTEST_STRATEGY_HPP

#include <string>
#include <unordered_map>

#include <pludux/backtest/stop_loss.hpp>
#include <pludux/backtest/take_profit.hpp>
#include <pludux/backtest/trade_record.hpp>
#include <pludux/screener.hpp>

namespace pludux::backtest {

class Strategy {
public:
  Strategy(std::string name,
           screener::ScreenerFilter entry_filter,
           screener::ScreenerFilter exit_filter,
           backtest::StopLoss stop_loss,
           backtest::TakeProfit take_profit);

  auto name() const noexcept -> const std::string&;

  auto entry_filter() const noexcept -> const screener::ScreenerFilter&;

  auto exit_filter() const noexcept -> const screener::ScreenerFilter&;

  auto stop_loss() const noexcept -> const backtest::StopLoss&;

  auto take_profit() const noexcept -> const backtest::TakeProfit&;

private:
  std::string name_;
  screener::ScreenerFilter entry_filter_;
  screener::ScreenerFilter exit_filter_;
  backtest::StopLoss stop_loss_;
  backtest::TakeProfit take_profit_;
};

} // namespace pludux::backtest

#endif
