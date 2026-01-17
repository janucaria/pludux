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

export module pludux.backtest;

export import pludux;

export import :asset;
export import :profile;
export import :trade_entry;
export import :trade_exit;
export import :trade_record;
export import :trade_position;
export import :trade_session;
export import :backtest_summary;
export import :strategy;
export import :broker;
export import :market;

export namespace pludux {

class Backtest {
public:
  Backtest(std::string name,
           std::shared_ptr<backtest::Asset> asset_ptr,
           std::shared_ptr<backtest::Strategy> strategy_ptr,
           std::shared_ptr<backtest::Market> market_ptr,
           std::shared_ptr<backtest::Broker> broker_ptr,
           std::shared_ptr<backtest::Profile> profile_ptr)
  : Backtest{std::move(name),
             std::move(asset_ptr),
             std::move(strategy_ptr),
             std::move(market_ptr),
             std::move(broker_ptr),
             std::move(profile_ptr),
             std::vector<backtest::BacktestSummary>{}}
  {
  }

  Backtest(std::string name,
           std::shared_ptr<backtest::Asset> asset_ptr,
           std::shared_ptr<backtest::Strategy> strategy_ptr,
           std::shared_ptr<backtest::Market> market_ptr,
           std::shared_ptr<backtest::Broker> broker_ptr,
           std::shared_ptr<backtest::Profile> profile_ptr,
           std::vector<backtest::BacktestSummary> summaries)
  : name_{std::move(name)}
  , asset_ptr_{asset_ptr}
  , strategy_ptr_{strategy_ptr}
  , market_ptr_{market_ptr}
  , broker_ptr_{broker_ptr}
  , profile_ptr_{profile_ptr}
  , is_failed_{false}
  , summaries_{std::move(summaries)}
  {
  }

  auto name(this const Backtest& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Backtest& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

  auto strategy_ptr(this const Backtest& self) noexcept
   -> const std::shared_ptr<backtest::Strategy>
  {
    return self.strategy_ptr_;
  }

  void
  strategy_ptr(this Backtest& self,
               std::shared_ptr<backtest::Strategy> new_strategy_ptr) noexcept
  {
    self.strategy_ptr_ = std::move(new_strategy_ptr);
  }

  auto strategy(this const Backtest& self) noexcept -> const backtest::Strategy&
  {
    return *self.strategy_ptr();
  }

  auto asset_ptr(this const Backtest& self) noexcept
   -> const std::shared_ptr<backtest::Asset>
  {
    return self.asset_ptr_;
  }

  void asset_ptr(this Backtest& self,
                 std::shared_ptr<backtest::Asset> new_asset_ptr) noexcept
  {
    self.asset_ptr_ = std::move(new_asset_ptr);
  }

  auto asset(this const Backtest& self) noexcept -> const backtest::Asset&
  {
    return *self.asset_ptr();
  }

  auto broker_ptr(this const Backtest& self) noexcept
   -> const std::shared_ptr<backtest::Broker>
  {
    return self.broker_ptr_;
  }

  void broker_ptr(this Backtest& self,
                  std::shared_ptr<backtest::Broker> new_broker_ptr) noexcept
  {
    self.broker_ptr_ = std::move(new_broker_ptr);
  }

  auto broker(this const Backtest& self) noexcept -> const backtest::Broker&
  {
    return *self.broker_ptr();
  }

  auto market_ptr(this const Backtest& self) noexcept
   -> const std::shared_ptr<backtest::Market>
  {
    return self.market_ptr_;
  }

  void market_ptr(this Backtest& self,
                  std::shared_ptr<backtest::Market> new_market_ptr) noexcept
  {
    self.market_ptr_ = std::move(new_market_ptr);
  }

  auto market(this const Backtest& self) noexcept -> const backtest::Market&
  {
    return *self.market_ptr();
  }

  auto profile_ptr(this const Backtest& self) noexcept
   -> const std::shared_ptr<backtest::Profile>
  {
    return self.profile_ptr_;
  }

  void profile_ptr(this Backtest& self,
                   std::shared_ptr<backtest::Profile> new_profile_ptr) noexcept
  {
    self.profile_ptr_ = std::move(new_profile_ptr);
  }

  auto profile(this const Backtest& self) noexcept -> const backtest::Profile&
  {
    return *self.profile_ptr();
  }

  void mark_as_failed(this Backtest& self) noexcept
  {
    self.is_failed_ = true;
  }

  auto is_failed(this const Backtest& self) noexcept -> bool
  {
    return self.is_failed_;
  }

  auto summaries(this const Backtest& self) noexcept
   -> const std::vector<backtest::BacktestSummary>&
  {
    return self.summaries_;
  }

  void reset(this Backtest& self) noexcept
  {
    self.is_failed_ = false;
    self.summaries_.clear();
  }

  auto should_run(this const Backtest& self) noexcept -> bool
  {
    const auto summaries_size = self.summaries_.size();
    const auto asset_size = self.asset().size();

    return summaries_size < asset_size && !self.is_failed();
  }

  void run(this Backtest& self)
  {
    using namespace backtest;

    if(!self.should_run()) {
      return;
    }

    const auto summaries_size = self.summaries_.size();
    const auto asset_size = self.asset().size();
    const auto last_index = asset_size - 1;
    const auto asset_index = last_index - std::min(summaries_size, last_index);
    const auto asset_snapshot = self.asset().get_snapshot(asset_index);
    const auto asset_timestamp =
     static_cast<std::time_t>(asset_snapshot.datetime());
    const auto asset_price = asset_snapshot.close();
    const auto& strategy = self.strategy();
    const auto& profile = self.profile();
    const auto& broker = self.broker();
    const auto& market = self.market();

    auto summary = !self.summaries_.empty()
                    ? self.summaries_.back()
                    : BacktestSummary{profile.initial_capital()};

    auto trade_session = summary.trade_session();

    trade_session.market_update(
     static_cast<std::time_t>(asset_snapshot.datetime()),
     asset_snapshot.close(),
     asset_snapshot.lookback());

    const auto& open_position = trade_session.open_position();
    if(open_position) {
      const auto exit_trade =
       trade_session
        .evaluate_exit_conditions(asset_snapshot[1].close(),
                                  asset_snapshot.open(),
                                  asset_snapshot.high(),
                                  asset_snapshot.low())
        .or_else([&]() {
          const auto position_size = open_position->unrealized_position_size();
          return self.exit_trade(asset_snapshot, position_size);
        });

      if(exit_trade) {
        const auto fee = broker.calculate_fee(*exit_trade);
        trade_session.exit_position(*exit_trade, fee);
      }
    } else {
      auto entry_trade =
       self.entry_trade(asset_snapshot, profile.get_risk_value());

      if(entry_trade) {
        {
          const auto qty_step = market.qty_step();
          const auto min_order_qty = market.min_order_qty();

          auto position_size = entry_trade->position_size();
          if(qty_step > 0.0 &&
             std::fmod(entry_trade->position_size(), qty_step) != 0.0) {
            position_size =
             qty_step * std::round(entry_trade->position_size() / qty_step);
          }

          if(position_size < min_order_qty) {
            position_size = min_order_qty;
          }

          entry_trade->position_size(position_size);
        }

        const auto fee = broker.calculate_fee(*entry_trade);
        trade_session.entry_position(*entry_trade, fee);
      }
    }

    summary.update_to_next_summary(std::move(trade_session));

    self.summaries_.emplace_back(std::move(summary));
  }

  auto entry_long_trade(this Backtest& self,
                        const AssetSnapshot& asset_snapshot,
                        double risk_value) noexcept
   -> std::optional<backtest::TradeEntry>
  {
    auto result = std::optional<backtest::TradeEntry>{};

    const auto& strategy = self.strategy();
    auto context = self.create_default_method_context();

    if(strategy.long_entry_filter()(asset_snapshot, context)) {
      const auto risk_size = strategy.risk_method()(asset_snapshot, context);
      const auto position_size = risk_value / risk_size;
      const auto entry_price = asset_snapshot.close();

      const auto stop_loss_price =
       strategy.stop_loss_enabled() ? entry_price - risk_size : NAN;
      const auto is_stop_loss_trailing = strategy.stop_loss_trailing_enabled();
      const auto take_profit_price =
       strategy.take_profit_enabled()
        ? entry_price + risk_size * strategy.take_profit_risk_multiplier()
        : NAN;

      result = backtest::TradeEntry{position_size,
                                    entry_price,
                                    stop_loss_price,
                                    is_stop_loss_trailing,
                                    take_profit_price};
    }

    return result;
  }

  auto entry_short_trade(this Backtest& self,
                         const AssetSnapshot& asset_snapshot,
                         double risk_value) noexcept
   -> std::optional<backtest::TradeEntry>
  {
    auto result = std::optional<backtest::TradeEntry>{};

    const auto& strategy = self.strategy();
    auto context = self.create_default_method_context();

    if(strategy.short_entry_filter()(asset_snapshot, context)) {
      const auto risk_size = -strategy.risk_method()(asset_snapshot, context);
      const auto position_size = risk_value / risk_size;
      const auto entry_price = asset_snapshot.close();

      const auto stop_loss_price =
       strategy.stop_loss_enabled() ? entry_price - risk_size : NAN;
      const auto is_stop_loss_trailing = strategy.stop_loss_trailing_enabled();
      const auto take_profit_price =
       strategy.take_profit_enabled()
        ? entry_price + risk_size * strategy.take_profit_risk_multiplier()
        : NAN;

      result = backtest::TradeEntry{position_size,
                                    entry_price,
                                    stop_loss_price,
                                    is_stop_loss_trailing,
                                    take_profit_price};
    }

    return result;
  }

  auto entry_trade(this Backtest& self,
                   const AssetSnapshot& asset_snapshot,
                   double risk_value) noexcept
   -> std::optional<backtest::TradeEntry>
  {
    return self.entry_long_trade(asset_snapshot, risk_value)
     .or_else([=] mutable {
       return self.entry_short_trade(asset_snapshot, risk_value);
     });
  }

  auto exit_trade(this Backtest& self,
                  const AssetSnapshot& asset_snapshot,
                  double position_size) noexcept
   -> std::optional<backtest::TradeExit>
  {
    const auto is_long_direction = position_size > 0;
    const auto is_short_direction = position_size < 0;
    const auto exit_price = asset_snapshot.close();

    auto const& strategy = self.strategy();
    auto context = self.create_default_method_context();

    if(is_long_direction) {
      if(strategy.long_exit_filter()(asset_snapshot, context)) {
        return backtest::TradeExit{
         position_size, exit_price, backtest::TradeExit::Reason::signal};
      }
    } else if(is_short_direction) {
      if(strategy.short_exit_filter()(asset_snapshot, context)) {
        const auto exit_price = asset_snapshot.close();
        return backtest::TradeExit{
         position_size, exit_price, backtest::TradeExit::Reason::signal};
      }
    }

    return std::nullopt;
  }

private:
  std::string name_;

  std::shared_ptr<backtest::Asset> asset_ptr_;
  std::shared_ptr<backtest::Strategy> strategy_ptr_;
  std::shared_ptr<backtest::Market> market_ptr_;
  std::shared_ptr<backtest::Broker> broker_ptr_;
  std::shared_ptr<backtest::Profile> profile_ptr_;

  bool is_failed_;
  std::vector<backtest::BacktestSummary> summaries_;

  auto create_default_method_context(this const Backtest& self)
   -> DefaultMethodContext
  {
    return DefaultMethodContext{self.strategy().series_registry(),
                                self.summaries_.size()};
  }
};

// ----------------------------------------------------------------

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

  auto result = AssetHistory{field_data.begin(), field_data.end()};
  result.datetime_field(date_record_header);

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
