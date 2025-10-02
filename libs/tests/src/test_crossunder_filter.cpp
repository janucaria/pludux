#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = AnySeriesMethodContext{};

TEST(CrossunderMethodTest, ReferenceMethod)
{
  const auto signal_value = 40.0;
  const auto reference_value = 50.0;
  auto signal_method = ValueMethod{signal_value};
  auto reference_method = ValueMethod{reference_value};
  const auto condition =
   CrossunderMethod{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_EQ(condition.reference()(asset_data, context), reference_value);
}

TEST(CrossunderMethodTest, SignalMethod)
{
  const auto signal_value = 40.0;
  const auto reference_value = 50.0;
  auto signal_method = ValueMethod{signal_value};
  auto reference_method = ValueMethod{reference_value};
  const auto condition =
   CrossunderMethod{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_EQ(condition.signal()(asset_data, context), signal_value);
}

TEST(CrossunderMethodTest, CrossunderConditionMet)
{
  auto signal_method = DataMethod{"close"};
  auto reference_method = ValueMethod{50.0};
  const auto crossunder_condition =
   CrossunderMethod{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = AssetHistory{{"close", {40, 60}}};

  EXPECT_TRUE(crossunder_condition(asset_data, context));
}

TEST(CrossunderMethodTest, CrossunderConditionNotMet)
{
  auto signal_method = DataMethod{"close"};
  auto reference_method = ValueMethod{50.0};
  const auto crossunder_condition =
   CrossunderMethod{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = AssetHistory{{"close", {50, 40}}};

  EXPECT_FALSE(crossunder_condition(asset_data, context));
}

TEST(CrossunderMethodTest, EqualityOperator)
{
  auto signal_method1 = DataMethod{"close"};
  auto reference_method1 = ValueMethod{50.0};
  const auto crossunder_condition1 =
   CrossunderMethod{std::move(signal_method1), std::move(reference_method1)};

  auto signal_method2 = DataMethod{"close"};
  auto reference_method2 = ValueMethod{50.0};
  const auto crossunder_condition2 =
   CrossunderMethod{std::move(signal_method2), std::move(reference_method2)};

  EXPECT_TRUE(crossunder_condition1 == crossunder_condition2);
  EXPECT_FALSE(crossunder_condition1 != crossunder_condition2);
  EXPECT_EQ(crossunder_condition1, crossunder_condition2);
}

TEST(CrossunderMethodTest, NotEqualOperator)
{
  auto signal_method1 = DataMethod{"close"};
  auto reference_method1 = ValueMethod{50.0};
  const auto crossunder_condition1 =
   CrossunderMethod{std::move(signal_method1), std::move(reference_method1)};

  auto signal_method2 = DataMethod{"close"};
  auto reference_method2 = ValueMethod{60.0};
  const auto crossunder_condition2 =
   CrossunderMethod{std::move(signal_method2), std::move(reference_method2)};

  EXPECT_TRUE(crossunder_condition1 != crossunder_condition2);
  EXPECT_FALSE(crossunder_condition1 == crossunder_condition2);
  EXPECT_NE(crossunder_condition1, crossunder_condition2);
}