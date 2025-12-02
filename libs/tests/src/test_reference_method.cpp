#include <gtest/gtest.h>

#include <cmath>

import pludux;

using namespace pludux;

TEST(ReferenceMethodTest, RunAllMethodClose)
{
  const auto open_method = OpenMethod{};
  const auto close_method = CloseMethod{};
  const auto asset_data =
   AssetHistory{{"Open", {4.0, 4.1, 4.2}}, {"Close", {1.0, 1.1, 1.2}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  auto registry = SeriesMethodRegistry{};
  registry.set("open", open_method);
  registry.set("close", close_method);

  auto context = DefaultMethodContext{registry};

  const auto open_ref_method = ReferenceMethod{"open"};
  EXPECT_EQ(open_ref_method(asset_snapshot[0], context), 4.0);
  EXPECT_EQ(open_ref_method(asset_snapshot[1], context), 4.1);
  EXPECT_EQ(open_ref_method(asset_snapshot[2], context), 4.2);

  const auto close_ref_method = ReferenceMethod{"close"};
  EXPECT_EQ(close_ref_method(asset_snapshot[0], context), 1.0);
  EXPECT_EQ(close_ref_method(asset_snapshot[1], context), 1.1);
  EXPECT_EQ(close_ref_method(asset_snapshot[2], context), 1.2);
}

TEST(ReferenceMethodTest, InvalidField)
{
  const auto close_method = CloseMethod{};
  const auto asset_data = AssetHistory{{"Close", {4.0, 4.1, 4.2}}};
  const auto asset_snapshot = AssetSnapshot{asset_data};

  auto registry = SeriesMethodRegistry{};
  registry.set("close", close_method);

  auto context = DefaultMethodContext{registry};

  const auto not_found_ref_method = ReferenceMethod{"invalid"};

  EXPECT_TRUE(std::isnan(not_found_ref_method(asset_snapshot[0], context)));
  EXPECT_TRUE(std::isnan(not_found_ref_method(asset_snapshot[1], context)));
}

TEST(ReferenceMethodTest, EqualityOperator)
{
  const auto ref_method1 = ReferenceMethod{"close"};
  const auto ref_method2 = ReferenceMethod{"close"};

  EXPECT_EQ(ref_method1, ref_method2);
}

TEST(ReferenceMethodTest, NotEqualOperator)
{
  const auto ref_method1 = ReferenceMethod{"close"};
  const auto ref_method2 = ReferenceMethod{"open"};

  EXPECT_TRUE(ref_method1 != ref_method2);
  EXPECT_NE(ref_method1, ref_method2);
}