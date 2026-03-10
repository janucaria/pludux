module;

#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <jsoncons/json.hpp>

export module pludux.backtest:strategy;

import pludux;

import :trade_entry;
import :trade_exit;

export namespace pludux::backtest {

class Strategy {
public:
  Strategy()
  : Strategy("",
             SeriesMethodRegistry{},
             FalseMethod{},
             FalseMethod{},
             FalseMethod{},
             FalseMethod{},
             false,
             false,
             false,
             1.0)
  {
  }

  Strategy(std::string name,
           SeriesMethodRegistry series_registry,
           AnyConditionMethod long_entry_filter,
           AnyConditionMethod long_exit_filter,
           AnyConditionMethod short_entry_filter,
           AnyConditionMethod short_exit_filter,
           bool stop_loss_enabled,
           bool stop_loss_trailing_enabled,
           bool take_profit_enabled,
           double take_profit_r_multiple)
  : name_{std::move(name)}
  , series_registry_{series_registry}
  , long_entry_filter_{std::move(long_entry_filter)}
  , long_exit_filter_{std::move(long_exit_filter)}
  , short_entry_filter_{std::move(short_entry_filter)}
  , short_exit_filter_{std::move(short_exit_filter)}
  , stop_loss_enabled_{stop_loss_enabled}
  , stop_loss_trailing_enabled_{stop_loss_trailing_enabled}
  , take_profit_enabled_{take_profit_enabled}
  , take_profit_r_multiple_{take_profit_r_multiple}
  {
  }

  auto operator==(const Strategy&) const noexcept -> bool = default;

  auto name(this const Strategy& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Strategy& self, std::string name) noexcept
  {
    self.name_ = std::move(name);
  }

  auto series_registry(this const Strategy& self) noexcept
   -> const SeriesMethodRegistry&
  {
    return self.series_registry_;
  }

  auto series_registry(this Strategy& self) noexcept -> SeriesMethodRegistry&
  {
    return self.series_registry_;
  }

  auto long_entry_filter(this const Strategy& self) noexcept
   -> const AnyConditionMethod&
  {
    return self.long_entry_filter_;
  }

  void long_entry_filter(this Strategy& self,
                         AnyConditionMethod long_entry_filter) noexcept
  {
    self.long_entry_filter_ = std::move(long_entry_filter);
  }

  auto long_exit_filter(this const Strategy& self) noexcept
   -> const AnyConditionMethod&
  {
    return self.long_exit_filter_;
  }

  void long_exit_filter(this Strategy& self,
                        AnyConditionMethod long_exit_filter) noexcept
  {
    self.long_exit_filter_ = std::move(long_exit_filter);
  }

  auto short_entry_filter(this const Strategy& self) noexcept
   -> const AnyConditionMethod&
  {
    return self.short_entry_filter_;
  }

  void short_entry_filter(this Strategy& self,
                          AnyConditionMethod short_entry_filter) noexcept
  {
    self.short_entry_filter_ = std::move(short_entry_filter);
  }

  auto short_exit_filter(this const Strategy& self) noexcept
   -> const AnyConditionMethod&
  {
    return self.short_exit_filter_;
  }

  void short_exit_filter(this Strategy& self,
                         AnyConditionMethod short_exit_filter) noexcept
  {
    self.short_exit_filter_ = std::move(short_exit_filter);
  }

  auto stop_loss_enabled(this const Strategy& self) noexcept -> bool
  {
    return self.stop_loss_enabled_;
  }

  void stop_loss_enabled(this Strategy& self, bool stop_loss_enabled) noexcept
  {
    self.stop_loss_enabled_ = stop_loss_enabled;
  }

  auto stop_loss_trailing_enabled(this const Strategy& self) noexcept -> bool
  {
    return self.stop_loss_trailing_enabled_;
  }

  void stop_loss_trailing_enabled(this Strategy& self,
                                  bool stop_loss_trailing_enabled) noexcept
  {
    self.stop_loss_trailing_enabled_ = stop_loss_trailing_enabled;
  }

  auto take_profit_enabled(this const Strategy& self) noexcept -> bool
  {
    return self.take_profit_enabled_;
  }

  void take_profit_enabled(this Strategy& self,
                           bool take_profit_enabled) noexcept
  {
    self.take_profit_enabled_ = take_profit_enabled;
  }

  auto take_profit_r_multiple(this const Strategy& self) noexcept -> double
  {
    return self.take_profit_r_multiple_;
  }

  void take_profit_r_multiple(this Strategy& self,
                              double take_profit_r_multiple) noexcept
  {
    self.take_profit_r_multiple_ = take_profit_r_multiple;
  }

  auto equal_rules(this const Strategy& self, const Strategy& other) noexcept
   -> bool
  {
    return self.series_registry_ == other.series_registry_ &&
           self.long_entry_filter_ == other.long_entry_filter_ &&
           self.long_exit_filter_ == other.long_exit_filter_ &&
           self.short_entry_filter_ == other.short_entry_filter_ &&
           self.short_exit_filter_ == other.short_exit_filter_ &&
           self.stop_loss_enabled_ == other.stop_loss_enabled_ &&
           self.stop_loss_trailing_enabled_ ==
            other.stop_loss_trailing_enabled_ &&
           self.take_profit_enabled_ == other.take_profit_enabled_ &&
           self.take_profit_r_multiple_ == other.take_profit_r_multiple_;
  }

private:
  std::string name_;

  SeriesMethodRegistry series_registry_;

  AnyConditionMethod long_entry_filter_{FalseMethod{}};
  AnyConditionMethod long_exit_filter_{FalseMethod{}};

  AnyConditionMethod short_entry_filter_{FalseMethod{}};
  AnyConditionMethod short_exit_filter_{FalseMethod{}};

  bool stop_loss_enabled_{false};
  bool stop_loss_trailing_enabled_{false};

  bool take_profit_enabled_{false};
  double take_profit_r_multiple_{1.0};
};

auto parse_backtest_strategy_json(std::string_view strategy_name,
                                  std::istream& json_strategy_stream)
 -> backtest::Strategy
{
  auto config_parser = make_default_registered_config_parser();

  auto strategy_json = jsoncons::ojson::parse(
   json_strategy_stream, jsoncons::json_options{}.allow_comments(true));

  if(!strategy_json.is_object()) {
    throw std::runtime_error(
     "Invalid strategy JSON: expected an object at the root");
  }

  if(strategy_json.contains("version")) {
    const auto version = strategy_json.at("version").as<int>();
    if(version != 2) {
      throw std::runtime_error("Unsupported strategy JSON version: " +
                               std::to_string(version));
    }
  }

  auto series_registry =
   strategy_json.contains("series")
    ? config_parser.parse_registered_methods(strategy_json.at("series"))
    : SeriesMethodRegistry{};

  auto long_entry_filter = AnyConditionMethod{FalseMethod{}};
  auto long_exit_filter = AnyConditionMethod{FalseMethod{}};
  if(strategy_json.contains("longPosition")) {
    const auto& long_position_json = strategy_json.at("longPosition");

    if(long_position_json.contains("entry")) {
      const auto& entry_json = long_position_json.at("entry");
      long_entry_filter = config_parser.parse_filter(entry_json.at("signal"));
    }
    if(long_position_json.contains("exit")) {
      const auto& exit_json = long_position_json.at("exit");
      long_exit_filter = config_parser.parse_filter(exit_json.at("signal"));
    }
  }

  auto short_entry_filter = AnyConditionMethod{FalseMethod{}};
  auto short_exit_filter = AnyConditionMethod{FalseMethod{}};
  if(strategy_json.contains("shortPosition")) {
    const auto& short_position_json = strategy_json.at("shortPosition");

    if(short_position_json.contains("entry")) {
      const auto& entry_json = short_position_json.at("entry");
      short_entry_filter = config_parser.parse_filter(entry_json.at("signal"));
    }
    if(short_position_json.contains("exit")) {
      const auto& exit_json = short_position_json.at("exit");
      short_exit_filter = config_parser.parse_filter(exit_json.at("signal"));
    }
  }

  auto is_take_profit_enabled = false;
  auto take_profit_r_multiple = 1.0;
  if(strategy_json.contains("takeProfit")) {
    const auto take_profit_config = strategy_json.at("takeProfit");
    if(take_profit_config.is_bool()) {
      is_take_profit_enabled = take_profit_config.as_bool();
    } else if(take_profit_config.is_object()) {
      is_take_profit_enabled =
       take_profit_config.get_value_or<bool>("enabled", true);
      take_profit_r_multiple =
       take_profit_config.get_value_or<double>("rMultiple", 1.0);
    } else {
      throw std::runtime_error(
       "Invalid take profit configuration in strategy JSON");
    }
  }

  auto is_trailing_stop_loss = false;
  auto is_stop_loss_enabled = false;
  if(strategy_json.contains("stopLoss")) {
    const auto stop_loss_config = strategy_json.at("stopLoss");
    if(stop_loss_config.is_bool()) {
      is_stop_loss_enabled = stop_loss_config.as_bool();
    } else if(stop_loss_config.is_object()) {
      is_trailing_stop_loss =
       stop_loss_config.get_value_or<bool>("trailing", false);
      is_stop_loss_enabled =
       stop_loss_config.get_value_or<bool>("enabled", true);
    } else {
      throw std::runtime_error(
       "Invalid stop loss configuration in strategy JSON");
    }
  }

  return pludux::backtest::Strategy{std::string{strategy_name},
                                    std::move(series_registry),
                                    std::move(long_entry_filter),
                                    std::move(long_exit_filter),
                                    std::move(short_entry_filter),
                                    std::move(short_exit_filter),
                                    is_stop_loss_enabled,
                                    is_trailing_stop_loss,
                                    is_take_profit_enabled,
                                    take_profit_r_multiple};
}

auto parse_backtest_strategy_json(std::string_view strategy_name,
                                  const std::string& json_strategy_str)
 -> backtest::Strategy
{
  auto json_strategy_stream = std::istringstream{json_strategy_str};
  return parse_backtest_strategy_json(strategy_name, json_strategy_stream);
}

auto stringify_backtest_strategy(const backtest::Strategy& strategy)
 -> jsoncons::ojson
{
  auto config_parser = make_default_registered_config_parser();

  auto strategy_json = jsoncons::ojson{};

  strategy_json["version"] = 2;

  strategy_json["series"] =
   config_parser.serialize_registered_methods(strategy.series_registry());

  auto long_position_json = jsoncons::ojson{};
  long_position_json["entry"] = jsoncons::ojson{};
  long_position_json["entry"]["signal"] =
   config_parser.serialize_filter(strategy.long_entry_filter());
  long_position_json["exit"] = jsoncons::ojson{};
  long_position_json["exit"]["signal"] =
   config_parser.serialize_filter(strategy.long_exit_filter());
  strategy_json["longPosition"] = std::move(long_position_json);

  auto short_position_json = jsoncons::ojson{};
  short_position_json["entry"] = jsoncons::ojson{};
  short_position_json["entry"]["signal"] =
   config_parser.serialize_filter(strategy.short_entry_filter());
  short_position_json["exit"] = jsoncons::ojson{};
  short_position_json["exit"]["signal"] =
   config_parser.serialize_filter(strategy.short_exit_filter());
  strategy_json["shortPosition"] = std::move(short_position_json);

  strategy_json["stopLoss"] = jsoncons::ojson{};
  strategy_json["stopLoss"]["enabled"] = strategy.stop_loss_enabled();
  strategy_json["stopLoss"]["trailing"] = strategy.stop_loss_trailing_enabled();

  strategy_json["takeProfit"] = jsoncons::ojson{};
  strategy_json["takeProfit"]["enabled"] = strategy.take_profit_enabled();
  strategy_json["takeProfit"]["rMultiple"] = strategy.take_profit_r_multiple();

  return strategy_json;
}

} // namespace pludux::backtest
