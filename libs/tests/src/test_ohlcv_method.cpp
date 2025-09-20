#include <cmath>
#include <gtest/gtest.h>

import pludux;

using namespace pludux;
using namespace pludux::screener;

TEST(OhlcvMethodTest, OpenHighLowCloseVolumeMethods)
{
  auto ah = AssetHistory{{"Open", {10, 20, 30}},
                         {"High", {15, 25, 35}},
                         {"Low", {5, 15, 25}},
                         {"Close", {12, 22, 32}},
                         {"Volume", {100, 200, 300}}};
  auto snap = AssetSnapshot{ah};

  auto open = OpenMethod{};
  auto high = HighMethod{};
  auto low = LowMethod{};
  auto close = CloseMethod{};
  auto volume = VolumeMethod{};

  auto open_vals = open(snap);
  auto high_vals = high(snap);
  auto low_vals = low(snap);
  auto close_vals = close(snap);
  auto volume_vals = volume(snap);

  EXPECT_EQ(open_vals[0], 10);
  EXPECT_EQ(open_vals[2], 30);
  EXPECT_TRUE(std::isnan(open_vals[3]));

  EXPECT_EQ(high_vals[0], 15);
  EXPECT_EQ(high_vals[1], 25);
  EXPECT_EQ(high_vals[2], 35);
  EXPECT_TRUE(std::isnan(high_vals[3]));

  EXPECT_EQ(low_vals[0], 5);
  EXPECT_EQ(low_vals[1], 15);
  EXPECT_EQ(low_vals[2], 25);
  EXPECT_TRUE(std::isnan(low_vals[3]));

  EXPECT_EQ(close_vals[0], 12);
  EXPECT_EQ(close_vals[1], 22);
  EXPECT_EQ(close_vals[2], 32);
  EXPECT_TRUE(std::isnan(close_vals[3]));

  EXPECT_EQ(volume_vals[0], 100);
  EXPECT_EQ(volume_vals[1], 200);
  EXPECT_EQ(volume_vals[2], 300);
  EXPECT_TRUE(std::isnan(volume_vals[3]));
}

TEST(OhlcvMethodTest, Equality)
{
  auto open1 = OpenMethod{};
  auto open2 = OpenMethod{};

  EXPECT_EQ(open1, open2);
}

TEST(OhlcvMethodTest, EmptySeries)
{
  auto ah = AssetHistory{{"Open", {}}};
  auto snap = AssetSnapshot{ah};
  auto open = OpenMethod{};
  auto open_vals = open(snap);
  EXPECT_TRUE(std::isnan(open_vals[0]));
  EXPECT_TRUE(std::isnan(open_vals[10]));
}

TEST(OhlcvMethodTest, InsertAfterConstruction)
{
  AssetHistory ah{{"Open", {10, 20}}};
  ah.insert("High", {15, 25});
  auto snap = AssetSnapshot{ah};
  auto high = HighMethod{};
  auto high_vals = high(snap);
  EXPECT_EQ(high_vals[0], 15);
  EXPECT_EQ(high_vals[1], 25);
  EXPECT_TRUE(std::isnan(high_vals[2]));
}

TEST(OhlcvMethodTest, ChangeKeyMapping)
{
  AssetHistory ah{{"foo", {1, 2, 3}}};
  ah.open_key("foo");
  auto snap = AssetSnapshot{ah};
  auto open = OpenMethod{};
  auto open_vals = open(snap);
  EXPECT_EQ(open_vals[0], 1);
  EXPECT_EQ(open_vals[2], 3);
  EXPECT_TRUE(std::isnan(open_vals[3]));
}
