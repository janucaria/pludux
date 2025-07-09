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
void save(Archive& archive, const pludux::backtest::TradeRecord& trade_record)
{
  const auto status = static_cast<std::size_t>(trade_record.status());
  archive(make_nvp("status", status),
          make_nvp("positionSize", trade_record.position_size()),
          make_nvp("entryIndex", trade_record.entry_index()),
          make_nvp("atIndex", trade_record.at_index()),
          make_nvp("entryTimestamp", trade_record.entry_timestamp()),
          make_nvp("exitTimestamp", trade_record.exit_timestamp()),
          make_nvp("entryPrice", trade_record.entry_price()),
          make_nvp("exitPrice", trade_record.exit_price()),
          make_nvp("stopLossPrice", trade_record.stop_loss_price()),
          make_nvp("trailingStopPrice", trade_record.trailing_stop_price()),
          make_nvp("takeProfitPrice", trade_record.take_profit_price()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::TradeRecord& trade_record)
{
  auto status = std::size_t{};
  auto position_size = double{};
  auto entry_index = std::size_t{};
  auto at_index = std::size_t{};
  auto entry_timestamp = std::time_t{};
  auto exit_timestamp = std::time_t{};
  auto entry_price = double{};
  auto exit_price = double{};
  auto stop_loss_price = double{};
  auto trailing_stop_price = double{};
  auto take_profit_price = double{};

  archive(make_nvp("status", status),
          make_nvp("positionSize", position_size),
          make_nvp("entryIndex", entry_index),
          make_nvp("atIndex", at_index),
          make_nvp("entryTimestamp", entry_timestamp),
          make_nvp("exitTimestamp", exit_timestamp),
          make_nvp("entryPrice", entry_price),
          make_nvp("exitPrice", exit_price),
          make_nvp("stopLossPrice", stop_loss_price),
          make_nvp("trailingStopPrice", trailing_stop_price),
          make_nvp("takeProfitPrice", take_profit_price));

  trade_record = pludux::backtest::TradeRecord{
   static_cast<pludux::backtest::TradeRecord::Status>(status),
   position_size,
   entry_index,
   at_index,
   entry_timestamp,
   exit_timestamp,
   entry_price,
   exit_price,
   stop_loss_price,
   trailing_stop_price,
   take_profit_price};
}
/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::BacktestingSummary& summary)
{
  archive(make_nvp("tradeRecord", summary.trade_record()),
          make_nvp("cumulativeInvestments", summary.cumulative_investments()),
          make_nvp("cumulativeTakeProfits", summary.cumulative_take_profits()),
          make_nvp("cumulativeStopLosses", summary.cumulative_stop_losses()),
          make_nvp("cumulativeExitWins", summary.cumulative_exit_wins()),
          make_nvp("cumulativeExitLosses", summary.cumulative_exit_losses()),
          make_nvp("cumulativeDurations", summary.cumulative_durations()),
          make_nvp("takeProfitCount", summary.take_profit_count()),
          make_nvp("stopLossCount", summary.stop_loss_count()),
          make_nvp("exitWinCount", summary.exit_win_count()),
          make_nvp("exitLossCount", summary.exit_loss_count()),
          make_nvp("breakEvenCount", summary.break_even_count()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::BacktestingSummary& summary)
{
  auto trade_record = pludux::backtest::TradeRecord{};
  auto cumulative_investments = double{};
  auto cumulative_take_profits = double{};
  auto cumulative_stop_losses = double{};
  auto cumulative_exit_wins = double{};
  auto cumulative_exit_losses = double{};
  auto cumulative_durations = std::time_t{};
  auto take_profit_count = std::size_t{};
  auto stop_loss_count = std::size_t{};
  auto exit_win_count = std::size_t{};
  auto exit_loss_count = std::size_t{};
  auto break_even_count = std::size_t{};

  archive(make_nvp("tradeRecord", trade_record),
          make_nvp("cumulativeInvestments", cumulative_investments),
          make_nvp("cumulativeTakeProfits", cumulative_take_profits),
          make_nvp("cumulativeStopLosses", cumulative_stop_losses),
          make_nvp("cumulativeExitWins", cumulative_exit_wins),
          make_nvp("cumulativeExitLosses", cumulative_exit_losses),
          make_nvp("cumulativeDurations", cumulative_durations),
          make_nvp("takeProfitCount", take_profit_count),
          make_nvp("stopLossCount", stop_loss_count),
          make_nvp("exitWinCount", exit_win_count),
          make_nvp("exitLossCount", exit_loss_count),
          make_nvp("breakEvenCount", break_even_count));

  summary.trade_record(trade_record);
  summary.cumulative_investments(cumulative_investments);
  summary.cumulative_take_profits(cumulative_take_profits);
  summary.cumulative_stop_losses(cumulative_stop_losses);
  summary.cumulative_exit_wins(cumulative_exit_wins);
  summary.cumulative_exit_losses(cumulative_exit_losses);
  summary.cumulative_durations(cumulative_durations);
  summary.take_profit_count(take_profit_count);
  summary.stop_loss_count(stop_loss_count);
  summary.exit_win_count(exit_win_count);
  summary.exit_loss_count(exit_loss_count);
  summary.break_even_count(break_even_count);
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void serialize(Archive& archive, pludux::backtest::TakeProfit& take_profit)
{
  auto config_parser = pludux::ConfigParser{};
  config_parser.register_default_parsers();

  const auto reward_method_json =
   config_parser.serialize_method(take_profit.reward_method());
  const auto trading_price_method_json =
   config_parser.serialize_method(take_profit.trading_price_method());

  archive(make_nvp("disabled", take_profit.is_disabled()),
          make_nvp("rewardMethod", reward_method_json),
          make_nvp("tradingPriceMethod", trading_price_method_json));
}

template<>
struct LoadAndConstruct<pludux::backtest::TakeProfit> {
  template<class Archive>
  static void
  load_and_construct(Archive& archive,
                     construct<pludux::backtest::TakeProfit>& constructor)
  {
    auto is_disabled = bool{};
    auto reward_method_json = nlohmann::json{};
    auto trading_price_method_json = nlohmann::json{};

    archive(make_nvp("disabled", is_disabled),
            make_nvp("rewardMethod", reward_method_json),
            make_nvp("tradingPriceMethod", trading_price_method_json));

    auto config_parser = pludux::ConfigParser{};
    config_parser.register_default_parsers();

    auto reward_method = config_parser.parse_method(reward_method_json);
    auto trading_price_method =
     config_parser.parse_method(trading_price_method_json);

    constructor(
     std::move(reward_method), is_disabled, std::move(trading_price_method));
  }
};

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void serialize(Archive& archive, pludux::backtest::StopLoss& stop_loss)
{
  auto config_parser = pludux::ConfigParser{};
  config_parser.register_default_parsers();

  const auto risk_method_json =
   config_parser.serialize_method(stop_loss.risk_method());

  archive(make_nvp("disabled", stop_loss.is_disabled()),
          make_nvp("trailing", stop_loss.is_trailing()),
          make_nvp("riskMethod", risk_method_json));
}

template<>
struct LoadAndConstruct<pludux::backtest::StopLoss> {
  template<class Archive>
  static void
  load_and_construct(Archive& archive,
                     construct<pludux::backtest::StopLoss>& constructor)
  {
    auto is_disabled = bool{};
    auto is_trailing = bool{};
    auto risk_method_json = nlohmann::json{};

    archive(make_nvp("disabled", is_disabled),
            make_nvp("trailing", is_trailing),
            make_nvp("riskMethod", risk_method_json));

    auto config_parser = pludux::ConfigParser{};
    config_parser.register_default_parsers();

    auto risk_method = config_parser.parse_method(risk_method_json);

    constructor(std::move(risk_method), is_disabled, is_trailing);
  }
};

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void serialize(Archive& archive, pludux::backtest::Strategy& strategy)
{
  auto config_parser = pludux::ConfigParser{};
  config_parser.register_default_parsers();

  const auto entry_filter_json =
   config_parser.serialize_filter(strategy.entry_filter());
  const auto exit_filter_json =
   config_parser.serialize_filter(strategy.exit_filter());

  auto stop_loss_ptr =
   std::make_unique<pludux::backtest::StopLoss>(strategy.stop_loss());
  auto take_profit_ptr =
   std::make_unique<pludux::backtest::TakeProfit>(strategy.take_profit());

  archive(make_nvp("name", strategy.name()),
          make_nvp("capitalRisk", strategy.capital_risk()),
          make_nvp("entryFilter", entry_filter_json),
          make_nvp("exitFilter", exit_filter_json),
          make_nvp("stopLoss", stop_loss_ptr),
          make_nvp("takeProfit", take_profit_ptr));
}

template<>
struct LoadAndConstruct<pludux::backtest::Strategy> {
  template<class Archive>
  static void
  load_and_construct(Archive& archive,
                     construct<pludux::backtest::Strategy>& constructor)
  {
    auto name = std::string{};
    auto capital_risk = double{};
    auto entry_filter_json = nlohmann::json{};
    auto exit_filter_json = nlohmann::json{};
    auto stop_loss_ptr = std::unique_ptr<pludux::backtest::StopLoss>{};
    auto take_profit_ptr = std::unique_ptr<pludux::backtest::TakeProfit>{};

    archive(make_nvp("name", name),
            make_nvp("capitalRisk", capital_risk),
            make_nvp("entryFilter", entry_filter_json),
            make_nvp("exitFilter", exit_filter_json),
            make_nvp("stopLoss", stop_loss_ptr),
            make_nvp("takeProfit", take_profit_ptr));

    auto config_parser = pludux::ConfigParser{};
    config_parser.register_default_parsers();

    auto entry_filter = config_parser.parse_filter(entry_filter_json);
    auto exit_filter = config_parser.parse_filter(exit_filter_json);

    constructor(std::move(name),
                capital_risk,
                std::move(entry_filter),
                std::move(exit_filter),
                std::move(*stop_loss_ptr),
                std::move(*take_profit_ptr));
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
    auto summaries = std::vector<pludux::backtest::BacktestingSummary>{};
    auto is_failed = bool{};

    archive(make_nvp("name", name),
            make_nvp("strategy", strategy_ptr),
            make_nvp("asset", asset_ptr),
            make_nvp("summaries", summaries),
            make_nvp("isFailed", is_failed));

    auto backtest = pludux::Backtest{
     std::move(name), strategy_ptr, asset_ptr, std::move(summaries)};

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
  const auto& backtests = app_state_data.backtests;

  archive(make_nvp("alertMessages", app_state_data.alert_messages));
  archive(
   make_nvp("selectedBacktestIndex", app_state_data.selected_backtest_index));
  archive(make_nvp("strategies", app_state_data.strategies));
  archive(make_nvp("assets", app_state_data.assets));

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
