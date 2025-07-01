#include <cmath>
#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/asset_snapshot.hpp>
#include <string>
#include <vector>

using namespace pludux;

TEST(AssetSnapshotTest, BasicAccessAndOffset)
{
  AssetHistory ah{{"close", {10, 20, 30, 40, 50}},
                  {"open", {11, 21, 31, 41, 51}},
                  {"Datetime", {1, 2, 3, 4, 5}}};

  AssetSnapshot snap{ah};
  EXPECT_EQ(snap.size(), 5);
  EXPECT_EQ(snap.offset(), 0);
  EXPECT_TRUE(snap.contains("close"));
  EXPECT_FALSE(snap.contains("foo"));

  // operator[] by key
  EXPECT_EQ(snap["close"], 10);
  EXPECT_EQ(snap["open"], 11);
  EXPECT_EQ(snap["Datetime"], 1);

  // get_values
  auto closes = snap.get_values("close");
  EXPECT_EQ(closes[0], 10);
  EXPECT_EQ(closes[1], 20);
  EXPECT_EQ(closes[4], 50);
  EXPECT_TRUE(std::isnan(closes[5]));

  // get_datetime_values
  auto datetimes = snap.get_datetime_values();
  EXPECT_EQ(datetimes[0], 1);
  EXPECT_EQ(datetimes[4], 5);

  // Offset snapshot
  auto snap2 = snap[2];
  EXPECT_EQ(snap2.offset(), 2);
  EXPECT_EQ(snap2.size(), 3);
  EXPECT_EQ(snap2["close"], 30);
  EXPECT_EQ(snap2.get_values("close")[0], 30);
  EXPECT_EQ(snap2.get_values("close")[2], 50);
  EXPECT_TRUE(std::isnan(snap2.get_values("close")[3]));
}

TEST(AssetSnapshotTest, GettersForOHLCV)
{
  AssetHistory ah{{"Datetime", {1, 2, 3}},
                  {"Open", {10, 20, 30}},
                  {"High", {15, 25, 35}},
                  {"Low", {5, 15, 25}},
                  {"Close", {12, 22, 32}},
                  {"Volume", {100, 200, 300}}};
  AssetSnapshot snap{ah};
  EXPECT_EQ(snap.get_open_values()[1], 20);
  EXPECT_EQ(snap.get_high_values()[2], 35);
  EXPECT_EQ(snap.get_low_values()[0], 5);
  EXPECT_EQ(snap.get_close_values()[2], 32);
  EXPECT_EQ(snap.get_volume_values()[0], 100);
}

TEST(AssetSnapshotTest, OutOfBoundsAccess)
{
  AssetHistory ah{{"close", {10, 20}}};
  AssetSnapshot snap{ah};
  auto closes = snap.get_values("close");
  EXPECT_TRUE(std::isnan(closes[2]));
  EXPECT_TRUE(std::isnan(closes[100]));
  EXPECT_TRUE(std::isnan(snap[100]["close"]));
}
