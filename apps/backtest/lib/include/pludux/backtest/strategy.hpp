#ifndef PLUDUX_PLUDUX_BACKTEST_STRATEGY_HPP
#define PLUDUX_PLUDUX_BACKTEST_STRATEGY_HPP

#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <pludux/asset_snapshot.hpp>
#include <pludux/backtest/trade_entry.hpp>
#include <pludux/backtest/trade_exit.hpp>
#include <pludux/config_parser.hpp>
#include <pludux/screener.hpp>

namespace pludux::backtest {

class Strategy {
public:
  enum class EntryRepeat { sequence, always };

  enum class Direction { long_direction, short_direction };

  Strategy(std::string name,
           std::shared_ptr<screener::MethodRegistry> method_registry,
           screener::ScreenerMethod risk_method,
           screener::ScreenerFilter long_entry_filter,
           screener::ScreenerFilter long_exit_filter,
           screener::ScreenerFilter short_entry_filter,
           screener::ScreenerFilter short_exit_filter,
           bool stop_loss_enabled,
           bool stop_loss_trailing_enabled,
           double stop_loss_risk_multiplier,
           bool take_profit_enabled,
           double take_profit_risk_multiplier);

  auto name() const noexcept -> const std::string&;

  auto method_registry() const noexcept
   -> std::shared_ptr<screener::MethodRegistry>;

  auto risk_method() const noexcept -> const screener::ScreenerMethod&;

  auto long_entry_filter() const noexcept -> const screener::ScreenerFilter&;

  auto long_exit_filter() const noexcept -> const screener::ScreenerFilter&;

  auto short_entry_filter() const noexcept -> const screener::ScreenerFilter&;

  auto short_exit_filter() const noexcept -> const screener::ScreenerFilter&;

  auto stop_loss_enabled() const noexcept -> bool;

  auto stop_loss_trailing_enabled() const noexcept -> bool;

  auto stop_loss_risk_multiplier() const noexcept -> double;

  auto take_profit_enabled() const noexcept -> bool;

  auto take_profit_risk_multiplier() const noexcept -> double;

  auto entry_long_trade(const AssetSnapshot& asset_snapshot,
                        double risk_value) const noexcept
   -> std::optional<TradeEntry>;

  auto entry_short_trade(const AssetSnapshot& asset_snapshot,
                         double risk_value) const noexcept
   -> std::optional<TradeEntry>;

  auto entry_trade(const AssetSnapshot& asset_snapshot,
                   double risk_value) const noexcept
   -> std::optional<TradeEntry>;

  auto exit_trade(const AssetSnapshot& asset_snapshot,
                  double position_size) const noexcept
   -> std::optional<TradeExit>;

private:
  std::string name_;

  std::shared_ptr<screener::MethodRegistry> method_registry_;

  screener::ScreenerMethod risk_method_;

  screener::ScreenerFilter long_entry_filter_{screener::FalseFilter{}};
  screener::ScreenerFilter long_exit_filter_{screener::FalseFilter{}};

  screener::ScreenerFilter short_entry_filter_{screener::FalseFilter{}};
  screener::ScreenerFilter short_exit_filter_{screener::FalseFilter{}};

  bool stop_loss_enabled_{false};
  bool stop_loss_trailing_enabled_{false};
  double stop_loss_risk_multiplier_{1.0};

  bool take_profit_enabled_{false};
  double take_profit_risk_multiplier_{1.0};
};

auto parse_backtest_strategy_json(std::string_view strategy_name,
                                  std::istream& json_strategy_stream)
 -> backtest::Strategy;

auto risk_reward_config_parser() -> ConfigParser;

} // namespace pludux::backtest

#endif
