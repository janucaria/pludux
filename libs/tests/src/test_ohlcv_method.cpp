#include <cmath>
#include <gtest/gtest.h>

import pludux;

using namespace pludux;

const auto context = std::monostate{};

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

  {
    auto open_val = open(snap, context);
    auto high_val = high(snap, context);
    auto low_val = low(snap, context);
    auto close_val = close(snap, context);
    auto volume_val = volume(snap, context);

    EXPECT_EQ(open_val, 10);
    EXPECT_EQ(high_val, 15);
    EXPECT_EQ(low_val, 5);
    EXPECT_EQ(close_val, 12);
    EXPECT_EQ(volume_val, 100);
  }

  {
    auto open_val = open(snap[1], context);
    auto high_val = high(snap[1], context);
    auto low_val = low(snap[1], context);
    auto close_val = close(snap[1], context);
    auto volume_val = volume(snap[1], context);

    EXPECT_EQ(open_val, 20);
    EXPECT_EQ(high_val, 25);
    EXPECT_EQ(low_val, 15);
    EXPECT_EQ(close_val, 22);
    EXPECT_EQ(volume_val, 200);
  }

  {
    auto open_val = open(snap[2], context);
    auto high_val = high(snap[2], context);
    auto low_val = low(snap[2], context);
    auto close_val = close(snap[2], context);
    auto volume_val = volume(snap[2], context);

    EXPECT_EQ(open_val, 30);
    EXPECT_EQ(high_val, 35);
    EXPECT_EQ(low_val, 25);
    EXPECT_EQ(close_val, 32);
    EXPECT_EQ(volume_val, 300);
  }

  {
    auto open_val = open(snap[3], context);
    auto high_val = high(snap[3], context);
    auto low_val = low(snap[3], context);
    auto close_val = close(snap[3], context);
    auto volume_val = volume(snap[3], context);

    EXPECT_TRUE(std::isnan(open_val));
    EXPECT_TRUE(std::isnan(high_val));
    EXPECT_TRUE(std::isnan(low_val));
    EXPECT_TRUE(std::isnan(close_val));
    EXPECT_TRUE(std::isnan(volume_val));
  }
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
  auto open_val = open(snap, context);
  EXPECT_TRUE(std::isnan(open_val));
}

TEST(OhlcvMethodTest, InsertAfterConstruction)
{
  AssetHistory ah{{"Open", {10, 20}}};
  ah.insert("High", {15, 25});
  auto snap = AssetSnapshot{ah};
  auto high = HighMethod{};
  auto high_val_0 = high(snap, context);
  auto high_val_1 = high(snap[1], context);
  auto high_val_2 = high(snap[2], context);
  EXPECT_EQ(high_val_0, 15);
  EXPECT_EQ(high_val_1, 25);
  EXPECT_TRUE(std::isnan(high_val_2));
}

TEST(OhlcvMethodTest, ChangeKeyMapping)
{
  AssetHistory ah{{"foo", {1, 2, 3}}};
  ah.open_field("foo");
  auto snap = AssetSnapshot{ah};
  auto open = OpenMethod{};
  auto open_val_0 = open(snap, context);
  auto open_val_2 = open(snap[2], context);
  auto open_val_3 = open(snap[3], context);
  EXPECT_EQ(open_val_0, 1);
  EXPECT_EQ(open_val_2, 3);
  EXPECT_TRUE(std::isnan(open_val_3));
}
