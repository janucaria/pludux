import pludux.asset_history;
import pludux.asset_snapshot;
import pludux.screener;
import pludux.series;

#include <gtest/gtest.h>
#include <stdexcept>

using namespace pludux::screener;

TEST(DataMethodTest, RunAllMethodClose)
{
  const auto field = "close";
  const auto offset = 1;
  const auto data_method = DataMethod{field, offset};
  const auto asset_data = pludux::AssetHistory{{"close", {4.0, 4.1, 4.2}}};

  const auto series = data_method(asset_data);
  const auto closes = asset_data["close"];
  ASSERT_EQ(series.size(), closes.size() - offset);
  EXPECT_EQ(series[0], closes[1]);
  EXPECT_EQ(series[1], closes[2]);
}

TEST(DataMethodTest, InvalidField)
{
  const auto field = "invalid";
  const auto offset = 1;
  const auto data_method = DataMethod{field, offset};
  const auto asset_data = pludux::AssetHistory{{"close", {4.0, 4.1, 4.2}}};

  EXPECT_TRUE(std::isnan(data_method(asset_data)[0]));
  EXPECT_TRUE(std::isnan(data_method(asset_data)[1]));
}

TEST(DataMethodTest, EqualityOperator)
{
  const auto field1 = "close";
  const auto offset1 = 0;
  const auto data_method1 = DataMethod{field1, offset1};

  const auto field2 = "close";
  const auto offset2 = 0;
  const auto data_method2 = DataMethod{field2, offset2};

  EXPECT_TRUE(data_method1 == data_method2);
  EXPECT_FALSE(data_method1 != data_method2);
  EXPECT_EQ(data_method1, data_method2);
}

TEST(DataMethodTest, NotEqualOperator)
{
  const auto field1 = "close";
  const auto offset1 = 0;
  const auto data_method1 = DataMethod{field1, offset1};

  const auto field2 = "open";
  const auto offset2 = 0;
  const auto data_method2 = DataMethod{field2, offset2};

  EXPECT_TRUE(data_method1 != data_method2);
  EXPECT_FALSE(data_method1 == data_method2);
  EXPECT_NE(data_method1, data_method2);
}