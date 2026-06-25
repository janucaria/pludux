#include <gtest/gtest.h>

#include <cmath>
#include <string>
#include <vector>

import pludux;

using namespace pludux;

TEST(SeriesReferenceMethodTest, RunAllMethodClose)
{
  const auto open_method = OpenMethod{};
  const auto close_method = CloseMethod{};
  const auto asset_data =
   AssetHistory{{"Open", {4.0, 4.1, 4.2}}, {"Close", {1.0, 1.1, 1.2}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  auto series_methods = OrderedNamedRegistry<AnySeriesMethod>{};
  series_methods.set("open", open_method);
  series_methods.set("close", close_method);

  auto results_collector = SeriesEvaluationResults{};
  auto context =
   DefaultMethodContext{series_methods, results_collector};

  const auto open_ref_method = SeriesNodeMethod{"open"};
  EXPECT_EQ(evaluate_series_method(open_ref_method, asset_snapshot[0], context),
            4.0);
  EXPECT_EQ(evaluate_series_method(open_ref_method, asset_snapshot[1], context),
            4.1);
  EXPECT_EQ(evaluate_series_method(open_ref_method, asset_snapshot[2], context),
            4.2);

  const auto close_ref_method = SeriesNodeMethod{"close"};
  EXPECT_EQ(
   evaluate_series_method(close_ref_method, asset_snapshot[0], context), 1.0);
  EXPECT_EQ(
   evaluate_series_method(close_ref_method, asset_snapshot[1], context), 1.1);
  EXPECT_EQ(
   evaluate_series_method(close_ref_method, asset_snapshot[2], context), 1.2);
}

TEST(SeriesReferenceMethodTest, InvalidField)
{
  const auto close_method = CloseMethod{};
  const auto asset_data = AssetHistory{{"Close", {4.0, 4.1, 4.2}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  auto series_methods = OrderedNamedRegistry<AnySeriesMethod>{};
  series_methods.set("close", close_method);

  auto results_collector = SeriesEvaluationResults{};
  auto context =
   DefaultMethodContext{series_methods, results_collector};

  const auto not_found_ref_method = SeriesNodeMethod{"invalid"};

  EXPECT_TRUE(std::isnan(
   evaluate_series_method(not_found_ref_method, asset_snapshot[0], context)));
  EXPECT_TRUE(std::isnan(
   evaluate_series_method(not_found_ref_method, asset_snapshot[1], context)));
}

TEST(SeriesReferenceMethodTest, SeriesEvaluationResultsCanAliasNameToMethodKey)
{
  const auto close_name = std::string{"close"};
  const auto close_method = CloseMethod{};

  auto results_collector = SeriesEvaluationResults{};
  results_collector.alias(close_name, close_method);
  results_collector.put(close_method, 1.0);
  results_collector.put(close_method, 1.1);

  const auto close_results = results_collector.results(close_name);
  ASSERT_TRUE(close_results.has_value());
  EXPECT_EQ(close_results->get(), (std::vector<double>{1.0, 1.1}));

  const auto missing_results =
   results_collector.results(std::string{"missing"});
  EXPECT_FALSE(missing_results.has_value());
}

TEST(SeriesReferenceMethodTest, EqualityOperator)
{
  const auto ref_method1 = SeriesNodeMethod{"close"};
  const auto ref_method2 = SeriesNodeMethod{"close"};

  EXPECT_EQ(ref_method1, ref_method2);
}

TEST(SeriesReferenceMethodTest, NotEqualOperator)
{
  const auto ref_method1 = SeriesNodeMethod{"close"};
  const auto ref_method2 = SeriesNodeMethod{"open"};

  EXPECT_TRUE(ref_method1 != ref_method2);
  EXPECT_NE(ref_method1, ref_method2);
}
