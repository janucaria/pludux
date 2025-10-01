#include <gtest/gtest.h>

#include <cmath>
#include <variant>

import pludux;

using namespace pludux::screener;

TEST(DataMethodTest, RunAllMethodClose)
{
  const auto data_method = DataMethod{"close"};
  const auto asset_data = pludux::AssetHistory{{"close", {4.0, 4.1, 4.2}}};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  const auto result_0 = data_method(asset_snapshot, context);
  const auto result_1 = data_method(asset_snapshot[1], context);
  const auto result_2 = data_method(asset_snapshot[2], context);
  const auto result_3 = data_method(asset_snapshot[3], context);

  EXPECT_EQ(result_0, 4.0);
  EXPECT_EQ(result_1, 4.1);
  EXPECT_EQ(result_2, 4.2);
  EXPECT_TRUE(std::isnan(result_3));
}

TEST(DataMethodTest, InvalidField)
{
  const auto data_method = DataMethod{"invalid"};
  const auto asset_data = pludux::AssetHistory{{"close", {4.0, 4.1, 4.2}}};
  const auto asset_snapshot = pludux::AssetSnapshot{asset_data};
  const auto context = std::monostate{};

  EXPECT_TRUE(std::isnan(data_method(asset_snapshot, context)));
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