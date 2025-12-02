#include <cmath>

#include <gtest/gtest.h>

import pludux;

using namespace pludux;

TEST(AssetHistoryTest, ConstructorWithInitializerList)
{
  const auto asset_history = AssetHistory{{"close", {875, 830, 800, 835, 870}},
                                          {"open", {870, 825, 795}}};

  EXPECT_EQ(asset_history.size(), 5);
  EXPECT_TRUE(asset_history.contains("close"));
  EXPECT_TRUE(asset_history.contains("open"));

  const auto close_series = asset_history["close"];
  EXPECT_EQ(close_series[0], 875);
  EXPECT_EQ(close_series[1], 830);
  EXPECT_EQ(close_series[2], 800);
  EXPECT_EQ(close_series[3], 835);
  EXPECT_EQ(close_series[4], 870);
  EXPECT_TRUE(std::isnan(close_series[5]));

  const auto open_series = asset_history["open"];
  EXPECT_EQ(open_series[0], 870);
  EXPECT_EQ(open_series[1], 825);
  EXPECT_EQ(open_series[2], 795);
  EXPECT_TRUE(std::isnan(open_series[3]));
  EXPECT_TRUE(std::isnan(open_series[4]));
}

TEST(AssetHistoryTest, OperatorSquareBracketsWithKey)
{
  const auto asset_history = AssetHistory{{"close", {875, 830, 800, 835, 870}}};

  const auto close_series = asset_history["close"];
  EXPECT_EQ(close_series[0], 875);
  EXPECT_EQ(close_series[1], 830);
  EXPECT_EQ(close_series[2], 800);
  EXPECT_EQ(close_series[3], 835);
  EXPECT_EQ(close_series[4], 870);
}

TEST(AssetHistoryTest, GetterMethods)
{
  auto asset_history = AssetHistory{{"Datetime", {1, 2, 3}},
                                    {"Open", {10, 20, 30}},
                                    {"High", {15, 25, 35}},
                                    {"Low", {5, 15, 25}},
                                    {"Close", {12, 22, 32}},
                                    {"Volume", {100, 200, 300}}};

  // Default keys
  EXPECT_EQ(asset_history.datetime_series()[0], 1);
  EXPECT_EQ(asset_history.open_series()[1], 20);
  EXPECT_EQ(asset_history.high_series()[2], 35);
  EXPECT_EQ(asset_history.low_series()[0], 5);
  EXPECT_EQ(asset_history.close_series()[1], 22);
  EXPECT_EQ(asset_history.volume_series()[2], 300);

  // Change keys and test
  asset_history.datetime_field("High");
  EXPECT_EQ(asset_history.datetime_series()[0], 15);
  asset_history.open_field("Low");
  EXPECT_EQ(asset_history.open_series()[1], 15);
  asset_history.high_field("Volume");
  EXPECT_EQ(asset_history.high_series()[2], 300);
  asset_history.low_field("Datetime");
  EXPECT_EQ(asset_history.low_series()[1], 2);
  asset_history.close_field("Open");
  EXPECT_EQ(asset_history.close_series()[2], 30);
  asset_history.volume_field("Close");
  EXPECT_EQ(asset_history.volume_series()[0], 12);
}

TEST(AssetHistoryTest, HistoryDataMethod)
{
  auto asset_history = AssetHistory{{"close", {1, 2, 3}}, {"open", {4, 5, 6}}};
  const auto& data = asset_history.field_data();
  EXPECT_EQ(data.size(), 2);
  EXPECT_TRUE(data.find("close") != data.end());
  EXPECT_TRUE(data.find("open") != data.end());
  EXPECT_EQ(data.at("close")[1], 2);
  EXPECT_EQ(data.at("open")[2], 6);
}

TEST(AssetHistoryTest, KeySettersAndGetters)
{
  AssetHistory ah{{"foo", {1, 2, 3}}};
  ah.datetime_field("foo");
  EXPECT_EQ(ah.datetime_field(), "foo");
  ah.open_field("foo");
  EXPECT_EQ(ah.open_field(), "foo");
  ah.high_field("foo");
  EXPECT_EQ(ah.high_field(), "foo");
  ah.low_field("foo");
  EXPECT_EQ(ah.low_field(), "foo");
  ah.close_field("foo");
  EXPECT_EQ(ah.close_field(), "foo");
  ah.volume_field("foo");
  EXPECT_EQ(ah.volume_field(), "foo");
}

TEST(AssetHistoryTest, InsertAfterConstruction)
{
  AssetHistory ah{{"close", {1, 2}}};
  ah.insert("open", {3, 4});
  EXPECT_TRUE(ah.contains("open"));
  EXPECT_EQ(ah.size(), 2);
  EXPECT_EQ(ah["open"][1], 4);
}

TEST(AssetHistoryTest, SizeMethod)
{
  const auto asset_history = AssetHistory{{"close", {875, 830, 800, 835, 870}}};
  EXPECT_EQ(asset_history.size(), 5);
}

TEST(AssetHistoryTest, ContainsMethod)
{
  const auto asset_history = AssetHistory{{"close", {875, 830, 800, 835, 870}}};
  EXPECT_TRUE(asset_history.contains("close"));
  EXPECT_FALSE(asset_history.contains("open"));
}

TEST(AssetHistoryTest, InsertMethod)
{
  auto asset_history = AssetHistory{{"close", {875, 830, 800, 835, 870}}};
  asset_history.insert("open", {870, 825, 795, 800, 850});

  EXPECT_TRUE(asset_history.contains("open"));
  const auto open_series = asset_history["open"];
  EXPECT_EQ(open_series[0], 870);
  EXPECT_EQ(open_series[1], 825);
  EXPECT_EQ(open_series[2], 795);
  EXPECT_EQ(open_series[3], 800);
  EXPECT_EQ(open_series[4], 850);
}

TEST(AssetHistoryTest, EmptyAssetHistory)
{
  const auto asset_history = AssetHistory{};

  EXPECT_EQ(asset_history.size(), 0);
  EXPECT_FALSE(asset_history.contains("close"));
}

TEST(AssetHistoryTest, InsertEmptySeries)
{
  auto asset_history = AssetHistory{{"close", {}}};
  EXPECT_EQ(asset_history.size(), 0);
  EXPECT_TRUE(asset_history.contains("close"));
}

TEST(AssetHistoryTest, InsertMismatchedSeriesSize)
{
  auto asset_history = AssetHistory{{"close", {875, 830, 800, 835, 870}}};
  asset_history.insert("open", {870, 825});

  EXPECT_EQ(asset_history.size(), 5);
  EXPECT_TRUE(asset_history.contains("close"));
  EXPECT_TRUE(asset_history.contains("open"));

  const auto open_series = asset_history["open"];
  EXPECT_EQ(open_series[0], 870);
  EXPECT_EQ(open_series[1], 825);
  EXPECT_TRUE(std::isnan(open_series[2]));
  EXPECT_TRUE(std::isnan(open_series[3]));
  EXPECT_TRUE(std::isnan(open_series[4]));
}

TEST(AssetHistoryTest, AccessNonExistentKey)
{
  const auto asset_history = AssetHistory{{"close", {875, 830, 800, 835, 870}}};
  EXPECT_FALSE(asset_history.contains("open"));

  const auto open_series = asset_history["open"];
  EXPECT_EQ(open_series.size(), 0);
  EXPECT_TRUE(std::isnan(open_series[0]));
  EXPECT_TRUE(std::isnan(open_series[1]));
  EXPECT_TRUE(std::isnan(open_series[2]));
}
