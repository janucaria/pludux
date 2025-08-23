#ifndef PLUDUX_PLUDUX_BACKTEST_STRATEGY_HPP
#define PLUDUX_PLUDUX_BACKTEST_STRATEGY_HPP

#include <istream>
#include <string>
#include <string_view>

#include <pludux/backtest/stop_loss.hpp>
#include <pludux/backtest/take_profit.hpp>
#include <pludux/config_parser.hpp>
#include <pludux/screener.hpp>

namespace pludux::backtest {

class Strategy {
public:
  enum class RepeatType { sequence, always };

  enum class Direction { long_direction, short_direction };

  Strategy(std::string name,
           Direction direction,
           RepeatType entry_repeat,
           screener::ScreenerFilter entry_filter,
           screener::ScreenerFilter exit_filter,
           backtest::StopLoss stop_loss,
           backtest::TakeProfit take_profit);

  auto name() const noexcept -> const std::string&;

  auto direction() const noexcept -> Direction;

  void direction(Direction dir) noexcept;

  auto entry_repeat() const noexcept -> RepeatType;

  void entry_repeat(RepeatType repeat_type) noexcept;

  auto entry_filter() const noexcept -> const screener::ScreenerFilter&;

  auto exit_filter() const noexcept -> const screener::ScreenerFilter&;

  auto stop_loss() const noexcept -> const backtest::StopLoss&;

  auto take_profit() const noexcept -> const backtest::TakeProfit&;

  auto is_entry_repeat_sequence() const noexcept -> bool;

  void set_entry_repeat_to_sequence() noexcept;

  auto is_entry_repeat_always() const noexcept -> bool;

  void set_entry_repeat_to_always() noexcept;

  auto is_long_direction() const noexcept -> bool;

  auto is_short_direction() const noexcept -> bool;

private:
  std::string name_;
  Direction direction_;
  RepeatType entry_repeat_;
  screener::ScreenerFilter entry_filter_;
  screener::ScreenerFilter exit_filter_;
  backtest::StopLoss stop_loss_;
  backtest::TakeProfit take_profit_;
};

auto parse_backtest_strategy_json(std::string_view strategy_name,
                                  std::istream& json_strategy_stream)
 -> backtest::Strategy;

auto risk_reward_config_parser() -> ConfigParser;

} // namespace pludux::backtest

#endif
