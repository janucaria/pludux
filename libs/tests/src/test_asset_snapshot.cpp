#include <cmath>

#include <gtest/gtest.h>

import pludux;

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

  // Offset snapshot
  auto snap2 = snap[2];
  EXPECT_EQ(snap2.offset(), 2);
  EXPECT_EQ(snap2.size(), 3);
  EXPECT_EQ(snap2["close"], 30);
}

TEST(AssetSnapshotTest, GettersForOHLCV)
{
  AssetHistory ah{{"Datetime", {1, 2, 3}},
                  {"Open", {10, 20, 30}},
                  {"High", {15, 25, 35}},
                  {"Low", {5, 15, 25}},
                  {"Close", {12, 22, 32}},
                  {"Volume", {100, 200, 300}}};

  auto snap = AssetSnapshot{ah};
  EXPECT_EQ(snap.datetime(), 1);
  EXPECT_EQ(snap.open(), 10);
  EXPECT_EQ(snap.high(), 15);
  EXPECT_EQ(snap.low(), 5);
  EXPECT_EQ(snap.close(), 12);
  EXPECT_EQ(snap.volume(), 100);

  auto snap2 = snap[1];
  EXPECT_EQ(snap2.datetime(), 2);
  EXPECT_EQ(snap2.open(), 20);
  EXPECT_EQ(snap2.high(), 25);
  EXPECT_EQ(snap2.low(), 15);
  EXPECT_EQ(snap2.close(), 22);
  EXPECT_EQ(snap2.volume(), 200);

  auto snap3 = snap[2];
  EXPECT_EQ(snap3.datetime(), 3);
  EXPECT_EQ(snap3.open(), 30);
  EXPECT_EQ(snap3.high(), 35);
  EXPECT_EQ(snap3.low(), 25);
  EXPECT_EQ(snap3.close(), 32);
  EXPECT_EQ(snap3.volume(), 300);
}

TEST(AssetSnapshotTest, OutOfBoundsAccess)
{
  AssetHistory ah{{"close", {10, 20}}};
  AssetSnapshot snap{ah};
  auto open = snap["open"];
  EXPECT_TRUE(std::isnan(open));
}
