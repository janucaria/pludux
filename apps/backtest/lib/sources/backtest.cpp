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

namespace pludux {

Backtest::Backtest(std::string name,
                   std::shared_ptr<backtest::Strategy> strategy_ptr,
                   std::shared_ptr<backtest::Asset> asset_ptr,
                   std::shared_ptr<backtest::Profile> profile_ptr)
: Backtest{std::move(name),
           std::move(strategy_ptr),
           std::move(asset_ptr),
           std::move(profile_ptr),
           std::vector<backtest::BacktestingSummary>{}}
{
}

Backtest::Backtest(std::string name,
                   std::shared_ptr<backtest::Strategy> strategy_ptr,
                   std::shared_ptr<backtest::Asset> asset_ptr,
                   std::shared_ptr<backtest::Profile> profile_ptr,
                   std::vector<backtest::BacktestingSummary> summaries)
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
 -> const std::vector<backtest::BacktestingSummary>&
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
  using backtest::TradeAction;
  using backtest::TradeSession;

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

  const auto& entry_filter = strategy.entry_filter();
  const auto& take_profit = strategy.take_profit();
  const auto& stop_loss = strategy.stop_loss();

  auto summary = !summaries_.empty()
                  ? summaries_.back()
                  : backtest::BacktestingSummary{profile.initial_capital()};

  auto trade_session = summary.trade_session();

  trade_session.market_update(asset_index, asset_timestamp, asset_price);

  if(trade_session.is_closed()) {
    trade_session = TradeSession{};
  }

  if((!trade_session.is_open() || strategy.is_entry_repeat_always()) &&
     entry_filter(asset_snapshot)) {
    const auto risk_size = stop_loss.get_risk_size(asset_snapshot);
    const auto order_size = profile.get_risk_value() / risk_size;

    const auto trading_take_profit = take_profit(asset_snapshot);
    const auto trading_stop_loss = stop_loss(asset_snapshot);

    const auto position_size = order_size;
    const auto entry_index = asset_index;
    const auto entry_price = asset_price;
    const auto entry_timestamp = asset_timestamp;
    const auto initial_stop_loss_price = trading_stop_loss.initial_exit_price();
    const auto trailing_stop_price =
     trading_stop_loss.is_trailing() ? trading_stop_loss.exit_price()
                                     : std::numeric_limits<double>::quiet_NaN();
    const auto take_profit_price = trading_take_profit.exit_price();

    trade_session.emplace_action(position_size,
                                 entry_timestamp,
                                 entry_price,
                                 entry_index,
                                 TradeAction::Reason::entry);

    trade_session.take_profit_price(take_profit_price);
    trade_session.stop_loss_price(initial_stop_loss_price);
    trade_session.trailing_stop_price(trailing_stop_price);

  } else if(trade_session.is_open()) {
    const auto entry_index = trade_session.entry_index();
    const auto position_size = trade_session.position_size();
    const auto exit_timestamp = asset_timestamp;

    const auto& entry_snapshot = asset().get_snapshot(entry_index);
    auto trading_take_profit = take_profit(entry_snapshot);
    auto trading_stop_loss = stop_loss(entry_snapshot);

    if(trading_stop_loss.is_trailing()) {
      trading_stop_loss.stop_price(trade_session.trailing_stop_price());
    }

    const auto exit_filter = strategy.exit_filter();
    const auto trade_action_type_opt =
     [&]() -> std::optional<TradeAction::Reason> {
      if(trading_stop_loss(asset_snapshot)) {
        return TradeAction::Reason::stop_loss;
      }

      if(trading_take_profit(asset_snapshot)) {
        return TradeAction::Reason::take_profit;
      }

      if(exit_filter(asset_snapshot)) {
        return TradeAction::Reason::exit;
      }

      return std::nullopt;
    }();

    const auto stop_loss_price = trading_stop_loss.exit_price();
    const auto take_profit_price = trading_take_profit.exit_price();

    const auto initial_stop_loss_price = trading_stop_loss.initial_exit_price();
    const auto trailing_stop_price =
     trading_stop_loss.is_trailing() ? stop_loss_price
                                     : std::numeric_limits<double>::quiet_NaN();
    trade_session.trailing_stop_price(trailing_stop_price);

    trade_action_type_opt.and_then(
     [&](const TradeAction::Reason& trade_action_type)
      -> std::optional<TradeAction::Reason> {
       const auto exit_price = [&]() -> double {
         switch(trade_action_type) {
         case TradeAction::Reason::take_profit:
           return take_profit_price;
         case TradeAction::Reason::stop_loss:
           return stop_loss_price;
         default:
           return asset_price;
         }
       }();

       trade_session.emplace_action(-position_size,
                                    exit_timestamp,
                                    exit_price,
                                    asset_index,
                                    trade_action_type);

       return std::nullopt;
     });
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
