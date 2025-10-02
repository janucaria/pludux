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
  enum class EntryRepeat { sequence, always };

  enum class Direction { long_direction, short_direction };

  Strategy(std::string name,
           std::shared_ptr<MethodRegistry> method_registry,
           AnySeriesMethod risk_method,
           AnyConditionMethod long_entry_filter,
           AnyConditionMethod long_exit_filter,
           AnyConditionMethod short_entry_filter,
           AnyConditionMethod short_exit_filter,
           bool stop_loss_enabled,
           bool stop_loss_trailing_enabled,
           double stop_loss_risk_multiplier,
           bool take_profit_enabled,
           double take_profit_risk_multiplier)
  : name_{std::move(name)}
  , method_registry_{std::move(method_registry)}
  , risk_method_{risk_method}
  , long_entry_filter_{std::move(long_entry_filter)}
  , long_exit_filter_{std::move(long_exit_filter)}
  , short_entry_filter_{std::move(short_entry_filter)}
  , short_exit_filter_{std::move(short_exit_filter)}
  , stop_loss_enabled_{stop_loss_enabled}
  , stop_loss_trailing_enabled_{stop_loss_trailing_enabled}
  , stop_loss_risk_multiplier_{stop_loss_risk_multiplier}
  , take_profit_enabled_{take_profit_enabled}
  , take_profit_risk_multiplier_{take_profit_risk_multiplier}
  {
  }

  auto name(this const auto& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  auto method_registry(this const auto& self) noexcept
   -> std::shared_ptr<MethodRegistry>
  {
    return self.method_registry_;
  }

  auto risk_method(this const auto& self) noexcept
   -> const AnySeriesMethod&
  {
    return self.risk_method_;
  }

  auto long_entry_filter(this const auto& self) noexcept
   -> const AnyConditionMethod&
  {
    return self.long_entry_filter_;
  }

  auto long_exit_filter(this const auto& self) noexcept
   -> const AnyConditionMethod&
  {
    return self.long_exit_filter_;
  }

  auto short_entry_filter(this const auto& self) noexcept
   -> const AnyConditionMethod&
  {
    return self.short_entry_filter_;
  }

  auto short_exit_filter(this const auto& self) noexcept
   -> const AnyConditionMethod&
  {
    return self.short_exit_filter_;
  }

  auto stop_loss_enabled(this const auto& self) noexcept -> bool
  {
    return self.stop_loss_enabled_;
  }

  auto stop_loss_trailing_enabled(this const auto& self) noexcept -> bool
  {
    return self.stop_loss_trailing_enabled_;
  }

  auto stop_loss_risk_multiplier(this const auto& self) noexcept -> double
  {
    return self.stop_loss_risk_multiplier_;
  }

  auto take_profit_enabled(this const auto& self) noexcept -> bool
  {
    return self.take_profit_enabled_;
  }

  auto take_profit_risk_multiplier(this const auto& self) noexcept -> double
  {
    return self.take_profit_risk_multiplier_;
  }

  auto entry_long_trade(this const auto& self,
                        const AssetSnapshot& asset_snapshot,
                        double risk_value) noexcept -> std::optional<TradeEntry>
  {
    auto result = std::optional<TradeEntry>{};

    auto context = DefaultMethodContext{*self.method_registry_};

    if(self.long_entry_filter()(asset_snapshot, context)) {
      const auto risk_size = self.risk_method_(asset_snapshot, context);
      const auto position_size = risk_value / risk_size;
      const auto entry_price = asset_snapshot.close();

      const auto stop_loss_price =
       self.stop_loss_enabled_
        ? entry_price - risk_size * self.stop_loss_risk_multiplier_
        : NAN;
      const auto is_stop_loss_trailing = self.stop_loss_trailing_enabled();
      const auto take_profit_price =
       self.take_profit_enabled_
        ? entry_price + risk_size * self.take_profit_risk_multiplier_
        : NAN;

      result = TradeEntry{position_size,
                          entry_price,
                          stop_loss_price,
                          is_stop_loss_trailing,
                          take_profit_price};
    }

    return result;
  }

  auto entry_short_trade(this const auto& self,
                         const AssetSnapshot& asset_snapshot,
                         double risk_value) noexcept
   -> std::optional<TradeEntry>
  {
    auto result = std::optional<TradeEntry>{};

    auto context = DefaultMethodContext{*self.method_registry_};

    if(self.short_entry_filter()(asset_snapshot, context)) {
      const auto risk_size = -self.risk_method_(asset_snapshot, context);
      const auto position_size = risk_value / risk_size;
      const auto entry_price = asset_snapshot.close();

      const auto stop_loss_price =
       self.stop_loss_enabled_
        ? entry_price - risk_size * self.stop_loss_risk_multiplier_
        : NAN;
      const auto is_stop_loss_trailing = self.stop_loss_trailing_enabled();
      const auto take_profit_price =
       self.take_profit_enabled_
        ? entry_price + risk_size * self.take_profit_risk_multiplier_
        : NAN;

      result = TradeEntry{position_size,
                          entry_price,
                          stop_loss_price,
                          is_stop_loss_trailing,
                          take_profit_price};
    }

    return result;
  }

  auto entry_trade(this const auto& self,
                   const AssetSnapshot& asset_snapshot,
                   double risk_value) noexcept -> std::optional<TradeEntry>
  {
    return self.entry_long_trade(asset_snapshot, risk_value).or_else([=] {
      return self.entry_short_trade(asset_snapshot, risk_value);
    });
  }

  auto exit_trade(this const auto& self,
                  const AssetSnapshot& asset_snapshot,
                  double position_size) noexcept -> std::optional<TradeExit>
  {
    const auto is_long_direction = position_size > 0;
    const auto is_short_direction = position_size < 0;
    const auto exit_price = asset_snapshot.close();
    
    auto context = DefaultMethodContext{*self.method_registry_};

    if(is_long_direction) {
      if(self.long_exit_filter()(asset_snapshot, context)) {
        return TradeExit{position_size, exit_price, TradeExit::Reason::signal};
      }
    } else if(is_short_direction) {
      if(self.short_exit_filter()(asset_snapshot, context)) {
        const auto exit_price = asset_snapshot.close();
        return TradeExit{position_size, exit_price, TradeExit::Reason::signal};
      }
    }

    return std::nullopt;
  }

private:
  std::string name_;

  std::shared_ptr<MethodRegistry> method_registry_;

  AnySeriesMethod risk_method_;

  AnyConditionMethod long_entry_filter_{FalseMethod{}};
  AnyConditionMethod long_exit_filter_{FalseMethod{}};

  AnyConditionMethod short_entry_filter_{FalseMethod{}};
  AnyConditionMethod short_exit_filter_{FalseMethod{}};

  bool stop_loss_enabled_{false};
  bool stop_loss_trailing_enabled_{false};
  double stop_loss_risk_multiplier_{1.0};

  bool take_profit_enabled_{false};
  double take_profit_risk_multiplier_{1.0};
};

auto risk_reward_config_parser() -> ConfigParser
{
  auto config_parser = ConfigParser{};

  config_parser.register_method_parser(
   "ATR",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod& method) -> jsoncons::json {
     auto serialized_method = jsoncons::json{};

     auto atr_method = any_method_cast<AtrMethod>(method);

     if(atr_method) {
       serialized_method["atr"] = jsoncons::json{};
       serialized_method["atr"]["period"] = atr_method->period();
       serialized_method["atr"]["multiplier"] = atr_method->multiplier();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const jsoncons::json& parameters) -> AnySeriesMethod {
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
      const AnySeriesMethod& method) -> jsoncons::json {
     auto serialized_method = jsoncons::json{};

     auto percent_method = any_method_cast<
      PercentageMethod<AnySeriesMethod,
                                 AnySeriesMethod>>(method);

     if(percent_method) {
       auto value_method = any_method_cast<ValueMethod>(
        percent_method->percent());

       if(value_method) {
         serialized_method["percentage"] = value_method->value();
       }
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const jsoncons::json& parameters) -> AnySeriesMethod {
     const auto total_method = CloseMethod{};

     const auto percent = parameters.at("percentage").as_double();
     const auto percent_method = ValueMethod{percent};

     const auto percentage_method =
      PercentageMethod{total_method, percent_method};

     return percentage_method;
   });

  config_parser.register_method_parser(
   "VALUE",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod& method) -> jsoncons::json {
     auto serialized_method = jsoncons::json{};

     auto value_method = any_method_cast<ValueMethod>(method);

     if(value_method) {
       serialized_method["value"] = value_method->value();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const jsoncons::json& parameters) -> AnySeriesMethod {
     const auto value = parameters.at("value").as_double();
     return ValueMethod{value};
   });

  return config_parser;
}

auto parse_backtest_strategy_json(std::string_view strategy_name,
                                  std::istream& json_strategy_stream)
 -> backtest::Strategy
{
  auto method_registry = std::make_shared<MethodRegistry>();
  auto config_parser = make_default_registered_config_parser(method_registry);

  auto strategy_json = jsoncons::json::parse(
   json_strategy_stream, jsoncons::json_options{}.allow_comments(true));
  if(strategy_json.contains("series")) {
    for(const auto& [name, series_method] :
        strategy_json["series"].object_range()) {
      const auto method = config_parser.parse_method(series_method);
      method_registry->set(name, method);
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
  auto stop_loss_risk_multiplier = 1.0;
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
                                    std::move(method_registry),
                                    std::move(risk_method),
                                    std::move(long_entry_filter),
                                    std::move(long_exit_filter),
                                    std::move(short_entry_filter),
                                    std::move(short_exit_filter),
                                    is_stop_loss_enabled,
                                    is_trailing_stop_loss,
                                    stop_loss_risk_multiplier,
                                    is_take_profit_enabled,
                                    take_profit_risk_multiplier};
}

} // namespace pludux::backtest
