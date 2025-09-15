#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/ohlcv_method.hpp>
#include <pludux/screener/reference_method.hpp>
#include <pludux/series.hpp>
#include <stdexcept>

using namespace pludux;
using namespace pludux::screener;

TEST(ReferenceMethodTest, RunAllMethodClose)
{
  const auto open_method = OpenMethod{};
  const auto close_method = CloseMethod{};
  const auto asset_data =
   AssetHistory{{"Open", {4.0, 4.1, 4.2}}, {"Close", {1.0, 1.1, 1.2}}};

  auto registry = std::make_shared<MethodRegistry>();
  registry->set("open", open_method);
  registry->set("close", close_method);

  const auto open_ref_method = ReferenceMethod{registry, "open"};
  const auto open_series = open_ref_method(asset_data);
  ASSERT_EQ(open_series.size(), 3);
  EXPECT_EQ(open_series[0], 4.0);
  EXPECT_EQ(open_series[1], 4.1);
  EXPECT_EQ(open_series[2], 4.2);

  const auto close_ref_method = ReferenceMethod{registry, "close", 1};
  const auto close_series = close_ref_method(asset_data);
  ASSERT_EQ(close_series.size(), 2);
  EXPECT_EQ(close_series[0], 1.1);
  EXPECT_EQ(close_series[1], 1.2);
  EXPECT_TRUE(std::isnan(close_series[2]));
}

TEST(ReferenceMethodTest, InvalidField)
{
  const auto close_method = CloseMethod{};
  const auto asset_data = AssetHistory{{"Close", {4.0, 4.1, 4.2}}};

  auto registry = std::make_shared<MethodRegistry>();
  registry->set("close", close_method);

  const auto not_found_ref_method = ReferenceMethod{registry, "invalid"};
  const auto not_found_series = not_found_ref_method(asset_data);

  EXPECT_TRUE(std::isnan(not_found_series[0]));
  EXPECT_TRUE(std::isnan(not_found_series[1]));
}

TEST(ReferenceMethodTest, EqualityOperator)
{
  auto registry = std::make_shared<MethodRegistry>();

  const auto ref_method1 = ReferenceMethod{registry, "close"};
  const auto ref_method2 = ReferenceMethod{registry, "close"};

  EXPECT_EQ(ref_method1, ref_method2);
}

TEST(ReferenceMethodTest, NotEqualOperator)
{
  auto registry1 = std::make_shared<MethodRegistry>();
  auto registry2 = std::make_shared<MethodRegistry>();

  const auto ref_method1 = ReferenceMethod{registry1, "close"};
  const auto ref_method2 = ReferenceMethod{registry2, "open"};
  const auto ref_method3 = ReferenceMethod{registry2, "close"};

  EXPECT_NE(ref_method1, ref_method2);
  EXPECT_NE(ref_method1, ref_method3);
  EXPECT_NE(ref_method2, ref_method3);
}