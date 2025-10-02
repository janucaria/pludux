#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux;

const auto context = std::monostate{};

TEST(ValueMethodTest, ConstructorInitialization)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset_data = AssetHistory{{"close", {0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  EXPECT_EQ(value_method(asset_snapshot[0], context), value);
  EXPECT_EQ(value_method(asset_snapshot[1], context), value);
  EXPECT_EQ(value_method(asset_snapshot[2], context), value);
  EXPECT_EQ(value_method(asset_snapshot[3], context), value);
  EXPECT_EQ(value_method(asset_snapshot[4], context), value);
}

TEST(ValueMethodTest, RunAllMethod)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset_data = AssetHistory{{"close", {0}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  for(std::size_t i = 0; i < asset_snapshot.size(); ++i) {
    auto result = value_method(asset_snapshot[i], context);
    EXPECT_EQ(result, value);
  }
}

TEST(ValueMethodTest, RunOneMethod)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset_data = AssetHistory{{"close", {0}}};

  EXPECT_EQ((value_method(asset_data, context)), value);
}

TEST(ValueMethodTest, EqualityOperator)
{
  const auto value1 = 42.0;
  const auto value2 = 42.0;
  const auto value3 = 24.0;

  const auto value_method1 = ValueMethod{value1};
  const auto value_method2 = ValueMethod{value2};
  const auto value_method3 = ValueMethod{value3};

  EXPECT_TRUE(value_method1 == value_method2);
  EXPECT_FALSE(value_method1 == value_method3);
  EXPECT_EQ(value_method1, value_method2);
}

TEST(ValueMethodTest, NotEqualOperator)
{
  const auto value1 = 42.0;
  const auto value2 = 42.0;
  const auto value3 = 24.0;

  const auto value_method1 = ValueMethod{value1};
  const auto value_method2 = ValueMethod{value2};
  const auto value_method3 = ValueMethod{value3};

  EXPECT_FALSE(value_method1 != value_method2);
  EXPECT_TRUE(value_method1 != value_method3);
  EXPECT_NE(value_method1, value_method3);
}