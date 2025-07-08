import pludux.asset_history;

#include <gtest/gtest.h>
#include <limits>
#include <string>
#include <vector>

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
  EXPECT_EQ(asset_history.get_datetimes()[0], 1);
  EXPECT_EQ(asset_history.get_opens()[1], 20);
  EXPECT_EQ(asset_history.get_highs()[2], 35);
  EXPECT_EQ(asset_history.get_lows()[0], 5);
  EXPECT_EQ(asset_history.get_closes()[1], 22);
  EXPECT_EQ(asset_history.get_volumes()[2], 300);

  // Change keys and test
  asset_history.datetime_key("High");
  EXPECT_EQ(asset_history.get_datetimes()[0], 15);
  asset_history.open_key("Low");
  EXPECT_EQ(asset_history.get_opens()[1], 15);
  asset_history.high_key("Volume");
  EXPECT_EQ(asset_history.get_highs()[2], 300);
  asset_history.low_key("Datetime");
  EXPECT_EQ(asset_history.get_lows()[1], 2);
  asset_history.close_key("Open");
  EXPECT_EQ(asset_history.get_closes()[2], 30);
  asset_history.volume_key("Close");
  EXPECT_EQ(asset_history.get_volumes()[0], 12);
}

TEST(AssetHistoryTest, HistoryDataMethod)
{
  auto asset_history = AssetHistory{{"close", {1, 2, 3}}, {"open", {4, 5, 6}}};
  const auto& data = asset_history.history_data();
  EXPECT_EQ(data.size(), 2);
  EXPECT_TRUE(data.find("close") != data.end());
  EXPECT_TRUE(data.find("open") != data.end());
  EXPECT_EQ(data.at("close")[1], 2);
  EXPECT_EQ(data.at("open")[2], 6);
}

TEST(AssetHistoryTest, KeySettersAndGetters)
{
  AssetHistory ah{{"foo", {1, 2, 3}}};
  ah.datetime_key("foo");
  EXPECT_EQ(ah.datetime_key(), "foo");
  ah.open_key("foo");
  EXPECT_EQ(ah.open_key(), "foo");
  ah.high_key("foo");
  EXPECT_EQ(ah.high_key(), "foo");
  ah.low_key("foo");
  EXPECT_EQ(ah.low_key(), "foo");
  ah.close_key("foo");
  EXPECT_EQ(ah.close_key(), "foo");
  ah.volume_key("foo");
  EXPECT_EQ(ah.volume_key(), "foo");
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
  EXPECT_THROW(asset_history["open"], std::out_of_range);
}
