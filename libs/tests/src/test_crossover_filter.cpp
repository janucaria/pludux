#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(CrossoverMethodTest, ReferenceMethod)
{
  const auto signal_value = 40.0;
  const auto reference_value = 50.0;
  auto signal_method = ValueMethod{signal_value};
  auto reference_method = ValueMethod{reference_value};
  const auto condition =
   CrossoverMethod{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  EXPECT_EQ(
   evaluate_series_method(condition.reference(), asset_snapshot[0], context),
   reference_value);
}

TEST(CrossoverMethodTest, SignalMethod)
{
  const auto signal_value = 40.0;
  const auto reference_value = 50.0;
  auto signal_method = ValueMethod{signal_value};
  auto reference_method = ValueMethod{reference_value};
  const auto condition =
   CrossoverMethod{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  EXPECT_EQ(
   evaluate_series_method(condition.source(), asset_snapshot[0], context),
   signal_value);
}

TEST(CrossoverMethodTest, CrossoverConditionMet)
{
  auto signal_method = DataMethod{"close"};
  auto reference_method = ValueMethod{50.0};
  const auto crossover_condition =
   CrossoverMethod{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = AssetHistory{{"close", {60, 50}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  EXPECT_TRUE(
   evaluate_series_method(crossover_condition, asset_snapshot[0], context));
}

TEST(CrossoverMethodTest, CrossoverConditionNotMet)
{
  auto signal_method = DataMethod{"close"};
  auto reference_method = ValueMethod{50.0};
  const auto crossover_condition =
   CrossoverMethod{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = AssetHistory{{"close", {50, 40}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  EXPECT_FALSE(
   evaluate_series_method(crossover_condition, asset_snapshot[0], context));
}

TEST(CrossoverMethodTest, EqualityOperator)
{
  auto signal_method1 = DataMethod{"close"};
  auto reference_method1 = ValueMethod{50.0};
  const auto crossover_condition1 =
   CrossoverMethod{std::move(signal_method1), std::move(reference_method1)};

  auto signal_method2 = DataMethod{"close"};
  auto reference_method2 = ValueMethod{50.0};
  const auto crossover_condition2 =
   CrossoverMethod{std::move(signal_method2), std::move(reference_method2)};

  EXPECT_TRUE(crossover_condition1 == crossover_condition2);
  EXPECT_FALSE(crossover_condition1 != crossover_condition2);
  EXPECT_EQ(crossover_condition1, crossover_condition2);
}

TEST(CrossoverMethodTest, NotEqualOperator)
{
  auto signal_method1 = DataMethod{"close"};
  auto reference_method1 = ValueMethod{50.0};
  const auto crossover_condition1 =
   CrossoverMethod{std::move(signal_method1), std::move(reference_method1)};

  auto signal_method2 = DataMethod{"close"};
  auto reference_method2 = ValueMethod{60.0};
  const auto crossover_condition2 =
   CrossoverMethod{std::move(signal_method2), std::move(reference_method2)};

  EXPECT_TRUE(crossover_condition1 != crossover_condition2);
  EXPECT_FALSE(crossover_condition1 == crossover_condition2);
  EXPECT_NE(crossover_condition1, crossover_condition2);
}
