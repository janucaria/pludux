#include <gtest/gtest.h>

#include <cstddef>
#include <concepts>
#include <string>
#include <type_traits>
#include <vector>

import pludux;

using namespace pludux;

namespace {

struct ConcreteMethodContext {
  auto call_series_method(const std::string&, AssetSnapshot) const -> double
  {
    return 0.0;
  }

  auto call_series_method(const std::string&, AssetSnapshot, MethodOutput) const
   -> double
  {
    return 0.0;
  }

  auto get_series_result(const std::string&, std::size_t) const -> double
  {
    return 0.0;
  }

  template<typename TMethodKey>
  auto get_series_results(TMethodKey) -> std::vector<double>&
  {
    return results;
  }

  auto index() const -> std::size_t { return 0; }

  std::vector<double> results;
};

} // namespace

TEST(TypedCompositeNodeTest, LookbackConvertsAndEvaluatesForConcreteContext)
{
  const auto node = LookbackNode<ConcreteMethodContext>{
   ErasedNode<ConcreteMethodContext>{CloseNode{}}, 1};
  auto conversion_context = NodeToErasedMethodContext{};
  const auto method =
   node_to_erased_method<ConcreteMethodContext>(node, conversion_context);
  const auto history = AssetHistory{{"Close", {4.0, 3.0}}};
  const auto snapshot = AssetSnapshot{history};

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(method, snapshot[0], ConcreteMethodContext{}), 4.0);
}

TEST(TypedCompositeNodeTest, OperatorsComposeTypedChildrenAndEvaluate)
{
  const auto node = BinaryOperatorNode<std::plus<>, ConcreteMethodContext>{
   ErasedNode<ConcreteMethodContext>{ValueNode{2.0}},
   ErasedNode<ConcreteMethodContext>{
    UnaryOperatorNode<std::negate<>, ConcreteMethodContext>{
     ErasedNode<ConcreteMethodContext>{LookbackNode<ConcreteMethodContext>{
      ErasedNode<ConcreteMethodContext>{CloseNode{}}, 1}}}}};
  auto conversion_context = NodeToErasedMethodContext{};
  const auto method =
   node_to_erased_method<ConcreteMethodContext>(node, conversion_context);
  const auto history = AssetHistory{{"Close", {5.0, 3.0}}};
  const auto snapshot = AssetSnapshot{history};

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(method, snapshot[0], ConcreteMethodContext{}), -3.0);
}

TEST(TypedCompositeNodeTest, AllOfComposesAndEvaluatesForConcreteContext)
{
  const auto node = AllOfNode<ConcreteMethodContext>{{
   ErasedNode<ConcreteMethodContext>{TrueNode{}},
   ErasedNode<ConcreteMethodContext>{TrueNode{}}}};
  auto conversion_context = NodeToErasedMethodContext{};
  const auto method =
   node_to_erased_method<ConcreteMethodContext>(node, conversion_context);
  const auto snapshot = AssetSnapshot{AssetHistory{{"Close", {5.0}}}};

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(method, snapshot[0], ConcreteMethodContext{}), 1.0);
}

TEST(TypedCompositeNodeTest, AnyOfComposesAndEvaluatesForConcreteContext)
{
  const auto node = AnyOfNode<ConcreteMethodContext>{{
   ErasedNode<ConcreteMethodContext>{FalseNode{}},
   ErasedNode<ConcreteMethodContext>{TrueNode{}}}};
  auto conversion_context = NodeToErasedMethodContext{};
  const auto method =
   node_to_erased_method<ConcreteMethodContext>(node, conversion_context);
  const auto snapshot = AssetSnapshot{AssetHistory{{"Close", {5.0}}}};

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(method, snapshot[0], ConcreteMethodContext{}), 1.0);
}

TEST(TypedCompositeNodeTest, MacdConvertsTypedMultiInputChildren)
{
  const auto node = MacdNode<ConcreteMethodContext>{
   ErasedNode<ConcreteMethodContext>{CloseNode{}},
   ErasedNode<ConcreteMethodContext>{ValueNode{5.0}},
    ErasedNode<ConcreteMethodContext>{ValueNode{10.0}},
    ErasedNode<ConcreteMethodContext>{ValueNode{3.0}}};
  static_assert(std::same_as<decltype(node.source()),
                             const ErasedNode<ConcreteMethodContext>&>);
  static_assert(std::same_as<decltype(node.fast_period()),
                             const ErasedNode<ConcreteMethodContext>&>);
  static_assert(std::same_as<decltype(node.slow_period()),
                             const ErasedNode<ConcreteMethodContext>&>);
  static_assert(std::same_as<decltype(node.signal_period()),
                             const ErasedNode<ConcreteMethodContext>&>);
  auto conversion_context = NodeToErasedMethodContext{};
  const auto method =
   node_to_erased_method<ConcreteMethodContext>(node, conversion_context);
  static_assert(std::same_as<std::remove_cvref_t<decltype(method)>,
                             ErasedSeriesMethod<ConcreteMethodContext>>);
}

TEST(TypedCompositeNodeTest, StochRsiConvertsTypedMultiInputChildren)
{
  const auto node = StochRsiNode<ConcreteMethodContext>{
   ErasedNode<ConcreteMethodContext>{CloseNode{}},
   ErasedNode<ConcreteMethodContext>{ValueNode{3.0}},
   ErasedNode<ConcreteMethodContext>{ValueNode{3.0}},
   ErasedNode<ConcreteMethodContext>{ValueNode{2.0}},
   ErasedNode<ConcreteMethodContext>{ValueNode{2.0}}};
  auto conversion_context = NodeToErasedMethodContext{};
  const auto method =
   node_to_erased_method<ConcreteMethodContext>(node, conversion_context);
  static_assert(std::same_as<std::remove_cvref_t<decltype(method)>,
                             ErasedSeriesMethod<ConcreteMethodContext>>);
}
