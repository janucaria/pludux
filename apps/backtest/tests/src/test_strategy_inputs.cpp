#include <gtest/gtest.h>

#include <jsoncons/json.hpp>

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
  long_position.take_profit(Strategy::TakeProfit{
   true,
   NumericInputNode{
    "Target Price", NumericInputNode::ValueRepresentation::Decimal, 120.0}});

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
          "signal": false
        },
        "stopLoss": {
          "enabled": true,
          "trailing": true,
          "stopPrice": "OPEN"
        },
        "takeProfit": {
          "enabled": true,
          "targetPrice": 120
        },
        "pyramiding": {
          "signalDelay": 0,
          "price": "CLOSE",
          "signal": false,
          "maxLayers": 2
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
  EXPECT_EQ(strategy.long_position().pyramiding().signal_delay(), 0);
  EXPECT_TRUE(
   node_cast<CloseNode>(strategy.long_position().pyramiding().price()));
  EXPECT_TRUE(strategy.long_position().stop_loss().trailing());
  EXPECT_TRUE(
   node_cast<OpenNode>(strategy.long_position().stop_loss().stop_price()));
  EXPECT_TRUE(strategy.long_position().take_profit().enabled());
  EXPECT_TRUE(
   node_cast<ValueNode>(strategy.long_position().take_profit().target_price()));
  EXPECT_FALSE(strategy.short_position().entry().signal() ==
               strategy.long_position().entry().signal());
}

TEST(StrategyParserTest, StringifiesPositionObjectsWithPerSideLevels)
{
  auto long_position = Strategy::Position{};
  long_position.entry(Strategy::Entry{TrueNode{}, 0, CloseNode{}});
  long_position.exit(Strategy::Exit{FalseNode{}, 0, CloseNode{}});
  auto pyramiding = Strategy::Pyramiding{};
  pyramiding.signal_delay(0);
  pyramiding.price(CloseNode{});
  long_position.pyramiding(pyramiding);
  long_position.stop_loss(Strategy::StopLoss{true, OpenNode{}, false});
  long_position.take_profit(Strategy::TakeProfit{true, ValueNode{120.0}});

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
               .at("takeProfit")
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
  EXPECT_FALSE(strategy_json.contains("stopLoss"));
  EXPECT_FALSE(strategy_json.contains("takeProfit"));
}
