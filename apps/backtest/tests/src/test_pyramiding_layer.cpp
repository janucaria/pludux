#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>

#include <jsoncons/json.hpp>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;
using json = jsoncons::ojson;

namespace {

auto make_snapshot() -> AssetSnapshot
{
  static const auto asset = Asset{"Test",
                                  AssetHistory{{"Datetime", {1.0}},
                                               {"Open", {100.0}},
                                               {"High", {100.0}},
                                               {"Low", {100.0}},
                                               {"Close", {100.0}},
                                               {"Volume", {0.0}}}};
  return asset.get_snapshot(0);
}

} // namespace

TEST(PyramidingLayerMethodTest, ReadsLayerFromBacktestContext)
{
  const auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<BacktestMethodContext>>{};
  auto series_results = SeriesEvaluationResults{};
  const auto account_state = BacktestAccountState{};
  const auto flat_context =
   BacktestMethodContext{series_methods, series_results, 0, account_state, 0};
  const auto layered_context =
   BacktestMethodContext{series_methods, series_results, 0, account_state, 3};
  const auto snapshot = make_snapshot();

  EXPECT_DOUBLE_EQ(
   evaluate_series_method(PyramidingLayerMethod{}, snapshot, flat_context),
   0.0);
  EXPECT_DOUBLE_EQ(
   evaluate_series_method(PyramidingLayerMethod{}, snapshot, layered_context),
   3.0);
}

TEST(PyramidingLayerMethodTest, HasStableIdentity)
{
  EXPECT_EQ(PyramidingLayerMethod{}, PyramidingLayerMethod{});
  EXPECT_EQ(hash_series_method(PyramidingLayerMethod{}),
            hash_series_method(PyramidingLayerMethod{}));
}

TEST(PyramidingLayerNodeTest, ConvertsToPyramidingLayerMethod)
{
  auto context = NodeToErasedMethodContext{};
  const auto method =
   node_to_erased_method<BacktestMethodContext>(PyramidingLayerNode{}, context);

  EXPECT_NE(series_method_cast<PyramidingLayerMethod>(method), nullptr);
}

TEST(PyramidingLayerNodeTest, ParsesAndSerializesShorthandAndObjectForms)
{
  auto parser = make_backtest_model_config_parser();
  const auto shorthand = json::parse(R"("POSITION.PYRAMIDING_LAYER")");
  const auto object =
   json::parse(R"({"method":"POSITION.PYRAMIDING_LAYER","params":{}})");
  const auto canonical =
   json::parse(R"({"method":"POSITION.PYRAMIDING_LAYER"})");

  const auto shorthand_node = parser.parse_node(shorthand);
  const auto object_node = parser.parse_node(object);
  EXPECT_NE(node_cast<PyramidingLayerNode>(shorthand_node), nullptr);
  EXPECT_NE(node_cast<PyramidingLayerNode>(object_node), nullptr);
  EXPECT_EQ(parser.serialize_node(shorthand_node), canonical);
  EXPECT_EQ(parser.serialize_node(object_node), canonical);
}
