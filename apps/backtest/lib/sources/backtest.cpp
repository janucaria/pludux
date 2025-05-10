#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <format>
#include <iostream>
#include <optional>
#include <vector>

#include <ctre.hpp>
#include <nlohmann/json.hpp>
#include <rapidcsv.h>

#include <pludux/backtest.hpp>
#include <pludux/config_parser.hpp>

#include <pludux/backtest/running_state.hpp>

namespace pludux {

Backtest::Backtest(const backtest::Strategy& strategy,
                   const backtest::Asset& asset)
: strategy_{strategy}
, asset_{asset}
, trading_session_{std::nullopt}
, current_index_{0}
, history_{}
{
}

auto Backtest::strategy() const noexcept -> const backtest::Strategy&
{
  return strategy_;
}

auto Backtest::asset() const noexcept -> const backtest::Asset&
{
  return asset_;
}

auto Backtest::capital_risk() const noexcept -> double
{
  return strategy_.capital_risk();
}

auto Backtest::history() const noexcept -> const backtest::History&
{
  return history_;
}

auto Backtest::get_current_asset_timestamp() const noexcept
 -> std::optional<std::time_t>
{
  if(!should_run()) {
    return std::nullopt;
  }
  const auto& asset_history = asset_.history();
  const auto asset_size = asset_history.size();
  const auto asset_index = asset_size - current_index_ - 1;
  const auto asset_snapshot = asset_history[asset_index];
  const auto quote_access = strategy_.quote_access();
  const auto asset_quote = AssetQuote{asset_snapshot, quote_access};

  return static_cast<std::time_t>(asset_quote.time()[0]);
}

void Backtest::reset()
{
  trading_session_.reset();
  current_index_ = 0;
  history_ = backtest::History{};
}

auto Backtest::should_run() const noexcept -> bool
{
  const auto& asset_history = asset_.history();
  const auto asset_size = asset_history.size();
  return current_index_ < asset_size;
}

void Backtest::run(std::time_t timeframe_timestamp)
{
  const auto& asset = asset_.history();
  const auto asset_size = asset.size();
  const auto last_index = asset_size - 1;
  const auto current_index = std::min(current_index_, last_index);

  const auto quote_access = strategy_.quote_access();

  auto& trading_session = trading_session_;
  const auto asset_index = last_index - current_index;
  const auto asset_snapshot = asset[asset_index];
  const auto asset_quote = AssetQuote{asset_snapshot, quote_access};
  auto running_state =
   backtest::RunningState{timeframe_timestamp, asset_quote, asset_index};
  running_state.capital_risk(capital_risk());

  if(!running_state.is_timeframe_match()) {
    // get the previous asset snapshot
    const auto previous_asset_index = asset_index + 1;
    const auto previous_asset_snapshot = asset[previous_asset_index];
    const auto previous_asset_quote =
     AssetQuote{previous_asset_snapshot, quote_access};
    const auto previous_running_state = backtest::RunningState{
     timeframe_timestamp, previous_asset_quote, previous_asset_index};

    push_nan_data(previous_running_state);
    return;
  }

  ++current_index_;

  if(!trading_session.has_value()) {
    if(strategy_.entry_filter()(asset_snapshot)) {
      const auto& take_profit = strategy_.take_profit();
      const auto& stop_loss = strategy_.stop_loss();
      const auto trading_take_profit = take_profit(running_state);
      const auto trading_stop_loss = stop_loss(running_state);
      const auto order_size = stop_loss.get_order_size(running_state);

      if(std::isnan(order_size)) {
        push_history_data(running_state);
        return;
      }

      const auto entry_index = running_state.asset_index();
      const auto entry_price = running_state.price();
      const auto entry_timestamp =
       static_cast<std::time_t>(running_state.timestamp());
      const auto exit_filter = strategy_.exit_filter();
      trading_session.emplace(order_size,
                              entry_index,
                              entry_price,
                              entry_timestamp,
                              exit_filter,
                              trading_take_profit,
                              trading_stop_loss);
    }

    push_history_data(running_state);
    return;
  }

  push_history_data(running_state);
  return;
}

void Backtest::push_history_data(const backtest::RunningState& running_state)
{
  const auto timestamp = running_state.timeframe_timestamp();
  const auto open = running_state.open();
  const auto high = running_state.high();
  const auto low = running_state.low();
  const auto close = running_state.close();
  const auto volume = running_state.volume();

  auto trade_record = std::optional<backtest::TradeRecord>{};
  if(trading_session_) {
    trade_record = trading_session_->get_trading_state(running_state);
    if(trade_record->is_closed()) {
      trading_session_.reset();
    }
  }

  history_.add_data(backtest::Snapshot{static_cast<double>(timestamp),
                                       open,
                                       high,
                                       low,
                                       close,
                                       volume,
                                       std::move(trade_record)});
}

void Backtest::push_nan_data(const backtest::RunningState& running_state)
{
  const auto timestamp = running_state.timeframe_timestamp();
  const auto open = std::numeric_limits<double>::quiet_NaN();
  const auto high = std::numeric_limits<double>::quiet_NaN();
  const auto low = std::numeric_limits<double>::quiet_NaN();
  const auto close = std::numeric_limits<double>::quiet_NaN();
  const auto volume = std::numeric_limits<double>::quiet_NaN();

  auto trade_record = std::optional<backtest::TradeRecord>{};
  if(trading_session_) {
    trade_record = trading_session_->get_trading_state(running_state);
    if(trade_record->is_closed()) {
      trading_session_.reset();
    }
  }

  history_.add_data(backtest::Snapshot{static_cast<double>(timestamp),
                                       open,
                                       high,
                                       low,
                                       close,
                                       volume,
                                       std::move(trade_record)});
}

auto get_env_var(std::string_view var_name) -> std::optional<std::string>
{
  if(const auto env_var = std::getenv(var_name.data()); env_var != nullptr) {
    return std::string{env_var};
  }
  return std::nullopt;
}

auto csv_daily_stock_data(std::istream& csv_stream)
 -> std::vector<std::pair<std::string, pludux::DataSeries<double>>>
{
  auto result =
   std::vector<std::pair<std::string, pludux::DataSeries<double>>>{};

  auto csv_doc = rapidcsv::Document(csv_stream);
  const auto column_count = csv_doc.GetColumnCount();

  if(column_count == 0) {
    return result;
  }

  constexpr auto date_record_index = 0;
  const auto first_records = csv_doc.GetColumn<std::string>(date_record_index);
  auto date_records = std::vector<double>{};

  constexpr auto date_regex_match =
   ctre::match<"^"
               "(\\d{4})-(\\d{2})-(\\d{2})" // YYYY-MM-DD
               "(?:[T ](\\d{2}):(\\d{2})"   // hh:mm
               "(?::(\\d{2}(?:\\.\\d+)?))?" // :ss.s
               "(Z|[+\\-]\\d{2}:\\d{2})"    // Z or +hh:mm or -hh:mm
               ")?"
               "$">;

  std::transform(
   first_records.cbegin(),
   first_records.cend(),
   std::back_inserter(date_records),
   [&date_regex_match](const std::string& date) -> double {
     const auto date_match = date_regex_match(date);

     if(!date_match) {
       auto timestamp = std::stod(date);
       return timestamp;
     }

     const auto year = date_match.template get<1>().str();
     const auto month = date_match.template get<2>().str();
     const auto day = date_match.template get<3>().str();
     const auto hour =
      date_match.template get<4>().to_optional_string().value_or("00");
     const auto minute =
      date_match.template get<5>().to_optional_string().value_or("00");
     const auto second =
      date_match.template get<6>().to_optional_string().value_or("00");
     const auto timezone =
      date_match.template get<7>().to_optional_string().value_or("Z");

     const auto date_str = std::format(
      "{}-{}-{}T{}:{}:{}{}", year, month, day, hour, minute, second, timezone);
     auto datetime_tm = std::tm{};
     auto date_stream = std::istringstream{date_str};
     date_stream >> std::get_time(&datetime_tm, "%Y-%m-%dT%H:%M:%S%z");
     const auto timestamp = std::mktime(&datetime_tm);
     return static_cast<double>(timestamp);
   });

  const auto should_reverse = date_records.front() < date_records.back();
  if(should_reverse) {
    std::reverse(date_records.begin(), date_records.end());
  }

  const auto date_record_header = csv_doc.GetColumnName(date_record_index);
  result.emplace_back(
   date_record_header,
   pludux::DataSeries<double>(date_records.begin(), date_records.end()));

  for(auto i = 1; i < column_count; ++i) {
    auto column = csv_doc.GetColumn<double>(i);
    if(should_reverse) {
      std::reverse(column.begin(), column.end());
    }

    const auto column_header = csv_doc.GetColumnName(i);
    result.emplace_back(
     column_header, pludux::DataSeries<double>(column.begin(), column.end()));
  }

  return result;
}

auto parse_backtest_strategy_json(std::istream& json_strategy_stream)
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

  auto named_methods = config_parser.get_named_methods();
  const std::vector<std::string>& required_methods = {
   "time", "open", "high", "low", "close", "volume"};
  for(const auto& required_method : required_methods) {
    if(!named_methods.contains(required_method)) {
      throw std::runtime_error{
       std::format("Missing required method: {}", required_method)};
    }
  }

  const auto quote_access = pludux::QuoteAccess{named_methods.at("time"),
                                                named_methods.at("open"),
                                                named_methods.at("high"),
                                                named_methods.at("low"),
                                                named_methods.at("close"),
                                                named_methods.at("volume")};

  const auto capital_risk = strategy_json["capitalRisk"].get<double>();
  const auto entry_filter =
   config_parser.parse_filter(strategy_json.at("entrySignal"));
  const auto exit_filter =
   config_parser.parse_filter(strategy_json.at("exitSignal"));
  const auto price_method = quote_access.close();

  auto reward_parser = risk_reward_config_parser(quote_access);
  const auto take_profit_config = strategy_json.at("takeProfit");
  const auto is_take_profit_disabled =
   take_profit_config.contains("disabled")
    ? take_profit_config.at("disabled").get<bool>()
    : false;
  const auto reward_method =
   reward_parser.parse_method(strategy_json.at("takeProfit"));
  const auto trading_take_profit_method = quote_access.high();
  const auto take_profit = pludux::backtest::TakeProfit{
   reward_method, is_take_profit_disabled, trading_take_profit_method};

  auto risk_parser = risk_reward_config_parser(quote_access);
  risk_parser.register_method_parser(
   "POSITION_SIZE",
   [&](ConfigParser::Parser config_parser,
       const nlohmann::json& parameters) -> screener::ScreenerMethod {
     const auto position_size = parameters.at("positionSize").get<double>();

     const auto position_size_method = screener::ValueMethod{position_size};
     const auto capital_risk_method = screener::ValueMethod{capital_risk};
     const auto entry_price_method = price_method;

     return screener::DivideMethod{
      screener::MultiplyMethod{capital_risk_method, price_method},
      position_size_method};
   });

  const auto stop_loss_config = strategy_json.at("stopLoss");
  const auto is_trailing_stop_loss =
   stop_loss_config.contains("isTrailing")
    ? stop_loss_config.at("isTrailing").get<bool>()
    : false;
  const auto is_stop_loss_disabled =
   stop_loss_config.contains("disabled")
    ? stop_loss_config.at("disabled").get<bool>()
    : false;
  const auto risk_method = risk_parser.parse_method(stop_loss_config);
  const auto stop_loss = pludux::backtest::StopLoss{
   risk_method, is_stop_loss_disabled, is_trailing_stop_loss};

  return pludux::backtest::Strategy{capital_risk,
                                    quote_access,
                                    entry_filter,
                                    exit_filter,
                                    stop_loss,
                                    take_profit};
}

auto risk_reward_config_parser(QuoteAccess quote_access) -> ConfigParser
{
  auto config_parser = ConfigParser{};

  config_parser.register_method_parser(
   "ATR",
   [quote_access](
    ConfigParser::Parser config_parser,
    const nlohmann::json& parameters) -> screener::ScreenerMethod {
     const auto period = parameters.contains("period")
                          ? parameters.at("period").get<std::size_t>()
                          : 14;
     const auto multiplier = parameters.contains("multiplier")
                              ? parameters.at("multiplier").get<double>()
                              : 1;

     const auto atr_method = screener::AtrMethod{quote_access.high(),
                                                 quote_access.low(),
                                                 quote_access.close(),
                                                 period,
                                                 multiplier};
     return atr_method;
   });

  config_parser.register_method_parser(
   "PERCENTAGE",
   [quote_access](
    ConfigParser::Parser config_parser,
    const nlohmann::json& parameters) -> screener::ScreenerMethod {
     const auto total_method = quote_access.close();

     const auto percent = parameters.at("percent").get<double>();
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

} // namespace pludux
