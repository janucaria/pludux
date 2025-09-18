module;

#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <nlohmann/json.hpp>

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

// ------------------------------------------------------------------------

Strategy::Strategy(std::string name,
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

auto Strategy::name() const noexcept -> const std::string&
{
  return name_;
}

auto Strategy::method_registry() const noexcept
 -> std::shared_ptr<screener::MethodRegistry>
{
  return method_registry_;
}

auto Strategy::risk_method() const noexcept -> const screener::ScreenerMethod&
{
  return risk_method_;
}

auto Strategy::long_entry_filter() const noexcept
 -> const screener::ScreenerFilter&
{
  return long_entry_filter_;
}

auto Strategy::long_exit_filter() const noexcept
 -> const screener::ScreenerFilter&
{
  return long_exit_filter_;
}

auto Strategy::short_entry_filter() const noexcept
 -> const screener::ScreenerFilter&
{
  return short_entry_filter_;
}

auto Strategy::short_exit_filter() const noexcept
 -> const screener::ScreenerFilter&
{
  return short_exit_filter_;
}

auto Strategy::stop_loss_enabled() const noexcept -> bool
{
  return stop_loss_enabled_;
}

auto Strategy::stop_loss_trailing_enabled() const noexcept -> bool
{
  return stop_loss_trailing_enabled_;
}

auto Strategy::stop_loss_risk_multiplier() const noexcept -> double
{
  return stop_loss_risk_multiplier_;
}

auto Strategy::take_profit_enabled() const noexcept -> bool
{
  return take_profit_enabled_;
}

auto Strategy::take_profit_risk_multiplier() const noexcept -> double
{
  return take_profit_risk_multiplier_;
}

auto Strategy::entry_long_trade(const AssetSnapshot& asset_snapshot,
                                double risk_value) const noexcept
 -> std::optional<TradeEntry>
{
  auto result = std::optional<TradeEntry>{};

  if(long_entry_filter()(asset_snapshot)) {
    const auto risk_size = risk_method_(asset_snapshot)[0];
    const auto position_size = risk_value / risk_size;
    const auto entry_price = asset_snapshot.get_close();

    const auto stop_loss_price =
     stop_loss_enabled_ ? entry_price - risk_size * stop_loss_risk_multiplier_
                        : NAN;
    const auto is_stop_loss_trailing = stop_loss_trailing_enabled();
    const auto take_profit_price =
     take_profit_enabled_
      ? entry_price + risk_size * take_profit_risk_multiplier_
      : NAN;

    result = TradeEntry{position_size,
                        entry_price,
                        stop_loss_price,
                        is_stop_loss_trailing,
                        take_profit_price};
  }

  return result;
}

auto Strategy::entry_short_trade(const AssetSnapshot& asset_snapshot,
                                 double risk_value) const noexcept
 -> std::optional<TradeEntry>
{
  auto result = std::optional<TradeEntry>{};

  if(short_entry_filter()(asset_snapshot)) {
    const auto risk_size = -risk_method_(asset_snapshot)[0];
    const auto position_size = risk_value / risk_size;
    const auto entry_price = asset_snapshot.get_close();

    const auto stop_loss_price =
     stop_loss_enabled_ ? entry_price - risk_size * stop_loss_risk_multiplier_
                        : NAN;
    const auto is_stop_loss_trailing = stop_loss_trailing_enabled();
    const auto take_profit_price =
     take_profit_enabled_
      ? entry_price + risk_size * take_profit_risk_multiplier_
      : NAN;

    result = TradeEntry{position_size,
                        entry_price,
                        stop_loss_price,
                        is_stop_loss_trailing,
                        take_profit_price};
  }

  return result;
}

auto Strategy::entry_trade(const AssetSnapshot& asset_snapshot,
                           double risk_value) const noexcept
 -> std::optional<TradeEntry>
{
  return entry_long_trade(asset_snapshot, risk_value).or_else([&] {
    return entry_short_trade(asset_snapshot, risk_value);
  });
}

auto Strategy::exit_trade(const AssetSnapshot& asset_snapshot,
                          double position_size) const noexcept
 -> std::optional<TradeExit>
{
  const auto is_long_direction = position_size > 0;
  const auto is_short_direction = position_size < 0;
  const auto exit_price = asset_snapshot.get_close();

  if(is_long_direction) {
    if(long_exit_filter()(asset_snapshot)) {
      return TradeExit{position_size, exit_price, TradeExit::Reason::signal};
    }
  } else if(is_short_direction) {
    if(short_exit_filter()(asset_snapshot)) {
      const auto exit_price = asset_snapshot.get_close();
      return TradeExit{position_size, exit_price, TradeExit::Reason::signal};
    }
  }

  return std::nullopt;
}

auto parse_backtest_strategy_json(std::string_view strategy_name,
                                  std::istream& json_strategy_stream)
 -> backtest::Strategy
{
  auto method_registry = std::make_shared<screener::MethodRegistry>();
  auto config_parser = pludux::ConfigParser{method_registry};

  config_parser.register_default_parsers();
  auto strategy_json =
   nlohmann::json::parse(json_strategy_stream, nullptr, true, true);
  if(strategy_json.contains("series")) {
    for(const auto& [name, series_method] : strategy_json["series"].items()) {
      const auto method = config_parser.parse_method(series_method);
      method_registry->set(name, method);
    }
  }

  auto risk_parser = risk_reward_config_parser();
  const auto risk_config = strategy_json.at("risk");
  const auto risk_method = risk_parser.parse_method(risk_config);

  auto long_entry_filter = screener::ScreenerFilter{screener::FalseFilter{}};
  auto long_exit_filter = screener::ScreenerFilter{screener::FalseFilter{}};
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

  auto short_entry_filter = screener::ScreenerFilter{screener::FalseFilter{}};
  auto short_exit_filter = screener::ScreenerFilter{screener::FalseFilter{}};
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
    if(take_profit_config.is_boolean()) {
      is_take_profit_enabled = take_profit_config.get<bool>();
    } else if(take_profit_config.is_object()) {
      is_take_profit_enabled = take_profit_config.value("enabled", true);
      take_profit_risk_multiplier =
       take_profit_config.value("riskMultiplier", 1.0);
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
    if(stop_loss_config.is_boolean()) {
      is_stop_loss_enabled = stop_loss_config.get<bool>();
    } else if(stop_loss_config.is_object()) {
      is_trailing_stop_loss = stop_loss_config.value("isTrailing", false);
      is_stop_loss_enabled = stop_loss_config.value("enabled", true);
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

auto risk_reward_config_parser() -> ConfigParser
{
  auto config_parser = ConfigParser{};

  config_parser.register_method_parser(
   "ATR",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod& method) -> nlohmann::json {
     auto serialized_method = nlohmann::json{};

     auto atr_method = screener_method_cast<screener::AtrMethod>(method);

     if(atr_method) {
       serialized_method["atr"] = nlohmann::json{};
       serialized_method["atr"]["period"] = atr_method->period();
       serialized_method["atr"]["multiplier"] = atr_method->multiplier();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const nlohmann::json& parameters) -> screener::ScreenerMethod {
     auto period = std::size_t{14};
     auto multiplier = 1.0;

     if(parameters.contains("atr") && parameters.at("atr").is_object()) {
       const auto& atr_params = parameters.at("atr");
       if(atr_params.contains("period")) {
         period = atr_params.at("period").get<std::size_t>();
       }
       if(atr_params.contains("multiplier")) {
         multiplier = atr_params.at("multiplier").get<double>();
       }
     }

     const auto atr_method = screener::AtrMethod{screener::HighMethod{},
                                                 screener::LowMethod{},
                                                 screener::CloseMethod{},
                                                 period,
                                                 multiplier};
     return atr_method;
   });

  config_parser.register_method_parser(
   "PERCENTAGE",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod& method) -> nlohmann::json {
     auto serialized_method = nlohmann::json{};

     auto percent_method =
      screener_method_cast<screener::PercentageMethod>(method);

     if(percent_method) {
       auto value_method =
        screener_method_cast<screener::ValueMethod>(percent_method->percent());

       if(value_method) {
         serialized_method["percentage"] = value_method->value();
       }
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const nlohmann::json& parameters) -> screener::ScreenerMethod {
     const auto total_method = screener::CloseMethod{};

     const auto percent = parameters.at("percentage").get<double>();
     const auto percent_method = screener::ValueMethod{percent};

     const auto percentage_method =
      screener::PercentageMethod{total_method, percent_method};

     return percentage_method;
   });

  config_parser.register_method_parser(
   "VALUE",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod& method) -> nlohmann::json {
     auto serialized_method = nlohmann::json{};

     auto value_method = screener_method_cast<screener::ValueMethod>(method);

     if(value_method) {
       serialized_method["value"] = value_method->value();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const nlohmann::json& parameters) -> screener::ScreenerMethod {
     const auto value = parameters.at("value").get<double>();
     return screener::ValueMethod{value};
   });

  return config_parser;
}

} // namespace pludux::backtest
