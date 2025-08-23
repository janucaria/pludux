#include <utility>

#include <nlohmann/json.hpp>

#include <pludux/backtest/strategy.hpp>

namespace pludux::backtest {

Strategy::Strategy(std::string name,
                   Direction direction,
                   RepeatType entry_repeat,
                   screener::ScreenerFilter entry_filter,
                   screener::ScreenerFilter exit_filter,
                   backtest::StopLoss stop_loss,
                   backtest::TakeProfit take_profit)
: name_{std::move(name)}
, direction_{direction}
, entry_repeat_{entry_repeat}
, entry_filter_{std::move(entry_filter)}
, exit_filter_{std::move(exit_filter)}
, stop_loss_{std::move(stop_loss)}
, take_profit_{std::move(take_profit)}
{
}

auto Strategy::direction() const noexcept -> Direction
{
  return direction_;
}

void Strategy::direction(Direction dir) noexcept
{
  direction_ = dir;
}

auto Strategy::name() const noexcept -> const std::string&
{
  return name_;
}

auto Strategy::entry_repeat() const noexcept -> RepeatType
{
  return entry_repeat_;
}

void Strategy::entry_repeat(RepeatType repeat_type) noexcept
{
  entry_repeat_ = repeat_type;
}

auto Strategy::entry_filter() const noexcept -> const screener::ScreenerFilter&
{
  return entry_filter_;
}

auto Strategy::exit_filter() const noexcept -> const screener::ScreenerFilter&
{
  return exit_filter_;
}

auto Strategy::stop_loss() const noexcept -> const backtest::StopLoss&
{
  return stop_loss_;
}

auto Strategy::take_profit() const noexcept -> const backtest::TakeProfit&
{
  return take_profit_;
}

auto Strategy::is_entry_repeat_sequence() const noexcept -> bool
{
  return entry_repeat_ == RepeatType::sequence;
}

void Strategy::set_entry_repeat_to_sequence() noexcept
{
  entry_repeat_ = RepeatType::sequence;
}

auto Strategy::is_entry_repeat_always() const noexcept -> bool
{
  return entry_repeat_ == RepeatType::always;
}

void Strategy::set_entry_repeat_to_always() noexcept
{
  entry_repeat_ = RepeatType::always;
}

auto Strategy::is_long_direction() const noexcept -> bool
{
  return direction_ == Direction::long_direction;
}

auto Strategy::is_short_direction() const noexcept -> bool
{
  return direction_ == Direction::short_direction;
}

auto parse_backtest_strategy_json(std::string_view strategy_name,
                                  std::istream& json_strategy_stream)
 -> backtest::Strategy
{
  auto config_parser = pludux::ConfigParser{};
  config_parser.register_default_parsers();

  auto strategy_json =
   nlohmann::json::parse(json_strategy_stream, nullptr, true, true);
  if(strategy_json.contains("methods")) {
    for(const auto& strategy_method : strategy_json["methods"]) {
      config_parser.parse_method(strategy_method);
    }
  }

  auto risk_parser = risk_reward_config_parser();
  const auto risk_config = strategy_json.at("risk");
  const auto risk_method = risk_parser.parse_method(risk_config);

  const auto& positions_json = strategy_json.at("positions");
  if(!positions_json.is_array() || positions_json.empty()) {
    throw std::runtime_error("Invalid or empty positions in strategy JSON");
  }

  if(positions_json.size() > 1) {
    throw std::runtime_error("Multiple positions are not supported yet");
  }

  const auto& position_json = positions_json[0];

  const auto direction_str = position_json.value("direction", "long");
  auto direction = Strategy::Direction::long_direction;
  if(direction_str == "long") {
    direction = Strategy::Direction::long_direction;
  } else if(direction_str == "short") {
    direction = Strategy::Direction::short_direction;
  } else {
    throw std::runtime_error("Invalid direction in strategy JSON");
  }

  const auto is_short_position =
   direction == Strategy::Direction::short_direction;

  const auto entry_json = position_json.at("entry");
  auto entry_repeat = Strategy::RepeatType::sequence;
  if(entry_json.contains("repeat")) {
    const auto repeat_value = entry_json.at("repeat").get<std::string>();
    if(repeat_value == "always") {
      entry_repeat = Strategy::RepeatType::always;
    }
  }
  const auto entry_filter = config_parser.parse_filter(entry_json.at("signal"));

  auto exit_filter = screener::ScreenerFilter{screener::FalseFilter{}};
  if(position_json.contains("exit")) {
    const auto exit_json = position_json.at("exit");
    if(!exit_json.is_object()) {
      throw std::runtime_error("Invalid exit configuration in strategy JSON");
    }

    if(exit_json.contains("signal")) {
      exit_filter = config_parser.parse_filter(exit_json.at("signal"));
    }
  }

  auto is_take_profit_disabled = false;
  auto reward_factor = 1.0;
  if(position_json.contains("takeProfit")) {
    const auto take_profit_config = position_json.at("takeProfit");
    if(take_profit_config.is_boolean()) {
      is_take_profit_disabled = !take_profit_config.get<bool>();
    } else if(take_profit_config.is_object()) {
      is_take_profit_disabled = take_profit_config.value("disabled", false);
      reward_factor = take_profit_config.value("rewardFactor", 1.0);
    } else {
      throw std::runtime_error(
       "Invalid take profit configuration in strategy JSON");
    }
  }

  const auto reward_method =
   screener::MultiplyMethod{risk_method, screener::ValueMethod{reward_factor}};
  const auto trading_take_profit_method =
   is_short_position ? screener::ScreenerMethod{screener::LowMethod{}}
                     : screener::ScreenerMethod{screener::HighMethod{}};
  const auto take_profit =
   pludux::backtest::TakeProfit{reward_method,
                                is_take_profit_disabled,
                                is_short_position,
                                trading_take_profit_method};

  auto is_trailing_stop_loss = false;
  auto is_stop_loss_disabled = false;

  if(position_json.contains("stopLoss")) {
    const auto stop_loss_config = position_json.at("stopLoss");
    if(stop_loss_config.is_boolean()) {
      is_stop_loss_disabled = !stop_loss_config.get<bool>();
    } else if(stop_loss_config.is_object()) {
      is_trailing_stop_loss = stop_loss_config.value("isTrailing", false);
      is_stop_loss_disabled = stop_loss_config.value("disabled", false);
    } else {
      throw std::runtime_error(
       "Invalid stop loss configuration in strategy JSON");
    }
  }

  const auto stop_loss = pludux::backtest::StopLoss{risk_method,
                                                    is_stop_loss_disabled,
                                                    is_short_position,
                                                    is_trailing_stop_loss};

  return pludux::backtest::Strategy{std::string{strategy_name},
                                    direction,
                                    entry_repeat,
                                    entry_filter,
                                    exit_filter,
                                    stop_loss,
                                    take_profit};
}

auto risk_reward_config_parser() -> ConfigParser
{
  auto config_parser = ConfigParser{};

  config_parser.register_method_parser(
   "ATR",
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
   [](ConfigParser::Parser config_parser,
      const nlohmann::json& parameters) -> screener::ScreenerMethod {
     const auto value = parameters.at("value").get<double>();
     return screener::ValueMethod{value};
   });

  return config_parser;
}

} // namespace pludux::backtest
