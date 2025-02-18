#include <algorithm>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <format>
#include <iostream>
#include <optional>
#include <vector>

#include <ctre.hpp>
#include <nlohmann/json.hpp>
#include <rapidcsv.h>

#include <pludux/backtest.hpp>
#include <pludux/config_parser.hpp>

namespace pludux {

Backtest::Backtest(double capital_risk,
                   QuoteAccess quote_access,
                   screener::ScreenerFilter entry_filter,
                   screener::ScreenerFilter exit_filter,
                   backtest::StopLoss stop_loss,
                   backtest::TakeProfit take_profit)
: capital_risk_{capital_risk}
, quote_access_{quote_access}
, entry_filter_{entry_filter}
, exit_filter_{exit_filter}
, stop_loss_{stop_loss}
, take_profit_{take_profit}
, trading_session_{std::nullopt}
, current_index_{0}
, asset_size_{0}
{
}

auto Backtest::capital_risk() const noexcept -> double
{
  return capital_risk_;
}

auto Backtest::quote_access() const noexcept -> const QuoteAccess&
{
  return quote_access_;
}

auto Backtest::backtesting_summary() const noexcept
 -> const backtest::BacktestingSummary&
{
  return backtesting_summary_;
}

void Backtest::reset()
{
  trading_session_.reset();
  current_index_ = 0;
  asset_size_ = 0;
  backtesting_summary_ = backtest::BacktestingSummary{};
}

auto Backtest::run(const AssetHistory& asset) -> bool
{
  if(asset_size_ != asset.size()) {
    reset();
    asset_size_ = asset.size();
  }

  auto& trading_session = trading_session_;
  if(current_index_ >= asset_size_) {
    return false;
  }

  const auto index = asset_size_ - current_index_ - 1;
  const auto snapshot = asset[index];
  ++current_index_;

  if(!trading_session.has_value()) {
    if(entry_filter_(snapshot)) {
      const auto price_method = quote_access_.close();
      const auto time_method = quote_access_.time();
      const auto entry_index = snapshot.offset();
      const auto entry_price = price_method(snapshot)[0];
      const auto trading_take_profit = take_profit_(snapshot);
      const auto trading_stop_loss = stop_loss_(snapshot);
      const auto order_size =
       stop_loss_.get_order_size(capital_risk_, snapshot);

      if(std::isnan(order_size)) {
        return false;
      }

      trading_session.emplace(
       order_size,
       entry_index,
       entry_price,
       static_cast<std::time_t>(time_method(snapshot)[0]),
       time_method,
       price_method,
       exit_filter_,
       trading_take_profit,
       trading_stop_loss);

      const auto open_trade = trading_session->ongoing_trade(snapshot);
      backtesting_summary_.add_trade(open_trade);
    }

    return true;
  }

  auto closed_trade = trading_session->done_trade(snapshot);
  if(closed_trade.has_value()) {
    const auto& closed_trade_val = closed_trade.value();
    backtesting_summary_.add_trade(closed_trade_val);

    trading_session.reset();
  }

  if(current_index_ == asset_size_) {
    const auto open_trade = trading_session->ongoing_trade(snapshot);
    backtesting_summary_.add_trade(open_trade);
  }

  return true;
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
 -> Backtest
{
  auto config_parser = pludux::ConfigParser{};
  config_parser.register_default_parsers();

  auto strategy =
   nlohmann::json::parse(json_strategy_stream, nullptr, true, true);
  if(strategy.contains("methods")) {
    for(const auto& strategy_method : strategy["methods"]) {
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

  auto risk_reward_parser = risk_reward_config_parser(quote_access);

  const auto capital_risk = strategy["capitalRisk"].get<double>();
  const auto entry_filter =
   config_parser.parse_filter(strategy.at("entrySignal"));
  const auto exit_filter =
   config_parser.parse_filter(strategy.at("exitSignal"));
  const auto risk_method =
   risk_reward_parser.parse_method(strategy.at("stopLoss"));
  const auto reward_method =
   risk_reward_parser.parse_method(strategy.at("takeProfit"));
  const auto price_method = quote_access.close();
  const auto timestamp_method = quote_access.time();
  const auto trading_stop_loss_method = quote_access.low();
  const auto trading_take_profit_method = quote_access.high();

  const auto stop_loss = pludux::backtest::StopLoss{
   price_method, risk_method, trading_stop_loss_method};
  const auto take_profit = pludux::backtest::TakeProfit{
   price_method, reward_method, trading_take_profit_method};

  return pludux::Backtest{capital_risk,
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
