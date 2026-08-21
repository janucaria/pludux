#include <gtest/gtest.h>

#include "test_method_context.hpp"

#include <variant>

import pludux;

using namespace pludux;

TEST(TypedErasedNodeTest, ConvertsForExplicitMethodContext)
{
  const auto node = ErasedNode<StatelessMethodContext>{TrueNode{}};
  auto conversion_context = NodeToErasedMethodContext{};
  const auto method =
   node_to_erased_method<StatelessMethodContext>(node, conversion_context);

  EXPECT_DOUBLE_EQ(evaluate_series_method(method,
                                          AssetSnapshot{AssetHistory{}},
                                          StatelessMethodContext{}),
                   1.0);
}
