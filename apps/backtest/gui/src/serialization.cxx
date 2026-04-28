module;

#include <algorithm>
#include <memory>
#include <ranges>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <cereal/cereal.hpp>
#include <jsoncons/json.hpp>
#include <rapidcsv.h>

#include <cereal/archives/json.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

export module pludux.apps.backtest:serialization;

import pludux.backtest;

import :state_arena;
import :state_descriptor;
import :application_state;

export namespace cereal {

template<class Archive>
void save(Archive& archive, const pludux::SeriesResultsCollector& collector)
{
  archive(make_nvp("results", collector.results()));
}

template<class Archive>
void load(Archive& archive, pludux::SeriesResultsCollector& collector)
{
  auto results = std::unordered_map<std::string, std::vector<double>>{};
  archive(make_nvp("results", results));
  collector.results(std::move(results));
}

template<class Archive>
void save(Archive& archive, const pludux::backtest::TradeRecord& trade_record)
{
  const auto status = static_cast<std::size_t>(trade_record.status());
  archive(make_nvp("status", status),
          make_nvp("positionSize", trade_record.position_size()),
          make_nvp("investment", trade_record.investment()),
          make_nvp("entryTimestamp", trade_record.entry_timestamp()),
          make_nvp("entryPrice", trade_record.entry_price()),
          make_nvp("totalEntryFees", trade_record.total_entry_fees()),
          make_nvp("exitTimestamp", trade_record.exit_timestamp()),
          make_nvp("exitPrice", trade_record.exit_price()),
          make_nvp("totalExitFees", trade_record.total_exit_fees()),
          make_nvp("stopLossPrice", trade_record.stop_loss_price()),
          make_nvp("trailingStopPrice", trade_record.trailing_stop_price()),
          make_nvp("takeProfitPrice", trade_record.take_profit_price()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::TradeRecord& trade_record)
{
  auto status = std::size_t{};
  auto position_size = double{};
  auto investment = double{};
  auto entry_timestamp = std::time_t{};
  auto entry_price = double{};
  auto total_entry_fees = double{};
  auto exit_timestamp = std::time_t{};
  auto exit_price = double{};
  auto total_exit_fees = double{};
  auto stop_loss_price = double{};
  auto trailing_stop_price = double{};
  auto take_profit_price = double{};

  archive(make_nvp("status", status),
          make_nvp("positionSize", position_size),
          make_nvp("investment", investment),
          make_nvp("entryTimestamp", entry_timestamp),
          make_nvp("entryPrice", entry_price),
          make_nvp("totalEntryFees", total_entry_fees),
          make_nvp("exitTimestamp", exit_timestamp),
          make_nvp("exitPrice", exit_price),
          make_nvp("totalExitFees", total_exit_fees),
          make_nvp("stopLossPrice", stop_loss_price),
          make_nvp("trailingStopPrice", trailing_stop_price),
          make_nvp("takeProfitPrice", take_profit_price));

  trade_record = pludux::backtest::TradeRecord{
   static_cast<pludux::backtest::TradeRecord::Status>(status),
   position_size,
   investment,
   entry_timestamp,
   entry_price,
   total_entry_fees,
   exit_timestamp,
   exit_price,
   total_exit_fees,
   stop_loss_price,
   trailing_stop_price,
   take_profit_price};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive,
          const pludux::backtest::TradePosition& trade_position)
{
  archive(
   make_nvp("positionSize", trade_position.position_size()),
   make_nvp("investment", trade_position.investment()),
   make_nvp("entryPrice", trade_position.entry_price()),
   make_nvp("totalEntryFees", trade_position.total_entry_fees()),
   make_nvp("stopLossInitialPrice", trade_position.stop_loss_initial_price()),
   make_nvp("stopLossTrailingPrice", trade_position.stop_loss_trailing_price()),
   make_nvp("takeProfitPrice", trade_position.take_profit_price()),
   make_nvp("entryTimestamp", trade_position.entry_timestamp()),
   make_nvp("realizedRecords", trade_position.realized_records()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::TradePosition& trade_position)
{
  auto position_size = double{};
  auto investment = double{};
  auto entry_price = double{};
  auto total_entry_fees = double{};
  auto stop_loss_initial_price = double{};
  auto stop_loss_trailing_price = double{};
  auto take_profit_price = double{};
  auto entry_timestamp = std::time_t{};
  auto realized_records = std::vector<pludux::backtest::TradeRecord>{};

  archive(make_nvp("positionSize", position_size),
          make_nvp("investment", investment),
          make_nvp("entryPrice", entry_price),
          make_nvp("totalEntryFees", total_entry_fees),
          make_nvp("stopLossInitialPrice", stop_loss_initial_price),
          make_nvp("stopLossTrailingPrice", stop_loss_trailing_price),
          make_nvp("takeProfitPrice", take_profit_price),
          make_nvp("entryTimestamp", entry_timestamp),
          make_nvp("realizedRecords", realized_records));

  trade_position = pludux::backtest::TradePosition{position_size,
                                                   investment,
                                                   entry_timestamp,
                                                   entry_price,
                                                   total_entry_fees,
                                                   stop_loss_initial_price,
                                                   stop_loss_trailing_price,
                                                   take_profit_price,
                                                   std::move(realized_records)};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::TradeSession& trade_session)
{
  archive(make_nvp("marketTimestamp", trade_session.market_timestamp()),
          make_nvp("marketPrice", trade_session.market_price()),
          make_nvp("marketLookback", trade_session.market_lookback()),
          make_nvp("openPosition", trade_session.open_position()),
          make_nvp("closedPosition", trade_session.closed_position()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::TradeSession& trade_session)
{
  auto market_timestamp = std::time_t{};
  auto market_price = double{};
  auto market_lookback = std::size_t{};
  auto open_position = std::optional<pludux::backtest::TradePosition>{};
  auto closed_position = std::optional<pludux::backtest::TradePosition>{};

  archive(make_nvp("marketTimestamp", market_timestamp),
          make_nvp("marketPrice", market_price),
          make_nvp("marketLookback", market_lookback),
          make_nvp("openPosition", open_position),
          make_nvp("closedPosition", closed_position));

  trade_session.market_timestamp(market_timestamp);
  trade_session.market_price(market_price);
  trade_session.market_lookback(market_lookback);
  trade_session.open_position(std::move(open_position));
  trade_session.closed_position(std::move(closed_position));
}
/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::BacktestSummary& summary)
{
  archive(make_nvp("tradeSession", summary.trade_session()),
          make_nvp("capital", summary.capital()),
          make_nvp("peakEquity", summary.peak_equity()),
          make_nvp("maxDrawdown", summary.max_drawdown()),
          make_nvp("cumulativeDurations", summary.cumulative_durations()),
          make_nvp("cumulativeInvestments", summary.cumulative_investments()),
          make_nvp("cumulativeProfits", summary.cumulative_profits()),
          make_nvp("cumulativeLosses", summary.cumulative_losses()),
          make_nvp("profitCount", summary.profit_count()),
          make_nvp("lossCount", summary.loss_count()),
          make_nvp("breakEvenCount", summary.break_even_count()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::BacktestSummary& summary)
{
  auto trade_session = pludux::backtest::TradeSession{};
  auto capital = double{};
  auto peak_equity = double{};
  auto max_drawdown = double{};
  auto cumulative_durations = std::time_t{};
  auto cumulative_investments = double{};
  auto cumulative_profits = double{};
  auto cumulative_losses = double{};
  auto profit_count = std::size_t{};
  auto loss_count = std::size_t{};
  auto break_even_count = std::size_t{};

  archive(make_nvp("tradeSession", trade_session),
          make_nvp("capital", capital),
          make_nvp("peakEquity", peak_equity),
          make_nvp("maxDrawdown", max_drawdown),
          make_nvp("cumulativeDurations", cumulative_durations),
          make_nvp("cumulativeInvestments", cumulative_investments),
          make_nvp("cumulativeProfits", cumulative_profits),
          make_nvp("cumulativeLosses", cumulative_losses),
          make_nvp("profitCount", profit_count),
          make_nvp("lossCount", loss_count),
          make_nvp("breakEvenCount", break_even_count));

  auto new_summary = pludux::backtest::BacktestSummary{};
  new_summary.trade_session(std::move(trade_session));
  new_summary.capital(capital);
  new_summary.peak_equity(peak_equity);
  new_summary.max_drawdown(max_drawdown);
  new_summary.cumulative_durations(cumulative_durations);
  new_summary.cumulative_investments(cumulative_investments);
  new_summary.cumulative_profits(cumulative_profits);
  new_summary.cumulative_losses(cumulative_losses);
  new_summary.profit_count(profit_count);
  new_summary.loss_count(loss_count);
  new_summary.break_even_count(break_even_count);

  summary = std::move(new_summary);
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::Strategy& strategy)
{
  auto strategy_json = pludux::backtest::stringify_backtest_strategy(strategy);

  archive(make_nvp("name", strategy.name()),
          make_nvp("strategyJson", strategy_json.to_string()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::Strategy& strategy)
{
  auto name = std::string{};
  auto strategy_str = std::string{};

  archive(make_nvp("name", name), make_nvp("strategyJson", strategy_str));

  strategy = pludux::backtest::parse_backtest_strategy_json(name, strategy_str);
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::Profile& profile)
{
  archive(
   make_nvp("name", profile.name()),
   make_nvp("capitalRisk", profile.capital_risk()),
   make_nvp("rDistanceMode", static_cast<int>(profile.r_distance_mode())),
   make_nvp("rModeAtrPeriod", profile.r_mode_atr().first),
   make_nvp("rModeAtrMultiplier", profile.r_mode_atr().second),
   make_nvp("rModePercentage", profile.r_mode_percentage()),
   make_nvp("rModePrice", profile.r_mode_price()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::Profile& profile)
{
  auto name = std::string{};
  auto capital_risk = double{};
  auto r_distance_mode = int{};
  auto r_mode_atr = std::pair<std::size_t, double>{};
  auto r_mode_percentage = double{};
  auto r_mode_price = double{};

  archive(make_nvp("name", name),
          make_nvp("capitalRisk", capital_risk),
          make_nvp("rDistanceMode", r_distance_mode),
          make_nvp("rModeAtrPeriod", r_mode_atr.first),
          make_nvp("rModeAtrMultiplier", r_mode_atr.second),
          make_nvp("rModePercentage", r_mode_percentage),
          make_nvp("rModePrice", r_mode_price));

  profile = pludux::backtest::Profile{
   std::move(name),
   capital_risk,
   static_cast<pludux::backtest::Profile::RDistance>(r_distance_mode),
   std::move(r_mode_atr),
   r_mode_percentage,
   r_mode_price};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::BrokerFee& broker_fee)
{
  const auto fee_type = static_cast<std::size_t>(broker_fee.fee_type());
  const auto fee_position = static_cast<std::size_t>(broker_fee.fee_position());
  const auto fee_trigger = static_cast<std::size_t>(broker_fee.fee_trigger());
  archive(make_nvp("name", broker_fee.name()),
          make_nvp("feeType", fee_type),
          make_nvp("feePosition", fee_position),
          make_nvp("feeTrigger", fee_trigger),
          make_nvp("feeValue", broker_fee.value()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::BrokerFee& broker_fee)
{
  auto name = std::string{};
  auto fee_type = std::size_t{};
  auto fee_position = std::size_t{};
  auto fee_trigger = std::size_t{};
  auto fee_value = double{};

  archive(make_nvp("name", name),
          make_nvp("feeType", fee_type),
          make_nvp("feePosition", fee_position),
          make_nvp("feeTrigger", fee_trigger),
          make_nvp("feeValue", fee_value));

  broker_fee = pludux::backtest::BrokerFee{
   std::move(name),
   static_cast<pludux::backtest::BrokerFee::FeeType>(fee_type),
   static_cast<pludux::backtest::BrokerFee::FeePosition>(fee_position),
   static_cast<pludux::backtest::BrokerFee::FeeTrigger>(fee_trigger),
   fee_value};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::Broker& broker)
{
  archive(make_nvp("name", broker.name()), make_nvp("fees", broker.fees()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::Broker& broker)
{
  auto name = std::string{};
  auto fees = std::vector<pludux::backtest::BrokerFee>{};

  archive(make_nvp("name", name), make_nvp("fees", fees));

  broker = pludux::backtest::Broker{std::move(name), std::move(fees)};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::Market& market)
{
  archive(make_nvp("name", market.name()),
          make_nvp("minOrderQuantity", market.min_order_quantity()),
          make_nvp("quantityStep", market.quantity_step()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::Market& market)
{
  auto name = std::string{};
  auto min_order_quantity = double{};
  auto quantity_step = double{};

  archive(make_nvp("name", name),
          make_nvp("minOrderQuantity", min_order_quantity),
          make_nvp("quantityStep", quantity_step));

  market =
   pludux::backtest::Market{std::move(name), min_order_quantity, quantity_step};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::AssetQuoteFieldResolver& resolver)
{
  archive(make_nvp("datetimeField", resolver.datetime_field()),
          make_nvp("openField", resolver.open_field()),
          make_nvp("highField", resolver.high_field()),
          make_nvp("lowField", resolver.low_field()),
          make_nvp("closeField", resolver.close_field()),
          make_nvp("volumeField", resolver.volume_field()));
}

template<class Archive>
void load(Archive& archive, pludux::AssetQuoteFieldResolver& resolver)
{
  auto datetime_field = std::string{};
  auto open_field = std::string{};
  auto high_field = std::string{};
  auto low_field = std::string{};
  auto close_field = std::string{};
  auto volume_field = std::string{};

  archive(make_nvp("datetimeField", datetime_field),
          make_nvp("openField", open_field),
          make_nvp("highField", high_field),
          make_nvp("lowField", low_field),
          make_nvp("closeField", close_field),
          make_nvp("volumeField", volume_field));

  resolver = pludux::AssetQuoteFieldResolver{std::move(datetime_field),
                                             std::move(open_field),
                                             std::move(high_field),
                                             std::move(low_field),
                                             std::move(close_field),
                                             std::move(volume_field)};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::AssetData& asset_data)
{
  archive(make_nvp("data", asset_data.data()));
}

template<class Archive>
void load(Archive& archive, pludux::AssetData& asset_data)
{
  auto data = std::vector<double>{};

  archive(make_nvp("data", data));

  asset_data.data(std::move(data));
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::AssetHistory& asset_history)
{
  archive(make_nvp("fieldData", asset_history.field_data()));
}

template<class Archive>
void load(Archive& archive, pludux::AssetHistory& asset_history)
{
  auto field_data = pludux::AssetHistory::FieldDataType{};

  archive(make_nvp("fieldData", field_data));

  for(auto& [key, series] : field_data) {
    asset_history.insert(key, std::move(series));
  }
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::Asset& asset)
{
  archive(make_nvp("name", asset.name()),
          make_nvp("history", asset.history()),
          make_nvp("fieldResolver", asset.field_resolver()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::Asset& asset)
{
  auto name = std::string{};
  auto history = pludux::AssetHistory{};
  auto field_resolver = pludux::AssetQuoteFieldResolver{};

  archive(make_nvp("name", name),
          make_nvp("history", history),
          make_nvp("fieldResolver", field_resolver));

  asset = pludux::backtest::Asset{
   std::move(name), std::move(history), std::move(field_resolver)};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive, typename THandle>
void save(Archive& archive,
          const pludux::backtest::StoreHandle<THandle>& handle)
{
  archive(make_nvp("slotIndex", handle.slot_index()),
          make_nvp("generation", handle.generation()));
}

template<class Archive, typename THandle>
void load(Archive& archive, pludux::backtest::StoreHandle<THandle>& handle)
{
  auto slot_index = std::size_t{};
  auto generation = std::size_t{};

  archive(make_nvp("slotIndex", slot_index),
          make_nvp("generation", generation));

  handle = pludux::backtest::StoreHandle<THandle>{slot_index, generation};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::StoreSlot& slot)
{
  archive(make_nvp("valueIndex", slot.value_index()),
          make_nvp("generation", slot.generation()),
          make_nvp("alive", slot.alive()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::StoreSlot& slot)
{
  auto value_index = std::size_t{};
  auto generation = std::size_t{};
  auto alive = bool{};

  archive(make_nvp("valueIndex", value_index),
          make_nvp("generation", generation),
          make_nvp("alive", alive));

  slot = pludux::backtest::StoreSlot{value_index, generation, alive};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive, typename TValue, typename THandleValue>
void save(Archive& archive,
          const pludux::backtest::Store<TValue, THandleValue>& store)
{
  archive(make_nvp("values", store.values()),
          make_nvp("slots", store.slots()),
          make_nvp("valueToSlotIndices", store.value_to_slot_indices()),
          make_nvp("freeSlotIndices", store.free_slot_indices()));
}

template<class Archive, typename TValue, typename THandleValue>
void load(Archive& archive,
          pludux::backtest::Store<TValue, THandleValue>& store)
{
  auto values = std::vector<TValue>{};
  auto slots = std::vector<pludux::backtest::StoreSlot>{};
  auto value_to_slot_indices = std::vector<std::size_t>{};
  auto free_slot_indices = std::vector<std::size_t>{};

  archive(make_nvp("values", values),
          make_nvp("slots", slots),
          make_nvp("valueToSlotIndices", value_to_slot_indices),
          make_nvp("freeSlotIndices", free_slot_indices));

  store = pludux::backtest::Store<TValue, THandleValue>{
   std::move(values),
   std::move(slots),
   std::move(value_to_slot_indices),
   std::move(free_slot_indices)};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::Backtest& backtest)
{
  archive(make_nvp("name", backtest.name()),
          make_nvp("initialCapital", backtest.initial_capital()),
          make_nvp("asset", backtest.asset_handle()),
          make_nvp("strategy", backtest.strategy_handle()),
          make_nvp("market", backtest.market_handle()),
          make_nvp("broker", backtest.broker_handle()),
          make_nvp("profile", backtest.profile_handle()),
          make_nvp("isFailed", backtest.is_failed()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::Backtest& backtest)
{
  auto name = std::string{};
  auto initial_capital = double{};
  auto asset_handle = pludux::backtest::StoreHandle<pludux::backtest::Asset>{};
  auto strategy_handle =
   pludux::backtest::StoreHandle<pludux::backtest::Strategy>{};
  auto market_handle =
   pludux::backtest::StoreHandle<pludux::backtest::Market>{};
  auto broker_handle =
   pludux::backtest::StoreHandle<pludux::backtest::Broker>{};
  auto profile_handle =
   pludux::backtest::StoreHandle<pludux::backtest::Profile>{};
  auto is_failed = bool{};

  archive(make_nvp("name", name),
          make_nvp("initialCapital", initial_capital),
          make_nvp("asset", asset_handle),
          make_nvp("strategy", strategy_handle),
          make_nvp("market", market_handle),
          make_nvp("broker", broker_handle),
          make_nvp("profile", profile_handle),
          make_nvp("isFailed", is_failed));

  backtest = pludux::backtest::Backtest{std::move(name),
                                        initial_capital,
                                        asset_handle,
                                        strategy_handle,
                                        market_handle,
                                        broker_handle,
                                        profile_handle,
                                        is_failed};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::apps::StateArena& arena)
{
  archive(make_nvp("imguiIniSettings", arena.imgui_ini_settings()),
          make_nvp("backtestStore", arena.backtest_store()),
          make_nvp("assetStore", arena.asset_store()),
          make_nvp("strategyStore", arena.strategy_store()),
          make_nvp("marketStore", arena.market_store()),
          make_nvp("brokerStore", arena.broker_store()),
          make_nvp("profileStore", arena.profile_store()),
          make_nvp("backtestSummariesStore", arena.backtest_summaries_store()),
          make_nvp("seriesResultsStore", arena.series_results_store()));
}

template<class Archive>
void load(Archive& archive, pludux::apps::StateArena& arena)
{
  auto imgui_ini_settings = std::string{};
  auto backtest_store = pludux::backtest::Store<pludux::backtest::Backtest>{};
  auto asset_store = pludux::backtest::Store<pludux::backtest::Asset>{};
  auto strategy_store = pludux::backtest::Store<pludux::backtest::Strategy>{};
  auto market_store = pludux::backtest::Store<pludux::backtest::Market>{};
  auto broker_store = pludux::backtest::Store<pludux::backtest::Broker>{};
  auto profile_store = pludux::backtest::Store<pludux::backtest::Profile>{};
  auto backtest_summaries_store =
   pludux::backtest::Store<std::vector<pludux::backtest::BacktestSummary>,
                           pludux::backtest::Backtest>{};
  auto series_results_store =
   pludux::backtest::Store<pludux::SeriesResultsCollector,
                           pludux::backtest::Backtest>{};

  archive(make_nvp("imguiIniSettings", imgui_ini_settings),
          make_nvp("backtestStore", backtest_store),
          make_nvp("assetStore", asset_store),
          make_nvp("strategyStore", strategy_store),
          make_nvp("marketStore", market_store),
          make_nvp("brokerStore", broker_store),
          make_nvp("profileStore", profile_store),
          make_nvp("backtestSummariesStore", backtest_summaries_store),
          make_nvp("seriesResultsStore", series_results_store));

  arena = pludux::apps::StateArena{std::move(imgui_ini_settings),
                                   std::move(backtest_store),
                                   std::move(asset_store),
                                   std::move(strategy_store),
                                   std::move(market_store),
                                   std::move(broker_store),
                                   std::move(profile_store),
                                   std::move(backtest_summaries_store),
                                   std::move(series_results_store)};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::apps::StateDescriptor& descriptor)
{
  archive(
   make_nvp("selectedBacktestHandle", descriptor.selected_backtest_handle()),
   make_nvp("backtestHandles", descriptor.backtest_handles()),
   make_nvp("assetHandles", descriptor.asset_handles()),
   make_nvp("strategyHandles", descriptor.strategy_handles()),
   make_nvp("marketHandles", descriptor.market_handles()),
   make_nvp("brokerHandles", descriptor.broker_handles()),
   make_nvp("profileHandles", descriptor.profile_handles()));
}

template<class Archive>
void load(Archive& archive, pludux::apps::StateDescriptor& descriptor)
{
  auto selected_backtest_handle =
   pludux::backtest::StoreHandle<pludux::backtest::Backtest>{};
  auto backtest_handles =
   std::vector<pludux::backtest::StoreHandle<pludux::backtest::Backtest>>{};
  auto asset_handles =
   std::vector<pludux::backtest::StoreHandle<pludux::backtest::Asset>>{};
  auto strategy_handles =
   std::vector<pludux::backtest::StoreHandle<pludux::backtest::Strategy>>{};
  auto market_handles =
   std::vector<pludux::backtest::StoreHandle<pludux::backtest::Market>>{};
  auto broker_handles =
   std::vector<pludux::backtest::StoreHandle<pludux::backtest::Broker>>{};
  auto profile_handles =
   std::vector<pludux::backtest::StoreHandle<pludux::backtest::Profile>>{};

  archive(make_nvp("selectedBacktestHandle", selected_backtest_handle),
          make_nvp("backtestHandles", backtest_handles),
          make_nvp("assetHandles", asset_handles),
          make_nvp("strategyHandles", strategy_handles),
          make_nvp("marketHandles", market_handles),
          make_nvp("brokerHandles", broker_handles),
          make_nvp("profileHandles", profile_handles));

  descriptor = pludux::apps::StateDescriptor{selected_backtest_handle,
                                             std::move(backtest_handles),
                                             std::move(asset_handles),
                                             std::move(strategy_handles),
                                             std::move(market_handles),
                                             std::move(broker_handles),
                                             std::move(profile_handles)};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::apps::ApplicationState& app_state)
{
  archive(make_nvp("$version", std::string{PLUDUX_VERSION}),
          make_nvp("stateArena", app_state.state_arena()),
          make_nvp("stateDescriptor", app_state.state_descriptor()));
}

template<class Archive>
void load(Archive& archive, pludux::apps::ApplicationState& app_state)
{
  auto version = std::string{};
  auto state_arena = pludux::apps::StateArena{};
  auto state_descriptor = pludux::apps::StateDescriptor{};

  archive(make_nvp("$version", version),
          make_nvp("stateArena", state_arena),
          make_nvp("stateDescriptor", state_descriptor));

  app_state = pludux::apps::ApplicationState{std::move(state_descriptor),
                                             std::move(state_arena)};

  if(version != PLUDUX_VERSION) {
    app_state.reset_all_backtests();
  }
}

} // namespace cereal
