#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <format>
#include <iostream>
#include <limits>
#include <optional>
#include <vector>

#include <ctre.hpp>
#include <nlohmann/json.hpp>
#include <rapidcsv.h>

#include <pludux/backtest.hpp>
#include <pludux/config_parser.hpp>

#include <pludux/backtest/running_state.hpp>

namespace pludux {

Backtest::Backtest(std::string name,
                   std::shared_ptr<backtest::Strategy> strategy_ptr,
                   std::shared_ptr<backtest::Asset> asset_ptr)
: name_{std::move(name)}
, strategy_ptr_{strategy_ptr}
, asset_ptr_{asset_ptr}
, is_failed_{false}
, trading_session_{std::nullopt}
, current_index_{0}
, trade_records_{}
{
}

auto Backtest::name() const noexcept -> const std::string&
{
  return name_;
}

auto Backtest::strategy_ptr() const noexcept
 -> const std::shared_ptr<backtest::Strategy>
{
  return strategy_ptr_;
}

auto Backtest::asset_ptr() const noexcept
 -> const std::shared_ptr<backtest::Asset>
{
  return asset_ptr_;
}

auto Backtest::strategy() const noexcept -> const backtest::Strategy&
{
  return *strategy_ptr();
}

auto Backtest::asset() const noexcept -> const backtest::Asset&
{
  return *asset_ptr();
}

void Backtest::mark_as_failed() noexcept
{
  is_failed_ = true;
}

auto Backtest::is_failed() const noexcept -> bool
{
  return is_failed_;
}

auto Backtest::capital_risk() const noexcept -> double
{
  return strategy().capital_risk();
}

auto Backtest::trade_records() const noexcept
 -> const std::vector<backtest::TradeRecord>&
{
  return trade_records_;
}

void Backtest::reset()
{
  trading_session_.reset();
  current_index_ = 0;
  trade_records_.clear();
}

auto Backtest::should_run() const noexcept -> bool
{
  const auto& asset_history = asset().history();
  const auto asset_size = asset_history.size();
  return current_index_ < asset_size && !is_failed();
}

void Backtest::run()
{
  const auto& asset_history = asset().history();
  const auto asset_size = asset_history.size();
  const auto last_index = asset_size - 1;
  const auto current_index = std::min(current_index_, last_index);

  const auto asset_index = last_index - current_index;
  const auto asset_snapshot = AssetSnapshot(asset_index, asset_history);
  auto running_state = backtest::RunningState{asset_snapshot};
  running_state.capital_risk(capital_risk());

  ++current_index_;

  if(!trading_session_.has_value() &&
     strategy().entry_filter()(running_state.asset_snapshot())) {
    const auto& take_profit = strategy().take_profit();
    const auto& stop_loss = strategy().stop_loss();
    const auto trading_take_profit = take_profit(running_state);
    const auto trading_stop_loss = stop_loss(running_state);
    const auto order_size = stop_loss.get_order_size(running_state);

    if(!std::isnan(order_size)) {
      const auto entry_index = running_state.asset_index();
      const auto entry_price = running_state.price();
      const auto entry_timestamp =
       static_cast<std::time_t>(running_state.timestamp());
      const auto exit_filter = strategy().exit_filter();
      trading_session_.emplace(order_size,
                               entry_index,
                               entry_price,
                               entry_timestamp,
                               exit_filter,
                               trading_take_profit,
                               trading_stop_loss);
    }
  }

  if(trading_session_) {
    const auto asset_index = running_state.asset_index();
    auto trade_record = trading_session_->get_trading_state(running_state);

    trade_records_.emplace_back(trade_record);

    if(trade_record.is_closed() || asset_index == 0) {
      trading_session_.reset();
    }
  } else {
    auto trade_record =
     backtest::TradeRecord{backtest::TradeRecord::Status::flat,
                           std::numeric_limits<double>::quiet_NaN(),
                           running_state.asset_index(),
                           running_state.asset_index(),
                           static_cast<std::time_t>(running_state.timestamp()),
                           static_cast<std::time_t>(running_state.timestamp()),
                           running_state.price(),
                           running_state.price(),
                           std::numeric_limits<double>::quiet_NaN(),
                           std::numeric_limits<double>::quiet_NaN(),
                           std::numeric_limits<double>::quiet_NaN()};
    trade_records_.emplace_back(trade_record);
  }
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

auto parse_backtest_strategy_json(const std::string& strategy_name,
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

  const auto capital_risk = strategy_json["capitalRisk"].get<double>();
  const auto entry_filter =
   config_parser.parse_filter(strategy_json.at("entrySignal"));
  const auto exit_filter =
   config_parser.parse_filter(strategy_json.at("exitSignal"));
  const auto price_method = screener::CloseMethod{};

  auto reward_parser = risk_reward_config_parser();
  const auto take_profit_config = strategy_json.at("takeProfit");
  const auto is_take_profit_disabled =
   take_profit_config.contains("disabled")
    ? take_profit_config.at("disabled").get<bool>()
    : false;
  const auto reward_method =
   reward_parser.parse_method(strategy_json.at("takeProfit"));
  const auto trading_take_profit_method = screener::HighMethod{};
  const auto take_profit = pludux::backtest::TakeProfit{
   reward_method, is_take_profit_disabled, trading_take_profit_method};

  auto risk_parser = risk_reward_config_parser();
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

  return pludux::backtest::Strategy{strategy_name,
                                    capital_risk,
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
     const auto period = parameters.contains("period")
                          ? parameters.at("period").get<std::size_t>()
                          : 14;
     const auto multiplier = parameters.contains("multiplier")
                              ? parameters.at("multiplier").get<double>()
                              : 1;

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

auto format_duration(std::size_t duration_in_seconds) -> std::string
{
  using namespace std::chrono;

  auto secs = seconds(duration_in_seconds);
  auto result = std::string{};

  if(secs.count() <= 0) {
    result = "0";
    return result;
  }

  auto years = duration_cast<days>(secs) / 365;
  if(years.count() > 0) {
    result +=
     std::to_string(years.count()) + (years.count() == 1 ? " year" : " years");
    secs -= days(years.count() * 365);
  }
  auto months = duration_cast<days>(secs) / 30;
  if(months.count() > 0) {
    if(!result.empty())
      result += ", ";
    result += std::to_string(months.count()) +
              (months.count() == 1 ? " month" : " months");
    secs -= days(months.count() * 30);
  }
  auto d = duration_cast<days>(secs);
  if(d.count() > 0) {
    if(!result.empty())
      result += ", ";
    result += std::to_string(d.count()) + (d.count() == 1 ? " day" : " days");
    secs -= d;
  }
  auto h = duration_cast<hours>(secs);
  if(h.count() > 0) {
    if(!result.empty())
      result += ", ";
    result += std::to_string(h.count()) + (h.count() == 1 ? " hour" : " hours");
    secs -= h;
  }
  auto m = duration_cast<minutes>(secs);
  if(m.count() > 0) {
    if(!result.empty())
      result += ", ";
    result +=
     std::to_string(m.count()) + (m.count() == 1 ? " minute" : " minutes");
    secs -= m;
  }
  auto s = duration_cast<seconds>(secs);
  if(s.count() > 0 || result.empty()) {
    if(!result.empty())
      result += ", ";
    result +=
     std::to_string(s.count()) + (s.count() == 1 ? " second" : " seconds");
  }
  return result;
}

auto format_datetime(std::time_t timestamp) -> std::string
{
  using namespace std::chrono;

  const auto tm_ptr = std::localtime(&timestamp);
  const auto& tm = *tm_ptr;

  // Example: "2025-06-29 14:30:00"
  auto formated_datetime = std::format(
   "{:04}-{:02}-{:02}", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

  if(tm.tm_hour != 0 || tm.tm_min != 0) {
    formated_datetime += std::format(" {:02}:{:02}", tm.tm_hour, tm.tm_min);
  }

  return formated_datetime;
}

} // namespace pludux
