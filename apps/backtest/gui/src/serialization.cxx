module;

#include <algorithm>
#include <ranges>
#include <tuple>
#include <utility>

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
#include <cereal/types/vector.hpp>

export module pludux.apps.backtest:serialization;

import pludux.backtest;
import :application_state;

export namespace cereal {

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
  auto risk_parser = pludux::backtest::risk_reward_config_parser();
  const auto risk_method_json =
   risk_parser.serialize_method(strategy.risk_method());

  auto config_parser = pludux::make_default_registered_config_parser();

  const auto long_entry_filter_json =
   config_parser.serialize_filter(strategy.long_entry_filter());
  const auto long_exit_filter_json =
   config_parser.serialize_filter(strategy.long_exit_filter());

  const auto short_entry_filter_json =
   config_parser.serialize_filter(strategy.short_entry_filter());
  const auto short_exit_filter_json =
   config_parser.serialize_filter(strategy.short_exit_filter());

  const auto stop_loss_enabled = strategy.stop_loss_enabled();
  const auto stop_loss_trailing_enabled = strategy.stop_loss_trailing_enabled();

  const auto take_profit_enabled = strategy.take_profit_enabled();
  const auto take_profit_risk_multiplier =
   strategy.take_profit_risk_multiplier();

  const auto& series_registry = strategy.series_registry();
  auto registered_methods = std::unordered_map<std::string, std::string>{};
  for(const auto& [method_name, method] : series_registry) {
    registered_methods[method_name] =
     config_parser.serialize_method(method).to_string();
  }

  const auto risk_method_str = risk_method_json.to_string();
  const auto long_entry_filter_str = long_entry_filter_json.to_string();
  const auto long_exit_filter_str = long_exit_filter_json.to_string();
  const auto short_entry_filter_str = short_entry_filter_json.to_string();
  const auto short_exit_filter_str = short_exit_filter_json.to_string();

  archive(make_nvp("name", strategy.name()),
          make_nvp("registeredMethods", registered_methods),
          make_nvp("riskMethod", risk_method_str),
          make_nvp("longEntryMethod", long_entry_filter_str),
          make_nvp("longExitMethod", long_exit_filter_str),
          make_nvp("shortEntryMethod", short_entry_filter_str),
          make_nvp("shortExitMethod", short_exit_filter_str),
          make_nvp("stopLossEnabled", stop_loss_enabled),
          make_nvp("stopLossTrailingEnabled", stop_loss_trailing_enabled),
          make_nvp("takeProfitEnabled", take_profit_enabled),
          make_nvp("takeProfitRiskMultiplier", take_profit_risk_multiplier));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::Strategy& strategy)
{
  auto name = std::string{};
  auto registered_methods = std::unordered_map<std::string, std::string>{};
  auto risk_method_str = std::string{};
  auto long_entry_filter_str = std::string{};
  auto long_exit_filter_str = std::string{};
  auto short_entry_filter_str = std::string{};
  auto short_exit_filter_str = std::string{};
  auto stop_loss_enabled = bool{};
  auto stop_loss_trailing_enabled = bool{};
  auto take_profit_enabled = bool{};
  auto take_profit_risk_multiplier = double{};

  archive(make_nvp("name", name),
          make_nvp("registeredMethods", registered_methods),
          make_nvp("riskMethod", risk_method_str),
          make_nvp("longEntryMethod", long_entry_filter_str),
          make_nvp("longExitMethod", long_exit_filter_str),
          make_nvp("shortEntryMethod", short_entry_filter_str),
          make_nvp("shortExitMethod", short_exit_filter_str),
          make_nvp("stopLossEnabled", stop_loss_enabled),
          make_nvp("stopLossTrailingEnabled", stop_loss_trailing_enabled),
          make_nvp("takeProfitEnabled", take_profit_enabled),
          make_nvp("takeProfitRiskMultiplier", take_profit_risk_multiplier));

  const auto risk_method_json = jsoncons::ojson::parse(risk_method_str);
  const auto long_entry_filter_json =
   jsoncons::ojson::parse(long_entry_filter_str);
  const auto long_exit_filter_json =
   jsoncons::ojson::parse(long_exit_filter_str);
  const auto short_entry_filter_json =
   jsoncons::ojson::parse(short_entry_filter_str);
  const auto short_exit_filter_json =
   jsoncons::ojson::parse(short_exit_filter_str);

  auto risk_parser = pludux::backtest::risk_reward_config_parser();
  auto risk_method = risk_parser.parse_method(risk_method_json);

  auto config_parser = pludux::make_default_registered_config_parser();

  auto long_entry_filter = config_parser.parse_filter(long_entry_filter_json);
  auto long_exit_filter = config_parser.parse_filter(long_exit_filter_json);
  auto short_entry_filter = config_parser.parse_filter(short_entry_filter_json);
  auto short_exit_filter = config_parser.parse_filter(short_exit_filter_json);

  auto series_registry = pludux::SeriesMethodRegistry{};
  for(const auto& [method_name, method_str] : registered_methods) {
    const auto method_json = jsoncons::ojson::parse(method_str);
    const auto method = config_parser.parse_method(method_json);
    series_registry.set(method_name, method);
  }

  strategy = pludux::backtest::Strategy{std::move(name),
                                        std::move(series_registry),
                                        std::move(risk_method),
                                        std::move(long_entry_filter),
                                        std::move(long_exit_filter),
                                        std::move(short_entry_filter),
                                        std::move(short_exit_filter),
                                        stop_loss_enabled,
                                        stop_loss_trailing_enabled,
                                        take_profit_enabled,
                                        take_profit_risk_multiplier};
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::backtest::Profile& profile)
{
  archive(make_nvp("name", profile.name()),
          make_nvp("initialCapital", profile.initial_capital()),
          make_nvp("capitalRisk", profile.capital_risk()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::Profile& profile)
{
  auto name = std::string{};
  auto initial_capital = double{};
  auto capital_risk = double{};

  archive(make_nvp("name", name),
          make_nvp("initialCapital", initial_capital),
          make_nvp("capitalRisk", capital_risk));

  profile =
   pludux::backtest::Profile{std::move(name), initial_capital, capital_risk};
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

template<class Archive>
void save(Archive& archive, const pludux::backtest::Backtest& backtest)
{
  archive(make_nvp("name", backtest.name()),
          make_nvp("asset", backtest.asset_ptr()),
          make_nvp("strategy", backtest.strategy_ptr()),
          make_nvp("market", backtest.market_ptr()),
          make_nvp("broker", backtest.broker_ptr()),
          make_nvp("profile", backtest.profile_ptr()),
          make_nvp("summaries", backtest.summaries()),
          make_nvp("isFailed", backtest.is_failed()));
}

template<class Archive>
void load(Archive& archive, pludux::backtest::Backtest& backtest)
{
  auto name = std::string{};
  auto asset_ptr = std::shared_ptr<pludux::backtest::Asset>{};
  auto strategy_ptr = std::shared_ptr<pludux::backtest::Strategy>{};
  auto market_ptr = std::shared_ptr<pludux::backtest::Market>{};
  auto broker_ptr = std::shared_ptr<pludux::backtest::Broker>{};
  auto profile_ptr = std::shared_ptr<pludux::backtest::Profile>{};
  auto summaries = std::vector<pludux::backtest::BacktestSummary>{};
  auto is_failed = bool{};

  archive(make_nvp("name", name),
          make_nvp("asset", asset_ptr),
          make_nvp("strategy", strategy_ptr),
          make_nvp("market", market_ptr),
          make_nvp("broker", broker_ptr),
          make_nvp("profile", profile_ptr),
          make_nvp("summaries", summaries),
          make_nvp("isFailed", is_failed));

  backtest = pludux::backtest::Backtest{std::move(name),
                                        asset_ptr,
                                        strategy_ptr,
                                        market_ptr,
                                        broker_ptr,
                                        profile_ptr,
                                        std::move(summaries)};

  if(is_failed) {
    backtest.mark_as_failed();
  }
}

/*--------------------------------------------------------------------------------------*/

template<class Archive>
void save(Archive& archive, const pludux::apps::ApplicationState& app_state)
{
  auto assets = app_state.assets();
  auto strategies = app_state.strategies();
  auto markets = app_state.markets();
  auto brokers = app_state.brokers();
  auto profiles = app_state.profiles();
  auto backtests = app_state.backtests();

  archive(make_nvp("alertMessages", app_state.alert_messages()));
  archive(make_nvp("backtests", backtests));
  archive(make_nvp("assets", assets));
  archive(make_nvp("strategies", strategies));
  archive(make_nvp("markets", markets));
  archive(make_nvp("brokers", brokers));
  archive(make_nvp("profiles", profiles));
}

template<class Archive>
void load(Archive& archive, pludux::apps::ApplicationState& app_state)
{
  auto alert_messages = std::queue<std::string>{};
  auto assets = std::vector<std::shared_ptr<pludux::backtest::Asset>>{};
  auto strategies = std::vector<std::shared_ptr<pludux::backtest::Strategy>>{};
  auto markets = std::vector<std::shared_ptr<pludux::backtest::Market>>{};
  auto brokers = std::vector<std::shared_ptr<pludux::backtest::Broker>>{};
  auto profiles = std::vector<std::shared_ptr<pludux::backtest::Profile>>{};
  auto backtests = std::vector<std::shared_ptr<pludux::backtest::Backtest>>{};

  archive(make_nvp("alertMessages", alert_messages));
  archive(make_nvp("backtests", backtests));
  archive(make_nvp("assets", assets));
  archive(make_nvp("strategies", strategies));
  archive(make_nvp("markets", markets));
  archive(make_nvp("brokers", brokers));
  archive(make_nvp("profiles", profiles));

  const auto selected_backtest_index = !backtests.empty() ? 0 : -1;

  app_state = pludux::apps::ApplicationState{selected_backtest_index,
                                             std::move(alert_messages),
                                             std::move(backtests),
                                             std::move(assets),
                                             std::move(strategies),
                                             std::move(markets),
                                             std::move(brokers),
                                             std::move(profiles)};
}

} // namespace cereal
