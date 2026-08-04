#include <gtest/gtest.h>

#include <variant>

import pludux;

using namespace pludux;

TEST(TypedErasedNodeTest, ConvertsForExplicitMethodContext)
{
  const auto node = ErasedNode<ErasedSeriesMethodContext>{TrueNode{}};
  auto conversion_context = NodeToErasedMethodContext{};
  const auto method =
   node_to_erased_method<ErasedSeriesMethodContext>(node, conversion_context);

  EXPECT_DOUBLE_EQ(evaluate_series_method(method,
                                          AssetSnapshot{AssetHistory{}},
                                          ErasedSeriesMethodContext{}),
                   1.0);
}
