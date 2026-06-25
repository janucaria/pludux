#include <gtest/gtest.h>

#include <vector>

import pludux;

using namespace pludux;

TEST(NodeToErasedMethodTest, NumericInputNodeConvertsToValueMethod)
{
  const auto node = ErasedNode{NumericInputNode{
   "Length", NumericInputNode::ValueRepresentation::SignedInteger, 14.9}};

  auto input_context = NodeToErasedMethodContext{};
  const auto method = node_to_erased_method(node, input_context);
  const auto* value_method = series_method_cast<ValueMethod>(method);

  ASSERT_NE(value_method, nullptr);
  EXPECT_DOUBLE_EQ(value_method->value(), 14.0);
}

TEST(NodeToErasedMethodTest, NumericInputNodeConsumesContextByTraversalOrder)
{
  const auto node = ErasedNode{AddNode{
   NumericInputNode{
    "Duplicate", NumericInputNode::ValueRepresentation::Decimal, 1.0},
   NumericInputNode{
    "Duplicate", NumericInputNode::ValueRepresentation::UnsignedInteger, 2.0}}};

  const auto inputs = std::vector<NumericInputNode>{
   NumericInputNode{
    "Duplicate", NumericInputNode::ValueRepresentation::Decimal, 10.5},
   NumericInputNode{
    "Duplicate", NumericInputNode::ValueRepresentation::UnsignedInteger, -4.8}};
  auto input_context = NodeToErasedMethodContext{inputs};

  const auto method = node_to_erased_method(node, input_context);
  const auto* add_method =
   series_method_cast<AddMethod<AnySeriesMethod, AnySeriesMethod>>(method);

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
