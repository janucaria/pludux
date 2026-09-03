#include <gtest/gtest.h>

#include "test_method_context.hpp"

#include <type_traits>
#include <vector>

import pludux;

using namespace pludux;

namespace {

struct UnsupportedNode {
  auto operator==(const UnsupportedNode&) const -> bool = default;
};

using TestErasedNode = ErasedNode<StatelessMethodContext>;
using TestNodeToErasedMethod =
 decltype(node_to_erased_method<StatelessMethodContext>);

static_assert(std::is_constructible_v<TestErasedNode, TrueNode>);
static_assert(std::is_convertible_v<TrueNode, TestErasedNode>);
static_assert(!std::is_constructible_v<TestErasedNode, UnsupportedNode>);
static_assert(std::is_invocable_r_v<
              ErasedSeriesMethod<StatelessMethodContext>,
              TestNodeToErasedMethod,
              const TrueNode&,
              NodeToErasedMethodContext&>);
static_assert(!std::is_invocable_v<TestNodeToErasedMethod,
                                   const UnsupportedNode&,
                                   NodeToErasedMethodContext&>);

} // namespace

auto value_method_value(
 const ErasedSeriesMethod<StatelessMethodContext>& method) noexcept -> double
{
  const auto* value_method = series_method_cast<ValueMethod>(method);
  EXPECT_NE(value_method, nullptr);
  return value_method == nullptr ? 0.0 : value_method->value();
}

TEST(NodeToErasedMethodTest, NumericInputNodeConvertsToValueMethod)
{
  const auto node = ErasedNode<StatelessMethodContext>{NumericInputNode{
   "Length", NumericInputNode::ValueRepresentation::SignedInteger, 14.9}};

  auto input_context = NodeToErasedMethodContext{};
  const auto method =
   node_to_erased_method<StatelessMethodContext>(node, input_context);
  const auto* value_method = series_method_cast<ValueMethod>(method);

  ASSERT_NE(value_method, nullptr);
  EXPECT_DOUBLE_EQ(value_method->value(), 14.0);
}

TEST(NodeToErasedMethodTest, NumericInputNodeConsumesContextByTraversalOrder)
{
  const auto node = ErasedNode<StatelessMethodContext>{
   AddNode<StatelessMethodContext>{
   NumericInputNode{
    "Duplicate", NumericInputNode::ValueRepresentation::Decimal, 1.0},
   NumericInputNode{
     "Duplicate", NumericInputNode::ValueRepresentation::UnsignedInteger, 2.0}}};

  const auto inputs = std::vector<double>{10.5, -4.8};
  auto input_context = NodeToErasedMethodContext{inputs};

  const auto method =
   node_to_erased_method<StatelessMethodContext>(node, input_context);
  const auto* add_method =
   series_method_cast<AddMethod<ErasedSeriesMethod<StatelessMethodContext>,
                                ErasedSeriesMethod<StatelessMethodContext>>>(
    method);

  ASSERT_NE(add_method, nullptr);

  const auto* first_value =
   series_method_cast<ValueMethod>(add_method->operand1());
  const auto* second_value =
   series_method_cast<ValueMethod>(add_method->operand2());

  ASSERT_NE(first_value, nullptr);
  ASSERT_NE(second_value, nullptr);
  EXPECT_DOUBLE_EQ(first_value->value(), 10.5);
  EXPECT_DOUBLE_EQ(second_value->value(), 0.0);
  EXPECT_EQ(input_context.input_index(), 2);
}

TEST(NodeToErasedMethodTest, MovingAveragePeriodConsumesContextInput)
{
  const auto node = ErasedNode<StatelessMethodContext>{
   SmaNode<StatelessMethodContext>{
   CloseNode{},
   NumericInputNode{
    "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 20.0}}};

  const auto inputs = std::vector<double>{7.0};
  auto input_context = NodeToErasedMethodContext{inputs};

  const auto method =
   node_to_erased_method<StatelessMethodContext>(node, input_context);
  const auto* sma_method =
   series_method_cast<SmaMethod<ErasedSeriesMethod<StatelessMethodContext>,
                                ErasedSeriesMethod<StatelessMethodContext>>>(
    method);

  ASSERT_NE(sma_method, nullptr);
  EXPECT_EQ(value_method_value(sma_method->period()), 7);
  EXPECT_EQ(input_context.input_index(), 1);
}

TEST(NodeToErasedMethodTest, PrimitiveMovingAveragePeriodBecomesValueMethod)
{
  const auto node =
   ErasedNode<StatelessMethodContext>{
    SmaNode<StatelessMethodContext>{CloseNode{}, 20}};

  auto input_context = NodeToErasedMethodContext{};

  const auto method =
   node_to_erased_method<StatelessMethodContext>(node, input_context);
  const auto* sma_method =
   series_method_cast<SmaMethod<ErasedSeriesMethod<StatelessMethodContext>,
                                ErasedSeriesMethod<StatelessMethodContext>>>(
    method);

  ASSERT_NE(sma_method, nullptr);
  EXPECT_EQ(value_method_value(sma_method->period()), 20);
  EXPECT_EQ(input_context.input_index(), 0);
}

TEST(NodeToErasedMethodTest, BollingerBandInputsConsumeContextInOrder)
{
  const auto node = ErasedNode<StatelessMethodContext>{BbNode<StatelessMethodContext>{
   CloseNode{},
   NumericInputNode{
    "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 20.0},
   NumericInputNode{
    "StdDev", NumericInputNode::ValueRepresentation::Decimal, 2.0}}};

  const auto inputs = std::vector<double>{7.0, 1.25};
  auto input_context = NodeToErasedMethodContext{inputs};

  const auto method =
   node_to_erased_method<StatelessMethodContext>(node, input_context);
  const auto* bb_method =
   series_method_cast<BbMethod<ErasedSeriesMethod<StatelessMethodContext>,
                               ErasedSeriesMethod<StatelessMethodContext>>>(
    method);

  ASSERT_NE(bb_method, nullptr);
  EXPECT_EQ(value_method_value(bb_method->period()), 7);
  EXPECT_DOUBLE_EQ(value_method_value(bb_method->stddev()), 1.25);
  EXPECT_EQ(input_context.input_index(), 2);
}

TEST(NodeToErasedMethodTest, KeltnerChannelInputsConsumeContextInOrder)
{
  const auto node = ErasedNode<StatelessMethodContext>{KcNode<StatelessMethodContext>{
   CloseNode{},
   NumericInputNode{
    "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 20.0},
   NumericInputNode{
    "Multiplier", NumericInputNode::ValueRepresentation::Decimal, 1.5},
   NumericInputNode{"Band ATR Period",
                    NumericInputNode::ValueRepresentation::UnsignedInteger,
                    14.0}}};

  const auto inputs = std::vector<double>{9.0, 2.25, 6.0};
  auto input_context = NodeToErasedMethodContext{inputs};

  const auto method =
   node_to_erased_method<StatelessMethodContext>(node, input_context);
  const auto* kc_method =
   series_method_cast<KcMethod<ErasedSeriesMethod<StatelessMethodContext>,
                               ErasedSeriesMethod<StatelessMethodContext>>>(
    method);

  ASSERT_NE(kc_method, nullptr);
  EXPECT_EQ(value_method_value(kc_method->period()), 9);
  EXPECT_DOUBLE_EQ(value_method_value(kc_method->multiplier()), 2.25);
  EXPECT_EQ(value_method_value(kc_method->band_atr_period()), 6);
  EXPECT_EQ(input_context.input_index(), 3);
}

TEST(NodeToErasedMethodTest, MacdInputsConsumeContextInOrder)
{
  const auto node = ErasedNode<StatelessMethodContext>{MacdNode<StatelessMethodContext>{
   CloseNode{},
   NumericInputNode{"Fast Period",
                    NumericInputNode::ValueRepresentation::UnsignedInteger,
                    12.0},
   NumericInputNode{"Slow Period",
                    NumericInputNode::ValueRepresentation::UnsignedInteger,
                    26.0},
   NumericInputNode{"Signal Period",
                    NumericInputNode::ValueRepresentation::UnsignedInteger,
                    9.0}}};

  const auto inputs = std::vector<double>{5.0, 13.0, 3.0};
  auto input_context = NodeToErasedMethodContext{inputs};

  const auto method =
   node_to_erased_method<StatelessMethodContext>(node, input_context);
  const auto* macd_method = series_method_cast<
   MacdMethod<ErasedSeriesMethod<StatelessMethodContext>,
              ErasedSeriesMethod<StatelessMethodContext>>>(method);

  ASSERT_NE(macd_method, nullptr);
  EXPECT_EQ(value_method_value(macd_method->fast_period()), 5);
  EXPECT_EQ(value_method_value(macd_method->slow_period()), 13);
  EXPECT_EQ(value_method_value(macd_method->signal_period()), 3);
  EXPECT_EQ(input_context.input_index(), 3);
}

TEST(NodeToErasedMethodTest, StochRsiInputsConsumeContextInOrder)
{
  const auto node = ErasedNode<StatelessMethodContext>{StochRsiNode<StatelessMethodContext>{
   CloseNode{},
   NumericInputNode{
    "RSI Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 14.0},
   NumericInputNode{
    "K Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 5.0},
   NumericInputNode{
    "K Smooth", NumericInputNode::ValueRepresentation::UnsignedInteger, 3.0},
   NumericInputNode{
    "D Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 3.0}}};

  const auto inputs = std::vector<double>{21.0, 8.0, 4.0, 2.0};
  auto input_context = NodeToErasedMethodContext{inputs};

  const auto method =
   node_to_erased_method<StatelessMethodContext>(node, input_context);
  const auto* stoch_rsi_method = series_method_cast<
   StochRsiMethod<ErasedSeriesMethod<StatelessMethodContext>,
                  ErasedSeriesMethod<StatelessMethodContext>>>(method);

  ASSERT_NE(stoch_rsi_method, nullptr);
  EXPECT_EQ(value_method_value(stoch_rsi_method->rsi_period()), 21);
  EXPECT_EQ(value_method_value(stoch_rsi_method->k_period()), 8);
  EXPECT_EQ(value_method_value(stoch_rsi_method->k_smooth()), 4);
  EXPECT_EQ(value_method_value(stoch_rsi_method->d_period()), 2);
  EXPECT_EQ(input_context.input_index(), 4);
}
