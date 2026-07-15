#include <gtest/gtest.h>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

TEST(StrategyInputsTest, CollectsNumericInputsInStrategyTraversalOrder)
{
  auto series_nodes = OrderedNamedRegistry<ErasedNode>{};
  series_nodes.set(
   "spread",
   AddNode{
    NumericInputNode{
     "Duplicate", NumericInputNode::ValueRepresentation::Decimal, 1.5},
    NumericInputNode{
     "Duplicate", NumericInputNode::ValueRepresentation::SignedInteger, 2.8}});
  series_nodes.set(
   "channel",
   KcNode{
    CloseNode{},
    NumericInputNode{
     "KC Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 20.0},
    NumericInputNode{
     "KC Multiplier", NumericInputNode::ValueRepresentation::Decimal, 1.5},
    NumericInputNode{"KC Band ATR",
                     NumericInputNode::ValueRepresentation::UnsignedInteger,
                     14.0}});

  auto long_pyramiding = Strategy::Pyramiding{};
  long_pyramiding.signal(NumericInputNode{
   "Long Pyramid", NumericInputNode::ValueRepresentation::Decimal, 3.5});

  auto long_position = Strategy::Position{};
  long_position.entry(Strategy::Entry{
   NumericInputNode{
    "Long Entry", NumericInputNode::ValueRepresentation::UnsignedInteger, 4.8},
   1,
   NumericInputNode{
    "Entry Price", NumericInputNode::ValueRepresentation::Decimal, 101.0}});
  long_position.pyramiding(std::move(long_pyramiding));
  long_position.stop_loss(Strategy::StopLoss{
   false,
   NumericInputNode{
    "Stop Price", NumericInputNode::ValueRepresentation::Decimal, 95.0},
   false});
  long_position.take_profits({Strategy::TakeProfit{
   true,
   NumericInputNode{
    "Target Price", NumericInputNode::ValueRepresentation::Decimal, 120.0}}});

  const auto strategy = Strategy{"Test",
                                 std::move(series_nodes),
                                 std::move(long_position),
                                 Strategy::Position{},
                                 {}};

  const auto inputs = collect_numeric_inputs(strategy);

  ASSERT_EQ(inputs.size(), 10);
  EXPECT_EQ(inputs[0].label(), "Duplicate");
  EXPECT_DOUBLE_EQ(inputs[0].value(), 1.5);
  EXPECT_EQ(inputs[1].label(), "Duplicate");
  EXPECT_DOUBLE_EQ(inputs[1].value(), 2.8);
  EXPECT_EQ(inputs[2].label(), "KC Period");
  EXPECT_DOUBLE_EQ(inputs[2].value(), 20.0);
  EXPECT_EQ(inputs[3].label(), "KC Multiplier");
  EXPECT_DOUBLE_EQ(inputs[3].value(), 1.5);
  EXPECT_EQ(inputs[4].label(), "KC Band ATR");
  EXPECT_DOUBLE_EQ(inputs[4].value(), 14.0);
  EXPECT_EQ(inputs[5].label(), "Long Entry");
  EXPECT_DOUBLE_EQ(inputs[5].value(), 4.8);
  EXPECT_EQ(inputs[6].label(), "Entry Price");
  EXPECT_DOUBLE_EQ(inputs[6].value(), 101.0);
  EXPECT_EQ(inputs[7].label(), "Long Pyramid");
  EXPECT_DOUBLE_EQ(inputs[7].value(), 3.5);
  EXPECT_EQ(inputs[8].label(), "Stop Price");
  EXPECT_DOUBLE_EQ(inputs[8].value(), 95.0);
  EXPECT_EQ(inputs[9].label(), "Target Price");
  EXPECT_DOUBLE_EQ(inputs[9].value(), 120.0);
}

TEST(StrategyParserTest, ParsesPerSideStopLossAndTakeProfit)
{
  const auto strategy_json = std::string{R"({
    "version": 2,
    "positions": {
      "long": {
        "entry": {
          "signalDelay": 0,
          "price": "CLOSE",
          "signal": true
        },
        "exit": {
          "signalDelay": 0,
          "price": "CLOSE",
          "signal": false,
          "reduce": 0.25
        },
        "stopLoss": {
          "enabled": true,
          "trailing": true,
          "stopPrice": "OPEN",
          "reduce": 0.5
        },
        "takeProfits": [
          {
            "enabled": true,
            "targetPrice": 120,
            "reduce": 0.75
          },
          {
            "enabled": false,
            "targetPrice": 140,
            "reduce": 0.5
          }
        ],
        "pyramiding": {
          "signalDelay": 0,
          "price": "CLOSE",
          "signal": false,
          "maxLayers": 2,
          "stopTargetReference": {
            "favorable": "INITIAL_ENTRY_PRICE",
            "unfavorable": "LATEST_ENTRY_PRICE"
          }
        }
      },
      "short": false
    }
  })"};

  const auto strategy = parse_backtest_strategy_json("Test", strategy_json);

  EXPECT_TRUE(strategy.long_position().stop_loss().enabled());
  EXPECT_EQ(strategy.long_position().entry().signal_delay(), 0);
  EXPECT_TRUE(node_cast<CloseNode>(strategy.long_position().entry().price()));
  EXPECT_EQ(strategy.long_position().exit().signal_delay(), 0);
  EXPECT_TRUE(node_cast<CloseNode>(strategy.long_position().exit().price()));
  EXPECT_DOUBLE_EQ(strategy.long_position().exit().reduce(), 0.25);
  EXPECT_EQ(strategy.long_position().pyramiding().signal_delay(), 0);
  EXPECT_TRUE(
   node_cast<CloseNode>(strategy.long_position().pyramiding().price()));
  EXPECT_EQ(
   strategy.long_position().pyramiding().favorable_stop_target_reference(),
   StopTargetReferencePrice::InitialEntryPrice);
  EXPECT_EQ(
   strategy.long_position().pyramiding().unfavorable_stop_target_reference(),
   StopTargetReferencePrice::LatestEntryPrice);
  EXPECT_TRUE(strategy.long_position().stop_loss().trailing());
  EXPECT_DOUBLE_EQ(strategy.long_position().stop_loss().reduce(), 0.5);
  EXPECT_TRUE(
   node_cast<OpenNode>(strategy.long_position().stop_loss().stop_price()));
  ASSERT_EQ(strategy.long_position().take_profits().size(), 2);
  EXPECT_TRUE(strategy.long_position().take_profits()[0].enabled());
  EXPECT_DOUBLE_EQ(strategy.long_position().take_profits()[0].reduce(), 0.75);
  EXPECT_FALSE(strategy.long_position().take_profits()[1].enabled());
  EXPECT_TRUE(node_cast<ValueNode>(
   strategy.long_position().take_profits()[0].target_price()));
  EXPECT_FALSE(strategy.short_position().entry().signal() ==
               strategy.long_position().entry().signal());
}

TEST(StrategyParserTest, StringifiesPositionObjectsWithPerSideLevels)
{
  auto long_position = Strategy::Position{};
  long_position.entry(Strategy::Entry{TrueNode{}, 0, CloseNode{}});
  long_position.exit(Strategy::Exit{FalseNode{}, 0, CloseNode{}, 0.25});
  auto pyramiding = Strategy::Pyramiding{};
  pyramiding.signal_delay(0);
  pyramiding.price(CloseNode{});
  pyramiding.favorable_stop_target_reference(
   StopTargetReferencePrice::InitialEntryPrice);
  pyramiding.unfavorable_stop_target_reference(
   StopTargetReferencePrice::LatestEntryPrice);
  long_position.pyramiding(pyramiding);
  long_position.stop_loss(Strategy::StopLoss{true, OpenNode{}, false, 0.5});
  long_position.take_profits(
   {Strategy::TakeProfit{true, ValueNode{120.0}, 0.75}});

  const auto strategy =
   Strategy{"Test", {}, std::move(long_position), Strategy::Position{}, {}};

  const auto strategy_json =
   jsoncons::ojson::parse(stringify_backtest_strategy(strategy));

  EXPECT_TRUE(strategy_json.at("positions").at("long").is_object());
  EXPECT_TRUE(strategy_json.at("positions").at("short").is_object());
  EXPECT_TRUE(strategy_json.at("positions")
               .at("long")
               .at("stopLoss")
               .contains("stopPrice"));
  EXPECT_TRUE(strategy_json.at("positions")
               .at("long")
               .at("takeProfits")
               .at(0)
               .contains("targetPrice"));
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("entry")
             .at("signalDelay")
             .as<std::size_t>(),
            0);
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("entry")
             .at("price")
             .at("method")
             .as<std::string>(),
            "CLOSE");
  EXPECT_DOUBLE_EQ(strategy_json.at("positions")
                    .at("long")
                    .at("exit")
                    .at("reduce")
                    .as<double>(),
                   0.25);
  EXPECT_DOUBLE_EQ(strategy_json.at("positions")
                    .at("long")
                    .at("stopLoss")
                    .at("reduce")
                    .as<double>(),
                   0.5);
  EXPECT_DOUBLE_EQ(strategy_json.at("positions")
                    .at("long")
                    .at("takeProfits")
                    .at(0)
                    .at("reduce")
                    .as<double>(),
                   0.75);
  EXPECT_FALSE(strategy_json.at("positions")
                .at("long")
                .at("exit")
                .contains("reduceRounding"));
  EXPECT_FALSE(strategy_json.at("positions")
                .at("long")
                .at("stopLoss")
                .contains("reduceRounding"));
  EXPECT_FALSE(strategy_json.at("positions")
                .at("long")
                .at("takeProfits")
                .at(0)
                .contains("reduceRounding"));
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("exit")
             .at("signalDelay")
             .as<std::size_t>(),
            0);
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("exit")
             .at("price")
             .at("method")
             .as<std::string>(),
            "CLOSE");
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("pyramiding")
             .at("signalDelay")
             .as<std::size_t>(),
            0);
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("pyramiding")
             .at("price")
             .at("method")
             .as<std::string>(),
            "CLOSE");
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("pyramiding")
             .at("stopTargetReference")
             .at("favorable")
             .as<std::string>(),
            "INITIAL_ENTRY_PRICE");
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("pyramiding")
             .at("stopTargetReference")
             .at("unfavorable")
             .as<std::string>(),
            "LATEST_ENTRY_PRICE");
  EXPECT_FALSE(strategy_json.contains("stopLoss"));
  EXPECT_FALSE(strategy_json.contains("takeProfit"));
}

TEST(StrategyParserTest, JsonconsConvTraitsRoundTripSchemaConfig)
{
  auto long_position = Strategy::Position{};
  long_position.entry(Strategy::Entry{TrueNode{}, 0, CloseNode{}});
  long_position.stop_loss(Strategy::StopLoss{true, OpenNode{}, false});

  const auto strategy = Strategy{
   "Config Name", {}, std::move(long_position), Strategy::Position{}, {}};

  auto strategy_json_str = std::string{};
  jsoncons::encode_json(strategy, strategy_json_str);

  const auto strategy_json = jsoncons::ojson::parse(strategy_json_str);
  EXPECT_EQ(strategy_json.at("version").as<int>(), 2);
  EXPECT_FALSE(strategy_json.contains("name"));

  const auto decoded_strategy =
   jsoncons::decode_json<Strategy>(strategy_json_str);

  EXPECT_TRUE(decoded_strategy.name().empty());
  EXPECT_TRUE(decoded_strategy.equivalent_rules(strategy));
}

TEST(StrategyParserTest, DefaultStopLossAndEmptyTakeProfits)
{
  const auto strategy = Strategy{};

  const auto* stop_price =
   node_cast<SlAtrNode>(strategy.long_position().stop_loss().stop_price());
  ASSERT_NE(stop_price, nullptr);
  const auto* stop_multiplier = node_cast<ValueNode>(stop_price->multiplier());
  ASSERT_NE(stop_multiplier, nullptr);
  EXPECT_DOUBLE_EQ(stop_multiplier->value(), 2.0);

  EXPECT_TRUE(strategy.long_position().take_profits().empty());
}

TEST(StrategyParserTest, RejectsLegacySingularTakeProfit)
{
  EXPECT_THROW(
   parse_backtest_strategy_json(
    "Legacy",
    R"({"version":2,"positions":{"long":{"stopLoss":{"stopPrice":"OPEN"},"takeProfit":{"enabled":true,"targetPrice":120}},"short":false}})"),
   std::runtime_error);
}

TEST(StrategyParserTest, LoadsEveryBundledStrategySample)
{
  const auto sample_directory =
   std::filesystem::path{PLUDUX_BACKTEST_SAMPLE_DIR};
  auto schema_stream =
   std::ifstream{sample_directory.parent_path() / "schemas" /
                 "pludux.backtest.strategy.v2-draft.json"};
  auto schema = jsoncons::ojson::parse(schema_stream);
  const auto compiled_schema =
   jsoncons::jsonschema::make_json_schema(std::move(schema));
  auto sample_count = std::size_t{0};
  for(const auto& entry :
      std::filesystem::directory_iterator{sample_directory}) {
    if(entry.path().extension() != ".json") {
      continue;
    }
    auto stream = std::ifstream{entry.path()};
    auto json = std::ostringstream{};
    json << stream.rdbuf();
    const auto document = jsoncons::ojson::parse(json.str());
    EXPECT_NO_THROW(compiled_schema.validate(document))
     << entry.path().string();
    EXPECT_NO_THROW(
     parse_backtest_strategy_json(entry.path().stem().string(), json.str()))
     << entry.path().string();
    ++sample_count;
  }
  EXPECT_GT(sample_count, 0);
}
