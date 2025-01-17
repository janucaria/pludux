#include <gtest/gtest.h>
#include <limits>
#include <pludux/asset_history.hpp>
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

TEST(AssetHistoryTest, OperatorSquareBracketsWithIndex)
{
  const auto asset_history = AssetHistory{{"close", {875, 830, 800, 835, 870}}};

  const auto snapshot = asset_history[1];

  EXPECT_EQ(snapshot.offset(), 1);
  EXPECT_EQ(snapshot["close"][1], 800);
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

TEST(AssetHistoryTest, IteratorMethods)
{
  const auto asset_history = AssetHistory{{"close", {875, 830, 800, 835, 870}}};

  auto it = asset_history.begin();
  EXPECT_EQ((*it)["close"][0], 875);
  ++it;
  EXPECT_EQ((*it)["close"][0], 830);
  --it;
  EXPECT_EQ((*it)["close"][0], 875);

  auto rit = asset_history.rbegin();
  EXPECT_EQ((*rit)["close"][0], 870);
  ++rit;
  EXPECT_EQ((*rit)["close"][0], 835);
  --rit;
  EXPECT_EQ((*rit)["close"][0], 870);
}