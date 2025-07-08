import pludux.series;

#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/value_method.hpp>

using namespace pludux::screener;

TEST(ValueMethodTest, ConstructorInitialization)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_EQ(value_method(asset_data)[0], value);
}

TEST(ValueMethodTest, RunAllMethod)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  auto series = value_method(asset_data);
  ASSERT_EQ(series.size(), asset_data.size());
  for(std::size_t i = 0; i < series.size(); ++i) {
    EXPECT_EQ(series[i], value);
  }
}

TEST(ValueMethodTest, RunOneMethod)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_EQ(value_method(asset_data)[0], value);
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