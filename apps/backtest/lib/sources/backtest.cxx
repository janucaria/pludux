module;

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
#include <rapidcsv.h>

#include <pludux/backtest.hpp>
#include <pludux/config_parser.hpp>

export module pludux.backtest;

export import pludux.backtest.asset;
export import pludux.backtest.profile;
export import pludux.backtest.trade_session;
export import pludux.backtest.backtest_summary;
export import pludux.backtest.strategy;

export namespace pludux {

class Backtest {
public:
  Backtest(std::string name,
           std::shared_ptr<backtest::Strategy> strategy_ptr,
           std::shared_ptr<backtest::Asset> asset_ptr,
           std::shared_ptr<backtest::Profile> profile_ptr);

  Backtest(std::string name,
           std::shared_ptr<backtest::Strategy> strategy_ptr,
           std::shared_ptr<backtest::Asset> asset_ptr,
           std::shared_ptr<backtest::Profile> profile_ptr,
           std::vector<backtest::BacktestSummary> summaries);

  auto name() const noexcept -> const std::string&;

  auto strategy_ptr() const noexcept
   -> const std::shared_ptr<backtest::Strategy>;

  auto asset_ptr() const noexcept -> const std::shared_ptr<backtest::Asset>;

  auto profile_ptr() const noexcept -> const std::shared_ptr<backtest::Profile>;

  auto strategy() const noexcept -> const backtest::Strategy&;

  auto asset() const noexcept -> const backtest::Asset&;

  auto get_profile() const noexcept -> const backtest::Profile&;

  void mark_as_failed() noexcept;

  auto is_failed() const noexcept -> bool;

  auto summaries() const noexcept
   -> const std::vector<backtest::BacktestSummary>&;

  void reset();

  auto should_run() const noexcept -> bool;

  void run();

private:
  std::string name_;

  std::shared_ptr<backtest::Strategy> strategy_ptr_;
  std::shared_ptr<backtest::Asset> asset_ptr_;
  std::shared_ptr<backtest::Profile> profile_ptr_;

  bool is_failed_;
  std::vector<backtest::BacktestSummary> summaries_;
};

auto get_env_var(std::string_view var_name) -> std::optional<std::string>;

auto csv_daily_stock_data(std::istream& csv_stream) -> AssetHistory;

auto format_duration(std::size_t duration_in_seconds) -> std::string;

auto format_datetime(std::time_t timestamp) -> std::string;

auto format_currency(double value) -> std::string;

// ----------------------------------------------------------------------------

Backtest::Backtest(std::string name,
                   std::shared_ptr<backtest::Strategy> strategy_ptr,
                   std::shared_ptr<backtest::Asset> asset_ptr,
                   std::shared_ptr<backtest::Profile> profile_ptr)
: Backtest{std::move(name),
           std::move(strategy_ptr),
           std::move(asset_ptr),
           std::move(profile_ptr),
           std::vector<backtest::BacktestSummary>{}}
{
}

Backtest::Backtest(std::string name,
                   std::shared_ptr<backtest::Strategy> strategy_ptr,
                   std::shared_ptr<backtest::Asset> asset_ptr,
                   std::shared_ptr<backtest::Profile> profile_ptr,
                   std::vector<backtest::BacktestSummary> summaries)
: name_{std::move(name)}
, strategy_ptr_{strategy_ptr}
, asset_ptr_{asset_ptr}
, profile_ptr_{profile_ptr}
, is_failed_{false}
, summaries_{std::move(summaries)}
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

auto Backtest::profile_ptr() const noexcept
 -> const std::shared_ptr<backtest::Profile>
{
  return profile_ptr_;
}

auto Backtest::strategy() const noexcept -> const backtest::Strategy&
{
  return *strategy_ptr();
}

auto Backtest::asset() const noexcept -> const backtest::Asset&
{
  return *asset_ptr();
}

auto Backtest::get_profile() const noexcept -> const backtest::Profile&
{
  return *profile_ptr();
}

void Backtest::mark_as_failed() noexcept
{
  is_failed_ = true;
}

auto Backtest::is_failed() const noexcept -> bool
{
  return is_failed_;
}

auto Backtest::summaries() const noexcept
 -> const std::vector<backtest::BacktestSummary>&
{
  return summaries_;
}

void Backtest::reset()
{
  is_failed_ = false;
  summaries_.clear();
}

auto Backtest::should_run() const noexcept -> bool
{
  const auto summaries_size = summaries_.size();
  const auto asset_size = asset().size();

  return summaries_size < asset_size && !is_failed();
}

void Backtest::run()
{
  using namespace backtest;

  if(!should_run()) {
    return;
  }

  const auto summaries_size = summaries_.size();
  const auto asset_size = asset().size();
  const auto last_index = asset_size - 1;
  const auto asset_index = last_index - std::min(summaries_size, last_index);
  const auto asset_snapshot = asset().get_snapshot(asset_index);
  const auto asset_timestamp =
   static_cast<std::time_t>(asset_snapshot.get_datetime());
  const auto asset_price = asset_snapshot.get_close();
  const auto& strategy = *strategy_ptr();
  const auto& profile = get_profile();

  auto summary = !summaries_.empty()
                  ? summaries_.back()
                  : BacktestSummary{profile.initial_capital()};

  auto trade_session = summary.trade_session();

  trade_session.market_update(
   static_cast<std::time_t>(asset_snapshot.get_datetime()),
   asset_snapshot.get_close(),
   asset_snapshot.offset());

  trade_session.evaluate_exit_conditions(asset_snapshot[1].get_close(),
                                         asset_snapshot.get_open(),
                                         asset_snapshot.get_high(),
                                         asset_snapshot.get_low());

  const auto& open_position = trade_session.open_position();
  if(open_position) {
    const auto position_size = open_position->unrealized_position_size();
    const auto exit_trade = strategy.exit_trade(asset_snapshot, position_size);

    if(exit_trade) {
      trade_session.exit_position(*exit_trade);
    }
  } else {
    const auto entry_trade =
     strategy.entry_trade(asset_snapshot, profile.get_risk_value());
    if(entry_trade) {
      trade_session.entry_position(*entry_trade);
    }
  }

  summary.update_to_next_summary(std::move(trade_session));

  summaries_.emplace_back(std::move(summary));
}

auto get_env_var(std::string_view var_name) -> std::optional<std::string>
{
  if(const auto env_var = std::getenv(var_name.data()); env_var != nullptr) {
    return std::string{env_var};
  }
  return std::nullopt;
}

auto csv_daily_stock_data(std::istream& csv_stream) -> AssetHistory
{
  auto csv_doc = rapidcsv::Document(csv_stream);
  const auto column_count = csv_doc.GetColumnCount();

  if(column_count == 0) {
    return AssetHistory{};
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

  auto history_data =
   std::vector<std::pair<std::string, pludux::DataSeries<double>>>{};

  const auto date_record_header = csv_doc.GetColumnName(date_record_index);
  history_data.emplace_back(
   date_record_header,
   pludux::DataSeries<double>(date_records.begin(), date_records.end()));

  for(auto i = 1; i < column_count; ++i) {
    auto column = csv_doc.GetColumn<double>(i);
    if(should_reverse) {
      std::reverse(column.begin(), column.end());
    }

    const auto column_header = csv_doc.GetColumnName(i);
    history_data.emplace_back(
     column_header, pludux::DataSeries<double>(column.begin(), column.end()));
  }

  auto result = AssetHistory{history_data.begin(), history_data.end()};
  result.datetime_key(date_record_header);

  return result;
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

} // namespace pludux
