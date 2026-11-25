module;

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <format>
#include <iostream>
#include <limits>
#include <optional>
#include <vector>

#include <jsoncons/json.hpp>
#include <rapidcsv.h>

export module pludux.backtest;

export import pludux;

export import :store_handle;
export import :store_slot;
export import :store_data_resolver;
export import :store_descriptor;
export import :store_arena;
export import :store;
export import :calls;
export import :call_csv_import;
import :table_field_parsing;
export import :execution_model;
export import :intrabar_path;
export import :asset;
export import :watchlist;
export import :strategy;
export import :strategy_inputs;
export import :market;
export import :broker;
export import :position_sizing;
export import :entry_order_sizing;
export import :requested_order;
export import :requested_order_method_context;
export import :requested_order_method;
export import :requested_order_node;
export import :profile;
export import :portfolio;
export import :trade_entry;
export import :trade_exit;
export import :take_profit_level;
export import :signal_exit_state;
export import :stop_loss_level;
export import :trade_event;
export import :open_position_snapshot;
export import :closed_trade;
export import :trade_position;
export import :trade_session;
export import :strategy_intent;
export import :strategy_closed_position;
export import :strategy_session;
export import :strategy_performance_model;
export import :strategy_performance;
export import :entry_filter_method_context;
export import :strategy_performance_method;
export import :strategy_performance_node;
export import :strategy_setup;
export import :system;
export import :backtest_timeline;
export import :portfolio_timeline;
export import :portfolio_results;
export import :backtest_method_context;
export import :drawdown_method;
export import :drawdown_node;
export import :equity_method;
export import :equity_node;
export import :risk_distance_method;
export import :risk_distance_node;
export import :position_method;
export import :position_node;
export import :call_node;
export import :pyramiding_layer_method;
export import :pyramiding_layer_node;
export import :stop_target_price_method;
export import :stop_target_price_node;
export import :backtest_runner;
export import :portfolio_runner;
export import :plot_group;
export import :plots;

export import :config_parser;
export import :strategy_parser;

export namespace pludux {

auto get_env_var(std::string_view var_name) -> std::optional<std::string>
{
  if(const auto env_var = std::getenv(var_name.data()); env_var != nullptr) {
    return std::string{env_var};
  }
  return std::nullopt;
}

void update_asset_from_csv(backtest::Asset& asset, std::istream& csv_stream)
{
  auto csv_doc = rapidcsv::Document(csv_stream);
  const auto column_count = csv_doc.GetColumnCount();

  if(column_count == 0) {
    return;
  }

  constexpr auto date_record_index = 0;
  const auto first_records = csv_doc.GetColumn<std::string>(date_record_index);
  auto date_records = std::vector<double>{};

  std::ranges::transform(first_records,
                         std::back_inserter(date_records),
                         backtest::parse_candle_datetime);

  const auto should_reverse =
   !date_records.empty() && date_records.back() < date_records.front();
  if(should_reverse) {
    std::reverse(date_records.begin(), date_records.end());
  }

  auto field_data = std::vector<std::pair<std::string, pludux::AssetData>>{};

  const auto date_record_header = csv_doc.GetColumnName(date_record_index);
  field_data.emplace_back(
   date_record_header,
   pludux::AssetData(date_records.begin(), date_records.end()));

  for(auto i = 1; i < column_count; ++i) {
    auto column = csv_doc.GetColumn<double>(i);
    if(should_reverse) {
      std::reverse(column.begin(), column.end());
    }

    const auto column_header = csv_doc.GetColumnName(i);
    field_data.emplace_back(column_header,
                            pludux::AssetData(column.begin(), column.end()));
  }

  auto asset_history = AssetHistory{field_data.begin(), field_data.end()};

  auto asset_field_resolver = AssetQuoteFieldResolver{};
  asset_field_resolver.datetime_field(date_record_header);

  asset.history(std::move(asset_history));
  asset.field_resolver(std::move(asset_field_resolver));
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

  // Example: "29 Jan 2025 14:30:00"
  auto formated_datetime = std::format(
   "{:02} {:s} {:04}",
   tm.tm_mday,
   std::string{"Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec"}.substr(
    tm.tm_mon * 4, 3),
   tm.tm_year + 1900);

  if(tm.tm_hour != 0 || tm.tm_min != 0) {
    formated_datetime += std::format(" {:02}:{:02}", tm.tm_hour, tm.tm_min);
  }

  return formated_datetime;
}

auto format_currency(double value) -> std::string
{
  auto num = std::format("{:.2f}", std::abs(value));
  auto pos = static_cast<int>(num.find('.')) - 3;
  while(pos > 0) {
    num.insert(pos, ",");
    pos -= 3;
  }
  if(value < 0) {
    num = "-" + num;
  }
  return num;
}

auto format_compact_integer(double value) -> std::string
{
  if(value == 0) {
    return "0";
  }
  const auto v =
   std::array<double, 5>{1000000000000, 1000000000, 1000000, 1000, 1};
  const auto p = std::array<const char*, 5>{"T", "B", "M", "k", ""};
  for(int i = 0; i < v.size(); ++i) {
    if(std::abs(value) >= v[i]) {
      return std::format("{:d}{}", static_cast<int>(value / v[i]), p[i]);
    }
  }
  return std::format("{:d}{}", static_cast<int>(value / v[4]), p[4]);
}

} // namespace pludux
