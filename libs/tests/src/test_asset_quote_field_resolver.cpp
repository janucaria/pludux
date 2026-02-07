#include <cmath>

#include <gtest/gtest.h>

import pludux;

using namespace pludux;

TEST(AssetQuoteFieldResolverTest, DefaultConstructor)
{
  const auto field_resolver = AssetQuoteFieldResolver{};

  EXPECT_EQ(field_resolver.datetime_field(), "Datetime");
  EXPECT_EQ(field_resolver.open_field(), "Open");
  EXPECT_EQ(field_resolver.high_field(), "High");
  EXPECT_EQ(field_resolver.low_field(), "Low");
  EXPECT_EQ(field_resolver.close_field(), "Close");
  EXPECT_EQ(field_resolver.volume_field(), "Volume");
}

TEST(AssetQuoteFieldResolverTest, ParameterizedConstructor)
{
  const auto field_resolver =
   AssetQuoteFieldResolver{"Date", "O", "H", "L", "C", "V"};

  EXPECT_EQ(field_resolver.datetime_field(), "Date");
  EXPECT_EQ(field_resolver.open_field(), "O");
  EXPECT_EQ(field_resolver.high_field(), "H");
  EXPECT_EQ(field_resolver.low_field(), "L");
  EXPECT_EQ(field_resolver.close_field(), "C");
  EXPECT_EQ(field_resolver.volume_field(), "V");
}

TEST(AssetQuoteFieldResolverTest, SetterMethods)
{
  auto field_resolver = AssetQuoteFieldResolver{};

  field_resolver.datetime_field("Date");
  field_resolver.open_field("O");
  field_resolver.high_field("H");
  field_resolver.low_field("L");
  field_resolver.close_field("C");
  field_resolver.volume_field("V");

  EXPECT_EQ(field_resolver.datetime_field(), "Date");
  EXPECT_EQ(field_resolver.open_field(), "O");
  EXPECT_EQ(field_resolver.high_field(), "H");
  EXPECT_EQ(field_resolver.low_field(), "L");
  EXPECT_EQ(field_resolver.close_field(), "C");
  EXPECT_EQ(field_resolver.volume_field(), "V");
}

TEST(AssetQuoteFieldResolverTest, EqualityOperator)
{
  const auto field_resolver1 =
   AssetQuoteFieldResolver{"Date", "O", "H", "L", "C", "V"};
  const auto field_resolver2 =
   AssetQuoteFieldResolver{"Date", "O", "H", "L", "C", "V"};
  const auto field_resolver3 = AssetQuoteFieldResolver{
   "Datetime", "Open", "High", "Low", "Close", "Volume"};

  EXPECT_TRUE(field_resolver1 == field_resolver2);
  EXPECT_FALSE(field_resolver1 == field_resolver3);
  EXPECT_TRUE(field_resolver2 != field_resolver3);
  EXPECT_FALSE(field_resolver1 != field_resolver2);
}

TEST(AssetQuoteFieldResolverTest, GettersWithAssetHistory)
{
  const auto asset_history = AssetHistory{{"Date", {1, 2, 3}},
                                          {"O", {10, 20, 30}},
                                          {"H", {15, 25, 35}},
                                          {"L", {5, 15, 25}},
                                          {"C", {12, 22, 32}},
                                          {"V", {1000, 2000, 3000}}};

  const auto field_resolver =
   AssetQuoteFieldResolver{"Date", "O", "H", "L", "C", "V"};

  const auto datetimes = field_resolver.get_datetimes(asset_history);
  const auto opens = field_resolver.get_opens(asset_history);
  const auto highs = field_resolver.get_highs(asset_history);
  const auto lows = field_resolver.get_lows(asset_history);
  const auto closes = field_resolver.get_closes(asset_history);
  const auto volumes = field_resolver.get_volumes(asset_history);

  EXPECT_EQ(datetimes.size(), 3);
  EXPECT_EQ(datetimes[0], 1);
  EXPECT_EQ(datetimes[1], 2);
  EXPECT_EQ(datetimes[2], 3);

  EXPECT_EQ(opens.size(), 3);
  EXPECT_EQ(opens[0], 10);
  EXPECT_EQ(opens[1], 20);
  EXPECT_EQ(opens[2], 30);

  EXPECT_EQ(highs.size(), 3);
  EXPECT_EQ(highs[0], 15);
  EXPECT_EQ(highs[1], 25);
  EXPECT_EQ(highs[2], 35);

  EXPECT_EQ(lows.size(), 3);
  EXPECT_EQ(lows[0], 5);
  EXPECT_EQ(lows[1], 15);
  EXPECT_EQ(lows[2], 25);

  EXPECT_EQ(closes.size(), 3);
  EXPECT_EQ(closes[0], 12);
  EXPECT_EQ(closes[1], 22);
  EXPECT_EQ(closes[2], 32);

  EXPECT_EQ(volumes.size(), 3);
  EXPECT_EQ(volumes[0], 1000);
  EXPECT_EQ(volumes[1], 2000);
  EXPECT_EQ(volumes[2], 3000);
}