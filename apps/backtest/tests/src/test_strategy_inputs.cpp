#include <gtest/gtest.h>

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

  auto long_pyramiding = Strategy::Pyramiding{};
  long_pyramiding.signal(NumericInputNode{
   "Long Pyramid", NumericInputNode::ValueRepresentation::Decimal, 3.5});

  auto long_side = Strategy::PositionSide{};
  long_side.pyramiding(std::move(long_pyramiding));

  auto positions = Strategy::Positions{};
  positions.long_side(std::move(long_side));

  const auto strategy = Strategy{
   "Test",
   std::move(series_nodes),
   NumericInputNode{
    "Long Entry", NumericInputNode::ValueRepresentation::UnsignedInteger, 4.8},
   FalseNode{},
   FalseNode{},
   FalseNode{},
   std::move(positions),
   false,
   false,
   false,
   1.0,
   {}};

  const auto inputs = collect_numeric_inputs(strategy);

  ASSERT_EQ(inputs.size(), 4);
  EXPECT_EQ(inputs[0].label(), "Duplicate");
  EXPECT_DOUBLE_EQ(inputs[0].value(), 1.5);
  EXPECT_EQ(inputs[1].label(), "Duplicate");
  EXPECT_DOUBLE_EQ(inputs[1].value(), 2.8);
  EXPECT_EQ(inputs[2].label(), "Long Entry");
  EXPECT_DOUBLE_EQ(inputs[2].value(), 4.8);
  EXPECT_EQ(inputs[3].label(), "Long Pyramid");
  EXPECT_DOUBLE_EQ(inputs[3].value(), 3.5);
}
