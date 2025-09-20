#include <gtest/gtest.h>

import pludux;

using namespace pludux::screener;

TEST(DataMethodTest, RunAllMethodClose)
{
  const auto data_method = DataMethod{"close"};
  const auto asset_data = pludux::AssetHistory{{"close", {4.0, 4.1, 4.2}}};

  const auto series = data_method(asset_data);
  ASSERT_EQ(series.size(), 3);
  EXPECT_EQ(series[0], 4.0);
  EXPECT_EQ(series[1], 4.1);
  EXPECT_EQ(series[2], 4.2);
}

TEST(DataMethodTest, InvalidField)
{
  const auto data_method = DataMethod{"invalid"};
  const auto asset_data = pludux::AssetHistory{{"close", {4.0, 4.1, 4.2}}};

  EXPECT_TRUE(std::isnan(data_method(asset_data)[0]));
  EXPECT_TRUE(std::isnan(data_method(asset_data)[1]));
}

TEST(DataMethodTest, EqualityOperator)
{
  const auto data_method1 = DataMethod{"close"};
  const auto data_method2 = DataMethod{"close"};

  EXPECT_TRUE(data_method1 == data_method2);
  EXPECT_FALSE(data_method1 != data_method2);
  EXPECT_EQ(data_method1, data_method2);
}

TEST(DataMethodTest, NotEqualOperator)
{
  const auto data_method1 = DataMethod{"close"};
  const auto data_method2 = DataMethod{"open"};

  EXPECT_TRUE(data_method1 != data_method2);
  EXPECT_FALSE(data_method1 == data_method2);
  EXPECT_NE(data_method1, data_method2);
}