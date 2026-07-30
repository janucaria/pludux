#include <gtest/gtest.h>

#include <cmath>
#include <string>
#include <vector>

import pludux;

using namespace pludux;

TEST(SeriesMethodTest, RunAllMethodClose)
{
  const auto open_method = OpenMethod{};
  const auto close_method = CloseMethod{};
  const auto asset_data =
   AssetHistory{{"Open", {4.2, 4.1, 4.0}}, {"Close", {1.2, 1.1, 1.0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  series_methods.set("open", open_method);
  series_methods.set("close", close_method);

  auto results_collector = SeriesEvaluationResults{};
  results_collector.alias("open", open_method);
  results_collector.put(open_method, 4.2);
  results_collector.put(open_method, 4.1);
  results_collector.alias("close", close_method);
  results_collector.put(close_method, 1.2);
  results_collector.put(close_method, 1.1);

  auto context = DefaultMethodContext{
   series_methods, results_collector, asset_snapshot.index()};

  const auto open_series_method = SeriesMethod{"open"};
  EXPECT_EQ(
   evaluate_series_method(open_series_method, asset_snapshot[0], context), 4.0);
  EXPECT_EQ(
   evaluate_series_method(open_series_method, asset_snapshot[1], context), 4.1);
  EXPECT_EQ(
   evaluate_series_method(open_series_method, asset_snapshot[2], context), 4.2);

  const auto close_series_method = SeriesMethod{"close"};
  EXPECT_EQ(
   evaluate_series_method(close_series_method, asset_snapshot[0], context),
   1.0);
  EXPECT_EQ(
   evaluate_series_method(close_series_method, asset_snapshot[1], context),
   1.1);
  EXPECT_EQ(
   evaluate_series_method(close_series_method, asset_snapshot[2], context),
   1.2);
}

TEST(SeriesMethodTest, InvalidField)
{
  const auto close_method = CloseMethod{};
  const auto asset_data = AssetHistory{{"Close", {4.2, 4.1, 4.0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  auto series_methods =
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
  series_methods.set("close", close_method);

  auto results_collector = SeriesEvaluationResults{};
  auto context = DefaultMethodContext{
   series_methods, results_collector, asset_snapshot.index()};

  const auto not_found_series_method = SeriesMethod{"invalid"};

  EXPECT_TRUE(std::isnan(evaluate_series_method(
   not_found_series_method, asset_snapshot[0], context)));
  EXPECT_TRUE(std::isnan(evaluate_series_method(
   not_found_series_method, asset_snapshot[1], context)));
}

TEST(SeriesMethodTest, SeriesEvaluationResultsCanAliasNameToMethodKey)
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

TEST(SeriesMethodTest, EqualityOperator)
{
  const auto series_method1 = SeriesMethod{"close"};
  const auto series_method2 = SeriesMethod{"close"};

  EXPECT_EQ(series_method1, series_method2);
}

TEST(SeriesMethodTest, NotEqualOperator)
{
  const auto series_method1 = SeriesMethod{"close"};
  const auto series_method2 = SeriesMethod{"open"};

  EXPECT_TRUE(series_method1 != series_method2);
  EXPECT_NE(series_method1, series_method2);
}
