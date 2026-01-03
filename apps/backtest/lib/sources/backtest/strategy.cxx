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
             PercentageMethod<>{10.0},
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
           AnySeriesMethod risk_method,
           AnyConditionMethod long_entry_filter,
           AnyConditionMethod long_exit_filter,
           AnyConditionMethod short_entry_filter,
           AnyConditionMethod short_exit_filter,
           bool stop_loss_enabled,
           bool stop_loss_trailing_enabled,
           bool take_profit_enabled,
           double take_profit_risk_multiplier)
  : name_{std::move(name)}
  , series_registry_{series_registry}
  , risk_method_{risk_method}
  , long_entry_filter_{std::move(long_entry_filter)}
  , long_exit_filter_{std::move(long_exit_filter)}
  , short_entry_filter_{std::move(short_entry_filter)}
  , short_exit_filter_{std::move(short_exit_filter)}
  , stop_loss_enabled_{stop_loss_enabled}
  , stop_loss_trailing_enabled_{stop_loss_trailing_enabled}
  , take_profit_enabled_{take_profit_enabled}
  , take_profit_risk_multiplier_{take_profit_risk_multiplier}
  {
  }

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

  auto risk_method(this const Strategy& self) noexcept -> const AnySeriesMethod&
  {
    return self.risk_method_;
  }

  void risk_method(this Strategy& self, AnySeriesMethod risk_method) noexcept
  {
    self.risk_method_ = std::move(risk_method);
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

  auto take_profit_risk_multiplier(this const Strategy& self) noexcept -> double
  {
    return self.take_profit_risk_multiplier_;
  }

  void take_profit_risk_multiplier(this Strategy& self,
                                   double take_profit_risk_multiplier) noexcept
  {
    self.take_profit_risk_multiplier_ = take_profit_risk_multiplier;
  }

private:
  std::string name_;

  SeriesMethodRegistry series_registry_;

  AnySeriesMethod risk_method_;

  AnyConditionMethod long_entry_filter_{FalseMethod{}};
  AnyConditionMethod long_exit_filter_{FalseMethod{}};

  AnyConditionMethod short_entry_filter_{FalseMethod{}};
  AnyConditionMethod short_exit_filter_{FalseMethod{}};

  bool stop_loss_enabled_{false};
  bool stop_loss_trailing_enabled_{false};

  bool take_profit_enabled_{false};
  double take_profit_risk_multiplier_{1.0};
};

auto risk_reward_config_parser() -> ConfigParser
{
  auto config_parser = ConfigParser{};

  config_parser.register_method_parser(
   "ATR",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod& method) -> jsoncons::ojson {
     auto serialized_method = jsoncons::ojson::null();

     auto atr_method = series_method_cast<AtrMethod>(method);

     if(atr_method) {
       serialized_method = jsoncons::ojson{};
       serialized_method["atr"] = jsoncons::ojson{};
       serialized_method["atr"]["period"] = atr_method->period();
       serialized_method["atr"]["multiplier"] = atr_method->multiplier();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const jsoncons::ojson& parameters) -> AnySeriesMethod {
     auto period = std::size_t{14};
     auto multiplier = 1.0;

     if(parameters.contains("atr") && parameters.at("atr").is_object()) {
       const auto& atr_params = parameters.at("atr");
       if(atr_params.contains("period")) {
         period = atr_params.at("period").as<std::size_t>();
       }
       if(atr_params.contains("multiplier")) {
         multiplier = atr_params.at("multiplier").as_double();
       }
     }

     const auto atr_method = AtrMethod{period, multiplier};
     return atr_method;
   });

  config_parser.register_method_parser(
   "PERCENTAGE",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod& method) -> jsoncons::ojson {
     auto serialized_method = jsoncons::ojson::null();

     auto percent_method =
      series_method_cast<PercentageMethod<AnySeriesMethod>>(method);

     if(percent_method) {
       serialized_method = jsoncons::ojson{};
       serialized_method["percentage"] = percent_method->percent();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const jsoncons::ojson& parameters) -> AnySeriesMethod {
     const auto base_method = CloseMethod{};

     const auto percent = parameters.at("percentage").as_double();

     const auto percentage_method =
      PercentageMethod<AnySeriesMethod>{base_method, percent};

     return percentage_method;
   });

  config_parser.register_method_parser(
   "VALUE",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod& method) -> jsoncons::ojson {
     auto serialized_method = jsoncons::ojson::null();

     auto value_method = series_method_cast<ValueMethod>(method);

     if(value_method) {
       serialized_method = jsoncons::ojson{};
       serialized_method["value"] = value_method->value();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const jsoncons::ojson& parameters) -> AnySeriesMethod {
     const auto value = parameters.at("value").as_double();
     return ValueMethod{value};
   });

  return config_parser;
}

auto parse_backtest_strategy_json(std::string_view strategy_name,
                                  std::istream& json_strategy_stream)
 -> backtest::Strategy
{
  auto series_registry = SeriesMethodRegistry{};
  auto config_parser = make_default_registered_config_parser();

  auto strategy_json = jsoncons::ojson::parse(
   json_strategy_stream, jsoncons::json_options{}.allow_comments(true));
  if(strategy_json.contains("series")) {
    for(const auto& [name, series_method] :
        strategy_json["series"].object_range()) {
      const auto method = config_parser.parse_method(series_method);
      series_registry.set(name, method);
    }
  }

  auto risk_parser = risk_reward_config_parser();
  const auto risk_config = strategy_json.at("risk");
  const auto risk_method = risk_parser.parse_method(risk_config);

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
  auto take_profit_risk_multiplier = 1.0;
  if(strategy_json.contains("takeProfit")) {
    const auto take_profit_config = strategy_json.at("takeProfit");
    if(take_profit_config.is_bool()) {
      is_take_profit_enabled = take_profit_config.as_bool();
    } else if(take_profit_config.is_object()) {
      is_take_profit_enabled =
       take_profit_config.get_value_or<bool>("enabled", true);
      take_profit_risk_multiplier =
       take_profit_config.get_value_or<double>("riskMultiplier", 1.0);
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
       stop_loss_config.get_value_or<bool>("isTrailing", false);
      is_stop_loss_enabled =
       stop_loss_config.get_value_or<bool>("enabled", true);
    } else {
      throw std::runtime_error(
       "Invalid stop loss configuration in strategy JSON");
    }
  }

  return pludux::backtest::Strategy{std::string{strategy_name},
                                    std::move(series_registry),
                                    std::move(risk_method),
                                    std::move(long_entry_filter),
                                    std::move(long_exit_filter),
                                    std::move(short_entry_filter),
                                    std::move(short_exit_filter),
                                    is_stop_loss_enabled,
                                    is_trailing_stop_loss,
                                    is_take_profit_enabled,
                                    take_profit_risk_multiplier};
}

auto stringify_backtest_strategy(const backtest::Strategy& strategy)
 -> jsoncons::ojson
{
  auto config_parser = make_default_registered_config_parser();
  auto risk_parser = risk_reward_config_parser();

  auto strategy_json = jsoncons::ojson{};

  auto series_json = jsoncons::ojson{};
  for(const auto& [name, method] : strategy.series_registry()) {
    series_json[name] = config_parser.serialize_method(method);
  }
  strategy_json["series"] = std::move(series_json);

  strategy_json["risk"] = risk_parser.serialize_method(strategy.risk_method());

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
  strategy_json["stopLoss"]["isTrailing"] =
   strategy.stop_loss_trailing_enabled();

  strategy_json["takeProfit"] = jsoncons::ojson{};
  strategy_json["takeProfit"]["enabled"] = strategy.take_profit_enabled();
  strategy_json["takeProfit"]["riskMultiplier"] =
   strategy.take_profit_risk_multiplier();

  return strategy_json;
}

} // namespace pludux::backtest
