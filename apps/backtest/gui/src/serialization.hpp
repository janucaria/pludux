#ifndef PLUDUX_APPS_BACKTEST_SERIALIZATION_HPP
#define PLUDUX_APPS_BACKTEST_SERIALIZATION_HPP

#include <algorithm>
#include <tuple>
#include <utility>

#include <nlohmann/json.hpp>

#include <cereal/cereal.hpp>

#include <cereal/archives/json.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include <pludux/config_parser.hpp>

#include "./app_state_data.hpp"

namespace cereal {
template<class Archive>
void save(Archive& archive, const nlohmann::json& json_data)
{
  const auto json_str = json_data.dump();
  archive(make_nvp("json", json_str));
}

template<class Archive>
void load(Archive& archive, nlohmann::json& json_data)
{
  auto json_str = std::string{};
  archive(make_nvp("json", json_str));
  json_data = nlohmann::json::parse(json_str);
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void serialize(Archive& archive, pludux::backtest::TradeRecord& trade_record)
{
  const auto status = static_cast<std::size_t>(trade_record.status());
  archive(make_nvp("status", status),
          make_nvp("positionSize", trade_record.position_size()),
          make_nvp("averagePrice", trade_record.average_price()),
          make_nvp("entryTimestamp", trade_record.entry_timestamp()),
          make_nvp("entryPrice", trade_record.entry_price()),
          make_nvp("entryIndex", trade_record.entry_index()),
          make_nvp("exitTimestamp", trade_record.exit_timestamp()),
          make_nvp("exitPrice", trade_record.exit_price()),
          make_nvp("exitIndex", trade_record.exit_index()),
          make_nvp("stopLossPrice", trade_record.stop_loss_price()),
          make_nvp("trailingStopPrice", trade_record.trailing_stop_price()),
          make_nvp("takeProfitPrice", trade_record.take_profit_price()));
}

template<>
struct LoadAndConstruct<pludux::backtest::TradeRecord> {
  template<class Archive>
  static void load_and_construct(
   Archive& archive,
   cereal::construct<pludux::backtest::TradeRecord>& constructor)
  {
    auto status = std::size_t{};
    auto position_size = double{};
    auto average_price = double{};
    auto entry_timestamp = std::time_t{};
    auto entry_price = double{};
    auto entry_index = std::size_t{};
    auto exit_timestamp = std::time_t{};
    auto exit_price = double{};
    auto exit_index = std::size_t{};
    auto stop_loss_price = double{};
    auto trailing_stop_price = double{};
    auto take_profit_price = double{};

    archive(make_nvp("status", status),
            make_nvp("positionSize", position_size),
            make_nvp("averagePrice", average_price),
            make_nvp("entryTimestamp", entry_timestamp),
            make_nvp("entryPrice", entry_price),
            make_nvp("entryIndex", entry_index),
            make_nvp("exitTimestamp", exit_timestamp),
            make_nvp("exitPrice", exit_price),
            make_nvp("exitIndex", exit_index),
            make_nvp("stopLossPrice", stop_loss_price),
            make_nvp("trailingStopPrice", trailing_stop_price),
            make_nvp("takeProfitPrice", take_profit_price));

    constructor(pludux::backtest::TradeRecord::Status(status),
                position_size,
                average_price,
                entry_timestamp,
                entry_price,
                entry_index,
                exit_timestamp,
                exit_price,
                exit_index,
                stop_loss_price,
                trailing_stop_price,
                take_profit_price);
  }
};

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void serialize(Archive& archive,
               pludux::backtest::TradePosition& trade_position)
{
  auto realized_record_ptrs =
   std::vector<std::unique_ptr<const pludux::backtest::TradeRecord,
                               decltype([](auto*) {})>>{};
  for(const auto& record : trade_position.realized_records()) {
    realized_record_ptrs.emplace_back(&record);
  }

  archive(
   make_nvp("positionSize", trade_position.position_size()),
   make_nvp("averagePrice", trade_position.average_price()),
   make_nvp("entryPrice", trade_position.entry_price()),
   make_nvp("stopLossInitialPrice", trade_position.stop_loss_initial_price()),
   make_nvp("stopLossTrailingPrice", trade_position.stop_loss_trailing_price()),
   make_nvp("takeProfitPrice", trade_position.take_profit_price()),
   make_nvp("entryIndex", trade_position.entry_index()),
   make_nvp("entryTimestamp", trade_position.entry_timestamp()),
   make_nvp("realizedRecords", realized_record_ptrs));
}

template<>
struct LoadAndConstruct<pludux::backtest::TradePosition> {
  template<class Archive>
  static void load_and_construct(
   Archive& archive,
   cereal::construct<pludux::backtest::TradePosition>& constructor)
  {
    auto position_size = double{};
    auto average_price = double{};
    auto entry_price = double{};
    auto stop_loss_initial_price = double{};
    auto stop_loss_trailing_price = double{};
    auto take_profit_price = double{};
    auto entry_index = std::size_t{};
    auto entry_timestamp = std::time_t{};
    auto realized_record_ptrs =
     std::vector<std::unique_ptr<const pludux::backtest::TradeRecord>>{};

    archive(make_nvp("positionSize", position_size),
            make_nvp("averagePrice", average_price),
            make_nvp("entryPrice", entry_price),
            make_nvp("stopLossInitialPrice", stop_loss_initial_price),
            make_nvp("stopLossTrailingPrice", stop_loss_trailing_price),
            make_nvp("takeProfitPrice", take_profit_price),
            make_nvp("entryIndex", entry_index),
            make_nvp("entryTimestamp", entry_timestamp),
            make_nvp("realizedRecords", realized_record_ptrs));

    auto realized_records = std::vector<pludux::backtest::TradeRecord>{};
    for(auto& record_ptr : realized_record_ptrs) {
      realized_records.emplace_back(std::move(*record_ptr));
    }

    constructor(position_size,
                average_price,
                entry_timestamp,
                entry_price,
                entry_index,
                stop_loss_initial_price,
                stop_loss_trailing_price,
                take_profit_price,
                std::move(realized_records));
  }
};

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::TradeSession& trade_session)
{
  using NoDeletePtr = std::unique_ptr<const pludux::backtest::TradePosition,
                                      decltype([](auto*) {})>;

  auto open_position_ptr = NoDeletePtr(trade_session.open_position()
                                        ? &trade_session.open_position().value()
                                        : nullptr);
  auto closed_position_ptr = NoDeletePtr(
   trade_session.closed_position() ? &trade_session.closed_position().value()
                                   : nullptr);
  archive(make_nvp("marketTimestamp", trade_session.market_timestamp()),
          make_nvp("marketPrice", trade_session.market_price()),
          make_nvp("marketIndex", trade_session.market_index()),
          make_nvp("openPosition", open_position_ptr),
          make_nvp("closedPosition", closed_position_ptr));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::TradeSession& trade_session)
{
  auto market_timestamp = std::time_t{};
  auto market_price = double{};
  auto market_index = std::size_t{};
  auto open_position_ptr = std::unique_ptr<pludux::backtest::TradePosition>{};
  auto closed_position_ptr = std::unique_ptr<pludux::backtest::TradePosition>{};

  archive(make_nvp("marketTimestamp", market_timestamp),
          make_nvp("marketPrice", market_price),
          make_nvp("marketIndex", market_index),
          make_nvp("openPosition", open_position_ptr),
          make_nvp("closedPosition", closed_position_ptr));

  trade_session.market_timestamp(market_timestamp);
  trade_session.market_price(market_price);
  trade_session.market_index(market_index);
  if(open_position_ptr) {
    trade_session.open_position(std::move(*open_position_ptr));
  }
  if(closed_position_ptr) {
    trade_session.closed_position(std::move(*closed_position_ptr));
  }
}
/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::BacktestingSummary& summary)
{
  archive(
   make_nvp("tradeSession", summary.trade_session()),
   make_nvp("capital", summary.capital()),
   make_nvp("peakEquity", summary.peak_equity()),
   make_nvp("maxDrawdown", summary.max_drawdown()),
   make_nvp("cumulativeInvestments", summary.cumulative_investments()),
   make_nvp("cumulativeTakeProfits", summary.cumulative_take_profits()),
   make_nvp("cumulativeStopLoseProfits",
            summary.cumulative_stop_loss_profits()),
   make_nvp("cumulativeStopLoseLosses", summary.cumulative_stop_loss_losses()),
   make_nvp("cumulativeExitSignalProfits",
            summary.cumulative_exit_signal_profits()),
   make_nvp("cumulativeExitSignalLosses",
            summary.cumulative_exit_signal_losses()),
   make_nvp("cumulativeDurations", summary.cumulative_durations()),
   make_nvp("takeProfitCount", summary.take_profit_count()),
   make_nvp("stopLossProfitCount", summary.stop_loss_profit_count()),
   make_nvp("stopLossLossCount", summary.stop_loss_loss_count()),
   make_nvp("exitSignalProfitCount", summary.exit_signal_profit_count()),
   make_nvp("exitSignalLossCount", summary.exit_signal_loss_count()),
   make_nvp("breakEvenCount", summary.break_even_count()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::BacktestingSummary& summary)
{
  auto trade_session = pludux::backtest::TradeSession{};
  auto capital = double{};
  auto peak_equity = double{};
  auto max_drawdown = double{};
  auto cumulative_investments = double{};
  auto cumulative_take_profits = double{};
  auto cumulative_stop_loss_profits = double{};
  auto cumulative_stop_loss_losses = double{};
  auto cumulative_exit_signal_profits = double{};
  auto cumulative_exit_signal_losses = double{};
  auto cumulative_durations = std::time_t{};
  auto take_profit_count = std::size_t{};
  auto stop_loss_profit_count = std::size_t{};
  auto stop_loss_loss_count = std::size_t{};
  auto exit_signal_profit_count = std::size_t{};
  auto exit_signal_loss_count = std::size_t{};
  auto break_even_count = std::size_t{};

  archive(
   make_nvp("tradeSession", trade_session),
   make_nvp("capital", capital),
   make_nvp("peakEquity", peak_equity),
   make_nvp("maxDrawdown", max_drawdown),
   make_nvp("cumulativeInvestments", cumulative_investments),
   make_nvp("cumulativeTakeProfits", cumulative_take_profits),
   make_nvp("cumulativeStopLoseProfits", cumulative_stop_loss_profits),
   make_nvp("cumulativeStopLoseLosses", cumulative_stop_loss_losses),
   make_nvp("cumulativeExitSignalProfits", cumulative_exit_signal_profits),
   make_nvp("cumulativeExitSignalLosses", cumulative_exit_signal_losses),
   make_nvp("cumulativeDurations", cumulative_durations),
   make_nvp("takeProfitCount", take_profit_count),
   make_nvp("stopLossProfitCount", stop_loss_profit_count),
   make_nvp("stopLossLossCount", stop_loss_loss_count),
   make_nvp("exitSignalProfitCount", exit_signal_profit_count),
   make_nvp("exitSignalLossCount", exit_signal_loss_count),
   make_nvp("breakEvenCount", break_even_count));

  auto new_summary = pludux::backtest::BacktestingSummary{};
  new_summary.trade_session(std::move(trade_session));
  new_summary.capital(capital);
  new_summary.peak_equity(peak_equity);
  new_summary.max_drawdown(max_drawdown);
  new_summary.cumulative_investments(cumulative_investments);
  new_summary.cumulative_take_profits(cumulative_take_profits);
  new_summary.cumulative_stop_loss_profits(cumulative_stop_loss_profits);
  new_summary.cumulative_stop_loss_losses(cumulative_stop_loss_losses);
  new_summary.cumulative_exit_signal_profits(cumulative_exit_signal_profits);
  new_summary.cumulative_exit_signal_losses(cumulative_exit_signal_losses);
  new_summary.cumulative_durations(cumulative_durations);
  new_summary.take_profit_count(take_profit_count);
  new_summary.stop_loss_profit_count(stop_loss_profit_count);
  new_summary.stop_loss_loss_count(stop_loss_loss_count);
  new_summary.exit_signal_profit_count(exit_signal_profit_count);
  new_summary.exit_signal_loss_count(exit_signal_loss_count);
  new_summary.break_even_count(break_even_count);

  summary = std::move(new_summary);
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void serialize(Archive& archive, pludux::backtest::Strategy& strategy)
{
  auto risk_parser = pludux::backtest::risk_reward_config_parser();
  const auto risk_method_json =
   risk_parser.serialize_method(strategy.risk_method());

  auto config_parser = pludux::ConfigParser{};
  config_parser.register_default_parsers();

  const auto long_entry_repeat =
   static_cast<std::size_t>(strategy.long_entry_repeat());
  const auto long_entry_filter_json =
   config_parser.serialize_filter(strategy.long_entry_filter());
  const auto long_exit_filter_json =
   config_parser.serialize_filter(strategy.long_exit_filter());

  const auto short_entry_repeat =
   static_cast<std::size_t>(strategy.short_entry_repeat());
  const auto short_entry_filter_json =
   config_parser.serialize_filter(strategy.short_entry_filter());
  const auto short_exit_filter_json =
   config_parser.serialize_filter(strategy.short_exit_filter());

  const auto stop_loss_enabled = strategy.stop_loss_enabled();
  const auto stop_loss_trailing_enabled = strategy.stop_loss_trailing_enabled();
  const auto stop_loss_risk_multiplier = strategy.stop_loss_risk_multiplier();

  const auto take_profit_enabled = strategy.take_profit_enabled();
  const auto take_profit_risk_multiplier =
   strategy.take_profit_risk_multiplier();

  archive(make_nvp("name", strategy.name()),
          make_nvp("riskMethod", risk_method_json),
          make_nvp("longEntryRepeat", long_entry_repeat),
          make_nvp("longEntryFilter", long_entry_filter_json),
          make_nvp("longExitFilter", long_exit_filter_json),
          make_nvp("shortEntryRepeat", short_entry_repeat),
          make_nvp("shortEntryFilter", short_entry_filter_json),
          make_nvp("shortExitFilter", short_exit_filter_json),
          make_nvp("stopLossEnabled", stop_loss_enabled),
          make_nvp("stopLossTrailingEnabled", stop_loss_trailing_enabled),
          make_nvp("stopLossRiskMultiplier", stop_loss_risk_multiplier),
          make_nvp("takeProfitEnabled", take_profit_enabled),
          make_nvp("takeProfitRiskMultiplier", take_profit_risk_multiplier));
}

template<>
struct LoadAndConstruct<pludux::backtest::Strategy> {
  template<class Archive>
  static void
  load_and_construct(Archive& archive,
                     construct<pludux::backtest::Strategy>& constructor)
  {
    auto name = std::string{};
    auto risk_method_json = nlohmann::json{};
    auto long_entry_repeat = std::size_t{};
    auto long_entry_filter_json = nlohmann::json{};
    auto long_exit_filter_json = nlohmann::json{};
    auto short_entry_repeat = std::size_t{};
    auto short_entry_filter_json = nlohmann::json{};
    auto short_exit_filter_json = nlohmann::json{};
    auto stop_loss_enabled = bool{};
    auto stop_loss_trailing_enabled = bool{};
    auto stop_loss_risk_multiplier = double{};
    auto take_profit_enabled = bool{};
    auto take_profit_risk_multiplier = double{};

    archive(make_nvp("name", name),
            make_nvp("riskMethod", risk_method_json),
            make_nvp("longEntryRepeat", long_entry_repeat),
            make_nvp("longEntryFilter", long_entry_filter_json),
            make_nvp("longExitFilter", long_exit_filter_json),
            make_nvp("shortEntryRepeat", short_entry_repeat),
            make_nvp("shortEntryFilter", short_entry_filter_json),
            make_nvp("shortExitFilter", short_exit_filter_json),
            make_nvp("stopLossEnabled", stop_loss_enabled),
            make_nvp("stopLossTrailingEnabled", stop_loss_trailing_enabled),
            make_nvp("stopLossRiskMultiplier", stop_loss_risk_multiplier),
            make_nvp("takeProfitEnabled", take_profit_enabled),
            make_nvp("takeProfitRiskMultiplier", take_profit_risk_multiplier));

    auto risk_parser = pludux::backtest::risk_reward_config_parser();
    auto risk_method = risk_parser.parse_method(risk_method_json);

    auto config_parser = pludux::ConfigParser{};
    config_parser.register_default_parsers();

    auto long_entry_filter = config_parser.parse_filter(long_entry_filter_json);
    auto long_exit_filter = config_parser.parse_filter(long_exit_filter_json);
    auto short_entry_filter =
     config_parser.parse_filter(short_entry_filter_json);
    auto short_exit_filter = config_parser.parse_filter(short_exit_filter_json);

    constructor(std::move(name),
                std::move(risk_method),
                pludux::backtest::Strategy::EntryRepeat(long_entry_repeat),
                std::move(long_entry_filter),
                std::move(long_exit_filter),
                pludux::backtest::Strategy::EntryRepeat(short_entry_repeat),
                std::move(short_entry_filter),
                std::move(short_exit_filter),
                stop_loss_enabled,
                stop_loss_trailing_enabled,
                stop_loss_risk_multiplier,
                take_profit_enabled,
                take_profit_risk_multiplier);
  }
};

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void serialize(Archive& archive, pludux::backtest::Profile& profile)
{
  archive(make_nvp("name", profile.name()),
          make_nvp("initialCapital", profile.initial_capital()),
          make_nvp("capitalRisk", profile.capital_risk()));
}

template<>
struct LoadAndConstruct<pludux::backtest::Profile> {
  template<class Archive>
  static void
  load_and_construct(Archive& archive,
                     construct<pludux::backtest::Profile>& constructor)
  {
    auto name = std::string{};
    auto initial_capital = double{};
    auto capital_risk = double{};

    archive(make_nvp("name", name),
            make_nvp("initialCapital", initial_capital),
            make_nvp("capitalRisk", capital_risk));

    auto profile = pludux::backtest::Profile{std::move(name)};
    profile.initial_capital(initial_capital);
    profile.capital_risk(capital_risk);

    constructor(std::move(profile));
  }
};

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::DataSeries<double>& data_series)
{
  archive(make_nvp("data", data_series.data()));
}

template<class Archive>
void load(Archive& archive, pludux::DataSeries<double>& data_series)
{
  auto data = std::vector<double>{};

  archive(make_nvp("data", data));

  data_series.data(std::move(data));
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void serialize(Archive& archive, pludux::PolySeries<double>& series)
{
  const auto data_series = screener_cast<pludux::DataSeries<double>>(&series);
  archive(make_nvp("series", *data_series));
}

template<>
struct LoadAndConstruct<pludux::PolySeries<double>> {
  template<class Archive>
  static void
  load_and_construct(Archive& archive,
                     construct<pludux::PolySeries<double>>& constructor)
  {
    auto data_series = pludux::DataSeries<double>{};
    archive(make_nvp("series", data_series));
    constructor(std::move(data_series));
  }
};

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::AssetHistory& asset_history)
{
  const auto& history_data = asset_history.history_data();

  auto history_data_map =
   std::unordered_map<std::string,
                      std::unique_ptr<pludux::PolySeries<double>>>{};
  for(const auto& [key, series] : history_data) {
    history_data_map[key] =
     std::make_unique<pludux::PolySeries<double>>(series);
  }

  archive(make_nvp("historyData", history_data_map));
  archive(make_nvp("datetimeKey", asset_history.datetime_key()));
  archive(make_nvp("openKey", asset_history.open_key()));
  archive(make_nvp("highKey", asset_history.high_key()));
  archive(make_nvp("lowKey", asset_history.low_key()));
  archive(make_nvp("closeKey", asset_history.close_key()));
  archive(make_nvp("volumeKey", asset_history.volume_key()));
}

template<class Archive>
void load(Archive& archive, pludux::AssetHistory& asset_history)
{
  auto history_data_map =
   std::unordered_map<std::string,
                      std::unique_ptr<pludux::PolySeries<double>>>{};

  archive(make_nvp("historyData", history_data_map));

  for(auto& [key, series] : history_data_map) {
    asset_history.insert(key, std::move(*series));
  }

  auto datetime_key = std::string{};
  auto open_key = std::string{};
  auto high_key = std::string{};
  auto low_key = std::string{};
  auto close_key = std::string{};
  auto volume_key = std::string{};
  archive(make_nvp("datetimeKey", datetime_key),
          make_nvp("openKey", open_key),
          make_nvp("highKey", high_key),
          make_nvp("lowKey", low_key),
          make_nvp("closeKey", close_key),
          make_nvp("volumeKey", volume_key));

  asset_history.datetime_key(std::move(datetime_key));
  asset_history.open_key(std::move(open_key));
  asset_history.high_key(std::move(high_key));
  asset_history.low_key(std::move(low_key));
  asset_history.close_key(std::move(close_key));
  asset_history.volume_key(std::move(volume_key));
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void serialize(Archive& archive, const pludux::backtest::Asset& asset)
{
  archive(make_nvp("name", asset.name()), make_nvp("history", asset.history()));
}

template<>
struct LoadAndConstruct<pludux::backtest::Asset> {
  template<class Archive>
  static void
  load_and_construct(Archive& archive,
                     construct<pludux::backtest::Asset>& constructor)
  {
    auto name = std::string{};
    auto history = pludux::AssetHistory{};

    archive(make_nvp("name", name), make_nvp("history", history));

    constructor(std::move(name), history);
  }
};

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void serialize(Archive& archive, pludux::Backtest& backtest)
{
  archive(make_nvp("name", backtest.name()),
          make_nvp("strategy", backtest.strategy_ptr()),
          make_nvp("asset", backtest.asset_ptr()),
          make_nvp("profile", backtest.profile_ptr()),
          make_nvp("summaries", backtest.summaries()),
          make_nvp("isFailed", backtest.is_failed()));
}

template<>
struct LoadAndConstruct<pludux::Backtest> {
  template<class Archive>
  static void load_and_construct(Archive& archive,
                                 construct<pludux::Backtest>& constructor)
  {
    auto name = std::string{};
    auto strategy_ptr = std::shared_ptr<pludux::backtest::Strategy>{};
    auto asset_ptr = std::shared_ptr<pludux::backtest::Asset>{};
    auto profile_ptr = std::shared_ptr<pludux::backtest::Profile>{};
    auto summaries = std::vector<pludux::backtest::BacktestingSummary>{};
    auto is_failed = bool{};

    archive(make_nvp("name", name),
            make_nvp("strategy", strategy_ptr),
            make_nvp("asset", asset_ptr),
            make_nvp("profile", profile_ptr),
            make_nvp("summaries", summaries),
            make_nvp("isFailed", is_failed));

    auto backtest = pludux::Backtest{std::move(name),
                                     strategy_ptr,
                                     asset_ptr,
                                     profile_ptr,
                                     std::move(summaries)};

    if(is_failed) {
      backtest.mark_as_failed();
    }

    constructor(std::move(backtest));
  }
};

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::apps::AppStateData& app_state_data)
{
  const auto& assets = app_state_data.assets;
  const auto& strategies = app_state_data.strategies;
  const auto& profiles = app_state_data.profiles;
  const auto& backtests = app_state_data.backtests;

  archive(make_nvp("alertMessages", app_state_data.alert_messages));
  archive(
   make_nvp("selectedBacktestIndex", app_state_data.selected_backtest_index));
  archive(make_nvp("strategies", app_state_data.strategies));
  archive(make_nvp("assets", app_state_data.assets));
  archive(make_nvp("profiles", app_state_data.profiles));

  auto backtest_ptrs = std::vector<
   std::unique_ptr<const pludux::Backtest, decltype([](auto*) {})>>{};
  for(const auto& backtest : backtests) {
    backtest_ptrs.emplace_back(&backtest);
  }
  archive(make_nvp("backtests", backtest_ptrs));
}

template<class Archive>
void load(Archive& archive, pludux::apps::AppStateData& app_state_data)
{
  archive(make_nvp("alertMessages", app_state_data.alert_messages));
  archive(
   make_nvp("selectedBacktestIndex", app_state_data.selected_backtest_index));
  archive(make_nvp("strategies", app_state_data.strategies));
  archive(make_nvp("assets", app_state_data.assets));
  archive(make_nvp("profiles", app_state_data.profiles));

  auto backtest_ptrs = std::vector<std::unique_ptr<pludux::Backtest>>{};
  archive(make_nvp("backtests", backtest_ptrs));
  app_state_data.backtests.clear();
  for(auto& backtest_ptr : backtest_ptrs) {
    if(backtest_ptr) {
      app_state_data.backtests.emplace_back(std::move(*backtest_ptr));
    }
  }
}

} // namespace cereal

#endif
