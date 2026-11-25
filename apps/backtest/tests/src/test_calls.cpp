#include <gtest/gtest.h>

#include <cmath>
#include <concepts>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

namespace {

const auto columns = CallColumns{"When", "Ticker", "Bullish", "Low", "High"};

auto read_calls(const std::string& csv) -> CallTable
{
  auto stream = std::istringstream{csv};
  return read_call_csv(stream, "Desk calls");
}

auto make_asset(std::string name, const std::string& candles) -> Asset
{
  auto asset = Asset{std::move(name)};
  auto stream =
   std::istringstream{"Datetime,Open,High,Low,Close,Volume\n" + candles};
  update_asset_from_csv(asset, stream);
  return asset;
}

auto as_method(StrategyNode node) -> StrategyMethod
{
  auto context = NodeToErasedMethodContext{};
  return node_to_erased_method<BacktestMethodContext>(node, context);
}

auto call_rule(
 StrategyMethod risk = ValueMethod{10.0},
 std::vector<BacktestRunner::PositionRule::StopLossRule> stops = {},
 std::vector<BacktestRunner::PositionRule::TakeProfitRule> targets = {})
 -> BacktestRunner::PositionRule
{
  auto rule =
   BacktestRunner::PositionRule{BooleanMethod<false>{},
                                {},
                                BooleanMethod<false>{},
                                0,
                                0,
                                std::move(risk),
                                std::move(stops),
                                SignalTiming::NextOpen,
                                SignalTiming::NextOpen,
                                StopTargetReferencePrice::AveragePrice,
                                StopTargetReferencePrice::AveragePrice,
                                std::move(targets)};
  rule.entry_source(EntrySource::Call);
  return rule;
}

auto call_setup(CallValues values,
                BacktestRunner::PositionRule long_rule,
                BacktestRunner::PositionRule short_rule = {})
 -> BacktestRunner::CompiledStrategy
{
  auto setup = BacktestRunner::CompiledStrategy{
   {}, std::move(long_rule), std::move(short_rule)};
  setup.calls = std::move(values);
  setup.call_table_name = "Desk calls";
  return setup;
}

auto fixed_quantity_profile() -> Profile
{
  auto profile =
   Profile{"Profile", PositionSizingNode{FixedQuantityPositionSizing{1.0}}};
  profile.insufficient_cash_policy(InsufficientCashPolicy::Reject);
  return profile;
}

} // namespace

static_assert(std::constructible_from<StrategyNode, CallNode>);
static_assert(!std::constructible_from<ComparatorNode, CallNode>);
static_assert(!std::constructible_from<EntryFilterNode, CallNode>);

TEST(CallCompilerTest, MapsSetupFieldsAndOptionalBoundsAtExactTimestamp)
{
  const auto table =
   read_calls("When,Ticker,Bullish,Low,High,SL1,SL2,\"Desk, note\"\r\n"
              "102.25,ABC,TRUE,,105,0,91,\"notes, preserved\"\r\n"
              "102.50,XYZ,false,150,,160,,\r\n"
              "102.25,ABC.csv,true,100,100,93,92,\r\n");
  ASSERT_EQ(table.name, "Desk calls");
  ASSERT_EQ(table.headers.back(), "Desk, note");
  ASSERT_EQ(table.rows.size(), 3U);
  EXPECT_EQ(table.rows.front().back(), "notes, preserved");
  const auto compiled = compile_calls(
   table,
   columns,
   {"ABC.csv", "ABC", "XYZ.csv"},
   {CallMappings{{"stop", "SL1"}}, CallMappings{{"stop", "SL2"}}});

  ASSERT_EQ(compiled.values.size(), 2U);
  ASSERT_EQ(compiled.values[0].size(), 3U);
  ASSERT_EQ(compiled.values[1].size(), 3U);
  const auto& exact_asset = compiled.values[0][1].at(102.25);
  EXPECT_TRUE(exact_asset.is_long);
  EXPECT_FALSE(exact_asset.entry_low);
  EXPECT_EQ(exact_asset.entry_high, 105.0);
  EXPECT_DOUBLE_EQ(exact_asset.fields.at("stop"), 0);
  EXPECT_DOUBLE_EQ(compiled.values[1][1].at(102.25).fields.at("stop"), 91);
  EXPECT_DOUBLE_EQ(compiled.values[0][0].at(102.25).fields.at("stop"), 93);
  EXPECT_FALSE(compiled.values[0][2].contains(102.25));
  const auto& inferred_asset = compiled.values[0][2].at(102.5);
  EXPECT_FALSE(inferred_asset.is_long);
  EXPECT_EQ(inferred_asset.entry_low, 150.0);
  EXPECT_FALSE(inferred_asset.entry_high);
  EXPECT_FALSE(compiled.values[1][2].at(102.5).fields.contains("stop"));
}

TEST(CallCompilerTest, DefaultsUnmappedDirectionToLong)
{
  const auto table =
   CallTable{"Desk calls", {"When", "Ticker"}, {{"1", "ABC"}}};
  const auto compiled = compile_calls(
   table, CallColumns{.asset = "Ticker"}, {"ABC"}, {CallMappings{}});

  EXPECT_TRUE(compiled.values.at(0).at(0).at(1).is_long);
  EXPECT_EQ(compiled.values.at(0).at(0).at(1).row, 2U);
  EXPECT_THROW(
   compile_calls(table,
                 CallColumns{.asset = "Ticker", .direction = "Missing"},
                 {"ABC"},
                 {CallMappings{}}),
   std::invalid_argument);
}

TEST(CallCompilerTest, ParsesDirectionAliasesAndDefaultsBlankCellsToLong)
{
  const auto table = read_calls("When,Ticker,Bullish\n"
                                "1,ABC,\n"
                                "2,ABC, \t \n"
                                "3,ABC, LoNG \n"
                                "4,ABC,tRuE\n"
                                "5,ABC,YES\n"
                                "6,ABC,1\n"
                                "7,ABC, sHoRT \n"
                                "8,ABC,False\n"
                                "9,ABC,nO\n"
                                "10,ABC,0\n");
  const auto compiled =
   compile_calls(table,
                 CallColumns{.asset = "Ticker", .direction = "Bullish"},
                 {"ABC"},
                 {CallMappings{}});
  const auto& calls = compiled.values.at(0).at(0);

  ASSERT_EQ(calls.size(), 10U);
  for(const auto& [time, call] : calls)
    EXPECT_EQ(call.is_long, time <= 6) << "Call table row " << call.row;
}

TEST(CallCompilerTest,
     RejectsAmbiguousAssetsDuplicateTimesAndInvalidMappedFields)
{
  const auto good =
   read_calls("When,Ticker,Bullish,Low,High,Stop\n108.15,ABC,true,,105,90\n");
  const auto duplicate =
   read_calls("When,Ticker,Bullish,Low,High,Stop\n"
              "108.15,ABC,true,,105,90\n108.150,ABC.csv,true,,105,91\n");
  const auto inverted_bounds = read_calls(
   "When,Ticker,Bullish,Low,High,Stop\n108.15,ABC,true,106,105,90\n");
  const auto bad_direction =
   read_calls("When,Ticker,Bullish,Low,High,Stop\n108.15,ABC,buy,,105,90\n");
  const auto bad_value = read_calls(
   "When,Ticker,Bullish,Low,High,Stop\n108.15,ABC,true,,105,invalid\n");
  const auto mappings = std::vector{CallMappings{{"stop", "Stop"}}};

  EXPECT_THROW(compile_calls(good, columns, {"ABC", "ABC"}, mappings),
               std::invalid_argument);
  EXPECT_THROW(compile_calls(duplicate, columns, {"ABC.csv"}, mappings),
               std::invalid_argument);
  EXPECT_THROW(compile_calls(inverted_bounds, columns, {"ABC"}, mappings),
               std::invalid_argument);
  EXPECT_THROW(compile_calls(bad_direction, columns, {"ABC"}, mappings),
               std::invalid_argument);
  EXPECT_THROW(compile_calls(bad_value, columns, {"ABC"}, mappings),
               std::invalid_argument);
}

TEST(CallCompilerTest, UsesIdenticalDatetimeKeysToAssetCsv)
{
  const auto asset = make_asset("ABC.csv",
                                "2026-01-06,100,100,100,100,1\n"
                                "2026-01-06T11:30:01.25,100,100,100,100,1\n"
                                "2026-01-06T11:30:01.75,100,100,100,100,1\n");
  const auto table = read_calls("When,Ticker,Bullish,Low,High,Field\n"
                                "2026-01-06,ABC,true,,,11\n"
                                "2026-01-06T11:30:01.25,ABC,true,,,13\n"
                                "2026-01-06T11:30:01.75,ABC,true,,,15\n");
  const auto compiled = compile_calls(
   table, columns, {asset.name()}, {CallMappings{{"field", "Field"}}});

  ASSERT_EQ(asset.size(), 3U);
  const auto& values = compiled.values.at(0).at(0);
  EXPECT_DOUBLE_EQ(
   values.at(asset.get_snapshot(0).datetime()).fields.at("field"), 11);
  EXPECT_DOUBLE_EQ(
   values.at(asset.get_snapshot(1).datetime()).fields.at("field"), 13);
  EXPECT_DOUBLE_EQ(
   values.at(asset.get_snapshot(2).datetime()).fields.at("field"), 15);
  EXPECT_DOUBLE_EQ(
   asset.get_snapshot(2).datetime() - asset.get_snapshot(1).datetime(), 0.5);
}

TEST(CallStrategyTest, RoundTripsEntrySourceAndCallNodeThroughStrategySchema)
{
  auto entry = Strategy::Entry{};
  entry.entry_source(EntrySource::Call);
  auto position = Strategy::Position{};
  position.entry(std::move(entry));
  position.take_profits({Strategy::TakeProfit{true, CallNode{"target"}}});
  const auto strategy =
   Strategy{"Strategy with Calls", {}, std::move(position), {}, {}};
  const auto serialized = stringify_strategy(strategy);
  const auto loaded = parse_strategy("Strategy with Calls", serialized);

  EXPECT_EQ(loaded, strategy);
  const auto document = jsoncons::ojson::parse(serialized);
  EXPECT_EQ(document.at("positions")
             .at("long")
             .at("entry")
             .at("source")
             .as<std::string>(),
            "CALL");
  EXPECT_EQ(document.at("positions")
             .at("long")
             .at("takeProfits")
             .at("rules")
             .at(0)
             .at("targetPrice")
             .at("method")
             .as<std::string>(),
            "OTHER.CALL");
  auto schema_stream = std::ifstream{
   std::filesystem::path{PLUDUX_BACKTEST_SAMPLE_DIR}.parent_path() / "schemas" /
   "pludux.backtest.strategy.v1-draft.json"};
  const auto schema = jsoncons::jsonschema::make_json_schema(
   jsoncons::ojson::parse(schema_stream));
  EXPECT_NO_THROW(schema.validate(document));
}

TEST(CallRunnerTest, MissedOpenRangeIsNeverRetried)
{
  const auto asset = make_asset("ABC",
                                "1,100,100,100,100,1\n"
                                "2,110,115,95,100,1\n"
                                "3,100,100,100,100,1\n");
  const auto market = Market{"Market"};
  const auto broker = Broker{"Broker"};
  const auto profile = fixed_quantity_profile();
  auto runner = BacktestRunner{
   asset,
   market,
   broker,
   profile,
   {call_setup(
    {{2,
      CallRow{.row = 2, .is_long = true, .entry_low = 95, .entry_high = 105}}},
    call_rule())},
   1'000};
  auto series_results = std::vector<SeriesEvaluationResults>(1);
  auto timeline = BacktestTimeline{};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_EQ(timeline.size(), 3U);
  EXPECT_FALSE(timeline.open_position(0));
  EXPECT_FALSE(timeline.open_position(1));
  EXPECT_FALSE(timeline.open_position(2));
  ASSERT_EQ(timeline.position_sizing_decisions(1).size(), 1U);
  const auto& skipped = timeline.position_sizing_decisions(1).front();
  EXPECT_EQ(skipped.call_table, "Desk calls");
  EXPECT_EQ(skipped.call_row, 2U);
  EXPECT_FALSE(skipped.call_skip.empty());
  EXPECT_TRUE(timeline.position_sizing_decisions(2).empty());
}

TEST(CallRunnerTest, BindsOpeningRowToRiskAndExitsForEntirePosition)
{
  const auto asset = make_asset("ABC",
                                "1,90,92,90,92,1\n"
                                "2,100,105,99,103,1\n"
                                "3,104,108,96,105,1\n"
                                "4,121,121,121,121,1\n"
                                "5,120,120,120,120,1\n");
  const auto table =
   read_calls("When,Ticker,Bullish,Low,High,Risk,Stop,Target\n"
              "2,ABC,true,100,100,5.5,95,120\n"
              "3,ABC,true,100,105,2,103,106\n");
  const auto compiled = compile_calls(
   table,
   columns,
   {asset.name()},
   {CallMappings{{"risk", "Risk"}, {"stop", "Stop"}, {"target", "Target"}}});
  const auto market = Market{"Market"};
  const auto broker = Broker{"Broker"};
  const auto profile = fixed_quantity_profile();
  auto setup = call_setup(
   compiled.values.at(0).at(0),
   call_rule(as_method(CallNode{
              "risk", NumericInputNode::ValueRepresentation::SignedInteger}),
             {BacktestRunner::PositionRule::StopLossRule{
              as_method(CallNode{"stop"}), true}},
             {BacktestRunner::PositionRule::TakeProfitRule{
              as_method(CallNode{"target"}), true}}),
   BacktestRunner::PositionRule{BooleanMethod<true>{},
                                {},
                                BooleanMethod<false>{},
                                0,
                                0,
                                ValueMethod{9.0},
                                {},
                                SignalTiming::NextOpen});
  setup.series_methods.set("opening target", as_method(CallNode{"target"}));
  auto runner = BacktestRunner{asset, market, broker, profile, {setup}, 1'000};
  auto series_results = std::vector<SeriesEvaluationResults>(1);
  auto timeline = BacktestTimeline{};

  for(auto bar = std::size_t{}; bar < asset.size(); ++bar) {
    runner.run(series_results, timeline);
  }

  ASSERT_EQ(timeline.size(), 5U);
  EXPECT_FALSE(timeline.open_position(0));
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->entry_price(), 100);
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->risk_distance(), 5);
  ASSERT_TRUE(timeline.open_position(2));
  ASSERT_EQ(timeline.open_position(2)->take_profit_levels().size(), 1U);
  EXPECT_DOUBLE_EQ(
   timeline.open_position(2)->take_profit_levels().front().price(), 120);
  EXPECT_TRUE(timeline.closed_trades(2).empty());
  ASSERT_EQ(timeline.closed_trades(3).size(), 1U);
  EXPECT_DOUBLE_EQ(timeline.closed_trades(3).front().exit_price(), 121);
  ASSERT_TRUE(timeline.open_position(4));
  EXPECT_DOUBLE_EQ(timeline.open_position(4)->position_size(), -1);
  const auto targets =
   series_results.front().results(std::string{"opening target"});
  ASSERT_TRUE(targets);
  EXPECT_DOUBLE_EQ(targets->get().at(2), 120);
  EXPECT_TRUE(std::isnan(targets->get().at(3)));
  EXPECT_TRUE(std::isnan(targets->get().at(4)));
}

TEST(CallRunnerTest, MarketDataInOpenCallSizingUsesPreviousCompletedCandle)
{
  const auto asset =
   make_asset("ABC", "1,70,75,70,75,1\n2,100,125,100,120,1\n");
  const auto market = Market{"Market"};
  const auto broker = Broker{"Broker"};
  const auto profile = fixed_quantity_profile();
  auto runner =
   BacktestRunner{asset,
                  market,
                  broker,
                  profile,
                  {call_setup({{2, CallRow{.row = 2, .is_long = true}}},
                              call_rule(CloseMethod{}))},
                  1'000};
  auto series_results = std::vector<SeriesEvaluationResults>(1);
  auto timeline = BacktestTimeline{};

  runner.run(series_results, timeline);
  runner.run(series_results, timeline);

  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->entry_price(), 100);
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->risk_distance(), 75);
}

TEST(CallRunnerTest, PrioritySelectsBetweenCallAndPendingNextOpenSignal)
{
  const auto asset =
   make_asset("ABC", "1,100,100,100,100,1\n2,110,110,110,110,1\n");
  const auto market = Market{"Market"};
  const auto broker = Broker{"Broker"};
  const auto profile = fixed_quantity_profile();
  const auto signal_long = BacktestRunner::PositionRule{BooleanMethod<true>{},
                                                        {},
                                                        BooleanMethod<false>{},
                                                        0,
                                                        0,
                                                        ValueMethod{10.0},
                                                        {},
                                                        SignalTiming::NextOpen};
  for(const auto priority :
      {EntryPriority::CallFirst, EntryPriority::SignalFirst}) {
    auto setup = call_setup(
     {{2, CallRow{.row = 2, .is_long = false, .fields = {{"risk", 7}}}}},
     signal_long,
     call_rule());
    setup.entry_priority = priority;
    setup.series_methods.set("call risk", as_method(CallNode{"risk"}));
    auto runner =
     BacktestRunner{asset, market, broker, profile, {std::move(setup)}, 1'000};
    auto series_results = std::vector<SeriesEvaluationResults>(1);
    auto timeline = BacktestTimeline{};

    runner.run(series_results, timeline);
    runner.run(series_results, timeline);

    EXPECT_FALSE(timeline.open_position(0));
    ASSERT_TRUE(timeline.open_position(1));
    EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(),
                     priority == EntryPriority::CallFirst ? -1.0 : 1.0);
    const auto observed = series_results[0].results(std::string{"call risk"});
    ASSERT_TRUE(observed);
    const auto& values = observed->get();
    ASSERT_EQ(values.size(), 2U);
    EXPECT_TRUE(std::isnan(values[0]));
    if(priority == EntryPriority::CallFirst) {
      EXPECT_DOUBLE_EQ(values[1], 7);
    } else {
      EXPECT_TRUE(std::isnan(values[1]));
    }
  }
}

TEST(CallPortfolioTest, OverlappingCallsCompeteForPortfolioCash)
{
  const auto first =
   make_asset("ABC", "1,100,100,100,100,1\n2,100,100,100,100,1\n");
  const auto second = make_asset("XYZ",
                                 "1,100,100,100,100,1\n"
                                 "2,100,100,100,100,1\n"
                                 "3,120,120,120,120,1\n");
  const auto market = Market{"Market"};
  const auto broker = Broker{"Broker"};
  const auto profile = fixed_quantity_profile();
  const auto run = [&](const Asset& asset, std::size_t slot) {
    return PortfolioRunner::BacktestRun{
     SystemStoreHandle{1, 1},
     AssetStoreHandle{slot, 1},
     BacktestRunner{
      asset,
      market,
      broker,
      profile,
      {call_setup(
       {{2,
         CallRow{
          .row = slot + 1, .is_long = true, .fields = {{"target", 115}}}}},
       call_rule(ValueMethod{10.0},
                 {},
                 {BacktestRunner::PositionRule::TakeProfitRule{
                  as_method(CallNode{"target"}), true}}))},
      100}};
  };
  auto runner = PortfolioRunner{100, 2, 2, {run(first, 1), run(second, 2)}};
  auto results = PortfolioResults{};

  ASSERT_EQ(runner.run_batch(results, 10), 3U);

  ASSERT_EQ(results.backtests().size(), 2U);
  EXPECT_TRUE(results.backtests()[0].timeline().open_position(1));
  const auto& rejected = results.backtests()[1].timeline();
  EXPECT_FALSE(rejected.open_position(1));
  ASSERT_EQ(rejected.trade_events(1).size(), 1U);
  EXPECT_TRUE(rejected.trade_events(1).front().is_rejected());
  EXPECT_TRUE(rejected.strategy_state(1, 0).strategy_open_position);
  EXPECT_FALSE(rejected.strategy_state(2, 0).strategy_open_position);
  EXPECT_EQ(rejected.strategy_state(2, 0).strategy_closed_positions.size(), 1U);
  EXPECT_TRUE(rejected.closed_trades(2).empty());
  EXPECT_DOUBLE_EQ(results.timeline().row(1).available_capital, 0);
}

TEST(CallRunnerTest, UnavailableFirstCandleRiskSkipsOnlyThatCall)
{
  const auto asset = make_asset("ABC", "1,90,92,89,91,1\n2,100,102,99,101,1\n");
  const auto market = Market{"Market"};
  const auto broker = Broker{"Broker"};
  const auto profile = fixed_quantity_profile();
  auto setup = call_setup({{1, CallRow{.row = 2, .is_long = true}},
                           {2, CallRow{.row = 3, .is_long = true}}},
                          call_rule(CloseMethod{}));
  auto runner = BacktestRunner{asset, market, broker, profile, {setup}, 1000};
  auto results = std::vector<SeriesEvaluationResults>(1);
  auto timeline = BacktestTimeline{};
  runner.run(results, timeline);
  runner.run(results, timeline);
  EXPECT_FALSE(timeline.open_position(0));
  const auto& skipped = timeline.position_sizing_decisions(0);
  ASSERT_EQ(skipped.size(), 1U);
  EXPECT_EQ(skipped.front().call_row, 2U);
  EXPECT_EQ(skipped.front().call_table, "Desk calls");
  EXPECT_FALSE(skipped.front().call_skip.empty());
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->risk_distance(), 91);
}

TEST(CallRunnerTest, KeepsOriginalRowAcrossPyramidAndPartialTarget)
{
  const auto asset = make_asset(
   "ABC", "1,100,100,100,100,1\n2,100,111,100,105,1\n3,102,105,101,104,1\n");
  const auto market = Market{"Market"};
  const auto broker = Broker{"Broker"};
  const auto profile = fixed_quantity_profile();
  auto rule =
   BacktestRunner::PositionRule{BooleanMethod<false>{},
                                {},
                                BooleanMethod<true>{},
                                2,
                                0,
                                SeriesMethod{"bound risk"},
                                {},
                                SignalTiming::NextOpen,
                                SignalTiming::NextOpen,
                                StopTargetReferencePrice::AveragePrice,
                                StopTargetReferencePrice::AveragePrice,
                                {{as_method(CallNode{"target"}), true, 0.5}}};
  rule.entry_source(EntrySource::Call);
  auto setup = call_setup(
   {{1,
     CallRow{
      .row = 2, .is_long = true, .fields = {{"risk", 5}, {"target", 110}}}},
    {2,
     CallRow{
      .row = 3, .is_long = true, .fields = {{"risk", 20}, {"target", 150}}}}},
   rule);
  setup.series_methods.set("bound risk", as_method(CallNode{"risk"}));
  auto runner = BacktestRunner{asset, market, broker, profile, {setup}, 1000};
  auto results = std::vector<SeriesEvaluationResults>(1);
  auto timeline = BacktestTimeline{};
  for(std::size_t i = 0; i < asset.size(); ++i)
    runner.run(results, timeline);
  ASSERT_TRUE(timeline.open_position(0));
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->risk_distance(), 5);
  ASSERT_EQ(timeline.trade_events(1).size(), 2U);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1)[0].price(), 100);
  EXPECT_DOUBLE_EQ(timeline.trade_events(1)[1].price(), 110);
  const auto values = results[0].results(std::string{"bound risk"});
  ASSERT_TRUE(values);
  EXPECT_DOUBLE_EQ(values->get().at(2), 5);
}

TEST(CallPortfolioTest, SizingSeesFreshOpeningMarksAndStaleLastClose)
{
  const auto held =
   make_asset("ABC", "1,100,100,100,100,1\n2,110,150,110,150,1\n");
  const auto stale = make_asset("DEF", "1,100,105,100,105,1\n");
  const auto fresh = make_asset("XYZ", "2,10,10,10,10,1\n");
  const auto market = Market{"Market"};
  const auto broker = Broker{"Broker"};
  const auto fixed = fixed_quantity_profile();
  const auto fraction =
   Profile{"Percent", PositionSizingNode{EquityFractionPositionSizing{0.2}}};
  const auto run = [&](const Asset& asset,
                       std::size_t slot,
                       const Profile& profile,
                       double time) {
    return PortfolioRunner::BacktestRun{
     {1, 1},
     {slot, 1},
     BacktestRunner{asset,
                    market,
                    broker,
                    profile,
                    {call_setup({{time, CallRow{.row = 2, .is_long = true}}},
                                call_rule(ValueMethod{1.0}))},
                    1000}};
  };
  auto runner = PortfolioRunner{1000,
                                3,
                                3,
                                {run(fresh, 1, fraction, 2),
                                 run(held, 2, fixed, 1),
                                 run(stale, 3, fixed, 1)}};
  auto results = PortfolioResults{};
  runner.run_batch(results, 10);
  const auto& decisions =
   results.backtests()[0].timeline().position_sizing_decisions(0);
  ASSERT_EQ(decisions.size(), 1U);
  ASSERT_TRUE(decisions.front().requested_quantity);
  EXPECT_NEAR(*decisions.front().requested_quantity, 1015.0 * 0.2 / 10.0, 1e-8);
}

TEST(CallStrategyTest, ConvertsNumberFormatsAndDistinguishesMissingFromZero)
{
  using Format = NumericInputNode::ValueRepresentation;
  const auto asset = make_asset("ABC", "1,100,100,100,100,1\n");
  const auto row = CallRow{
   .row = 2, .is_long = true, .fields = {{"number", -3.75}, {"zero", 0}}};
  auto series = StrategyMethodRegistry{};
  auto results = SeriesEvaluationResults{};
  const auto account = BacktestAccountState{};
  const auto ctx = BacktestMethodContext{series, results, 0, account, 0, &row};
  const auto snapshot = asset.get_snapshot(0);
  EXPECT_DOUBLE_EQ(evaluate_series_method(CallNode{"number"}, snapshot, ctx),
                   -3.75);
  EXPECT_DOUBLE_EQ(evaluate_series_method(
                    CallNode{"number", Format::SignedInteger}, snapshot, ctx),
                   -3);
  EXPECT_DOUBLE_EQ(evaluate_series_method(
                    CallNode{"number", Format::UnsignedInteger}, snapshot, ctx),
                   0);
  EXPECT_DOUBLE_EQ(evaluate_series_method(CallNode{"zero"}, snapshot, ctx), 0);
  EXPECT_TRUE(
   std::isnan(evaluate_series_method(CallNode{"missing"}, snapshot, ctx)));
}

TEST(CallRunnerTest, FilteredMainActivatesFailsafeWithItsOwnCallMapping)
{
  const auto asset = make_asset("ABC", "1,100,100,100,100,1\n");
  const auto market = Market{"Market"};
  const auto broker = Broker{"Broker"};
  const auto profile = fixed_quantity_profile();
  const auto make_setup = [](double risk) {
    return call_setup(
     {{1, CallRow{.row = 2, .is_long = true, .fields = {{"risk", risk}}}}},
     call_rule(as_method(CallNode{"risk"})));
  };
  auto main = make_setup(3);
  main.entry_filter = BooleanMethod<false>{};
  auto failsafe = make_setup(7);
  failsafe.failsafe_activation =
   FailsafeStrategyActivation::PreviousStrategyEntryFilteredPosition;
  auto runner =
   BacktestRunner{asset, market, broker, profile, {main, failsafe}, 1000};
  auto results = std::vector<SeriesEvaluationResults>(2);
  auto timeline = BacktestTimeline{};
  runner.run(results, timeline);
  EXPECT_TRUE(timeline.strategy_state(0, 0).entry_filtered_position);
  ASSERT_TRUE(timeline.open_position(0));
  EXPECT_DOUBLE_EQ(timeline.open_position(0)->risk_distance(), 7);
  ASSERT_EQ(timeline.trade_events(0).size(), 1U);
  EXPECT_EQ(timeline.trade_events(0).front().strategy_index(), 1U);
}

TEST(CallPortfolioTest, ComparatorRanksCallsUsingPreviousCompletedCandle)
{
  const auto first =
   make_asset("ABC", "1,100,100,80,80,1\n2,100,130,100,130,1\n");
  const auto second =
   make_asset("XYZ", "1,100,120,100,120,1\n2,100,100,70,70,1\n");
  const auto market = Market{"Market"};
  const auto broker = Broker{"Broker"};
  const auto profile = fixed_quantity_profile();
  const auto run = [&](const Asset& asset, std::size_t slot) {
    return PortfolioRunner::BacktestRun{
     {1, 1},
     {slot, 1},
     BacktestRunner{
      asset,
      market,
      broker,
      profile,
      {call_setup({{2, CallRow{.row = 2, .is_long = true}}}, call_rule())},
      1000}};
  };
  auto runner =
   PortfolioRunner{1000,
                   1,
                   1,
                   {run(first, 1), run(second, 2)},
                   {PortfolioEntryComparator{
                    CloseNode{}, PortfolioEntryComparatorOrder::HigherFirst}}};
  auto results = PortfolioResults{};
  runner.run_batch(results, 10);
  EXPECT_FALSE(results.backtests()[0].timeline().open_position(1));
  EXPECT_TRUE(results.backtests()[1].timeline().open_position(1));
}

TEST(CallRunnerTest, InvalidCallRiskAllowsPendingOppositeSignalAtSameOpen)
{
  const auto asset =
   make_asset("ABC", "1,100,100,100,100,1\n2,100,100,100,100,1\n");
  const auto market = Market{"Market"};
  const auto broker = Broker{"Broker"};
  const auto profile = fixed_quantity_profile();
  auto setup = call_setup(
   {{2, CallRow{.row = 2, .is_long = true, .fields = {{"risk", 100}}}}},
   call_rule(as_method(CallNode{"risk"})),
   BacktestRunner::PositionRule{BooleanMethod<true>{},
                                {},
                                BooleanMethod<false>{},
                                0,
                                0,
                                ValueMethod{5.0},
                                {},
                                SignalTiming::NextOpen});
  auto runner = BacktestRunner{asset, market, broker, profile, {setup}, 1000};
  auto results = std::vector<SeriesEvaluationResults>(1);
  auto timeline = BacktestTimeline{};
  runner.run(results, timeline);
  runner.run(results, timeline);
  ASSERT_TRUE(timeline.open_position(1));
  EXPECT_DOUBLE_EQ(timeline.open_position(1)->position_size(), -1);
  const auto& decisions = timeline.position_sizing_decisions(1);
  ASSERT_EQ(decisions.size(), 2U);
  EXPECT_EQ(decisions.front().call_table, "Desk calls");
  EXPECT_EQ(decisions.front().call_row, 2U);
  EXPECT_EQ(decisions.front().call_skip, "Invalid adverse 1R exit price");
}
