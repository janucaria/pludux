#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(CrossunderFilterTest, ReferenceMethod)
{
  const auto signal_value = 40.0;
  const auto reference_value = 50.0;
  auto signal_method = ValueMethod{signal_value};
  auto reference_method = ValueMethod{reference_value};
  const auto filter =
   CrossunderFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_EQ(filter.reference()(asset_data)[0], reference_value);
}

TEST(CrossunderFilterTest, SignalMethod)
{
  const auto signal_value = 40.0;
  const auto reference_value = 50.0;
  auto signal_method = ValueMethod{signal_value};
  auto reference_method = ValueMethod{reference_value};
  const auto filter =
   CrossunderFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_EQ(filter.signal()(asset_data)[0], signal_value);
}

TEST(CrossunderFilterTest, CrossunderConditionMet)
{
  auto signal_method = DataMethod{"close", 0};
  auto reference_method = ValueMethod{50.0};
  const auto crossunder_filter =
   CrossunderFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = pludux::AssetHistory{{"close", {40, 60}}};

  EXPECT_TRUE(crossunder_filter(asset_data));
}

TEST(CrossunderFilterTest, CrossunderConditionNotMet)
{
  auto signal_method = DataMethod{"close", 0};
  auto reference_method = ValueMethod{50.0};
  const auto crossunder_filter =
   CrossunderFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = pludux::AssetHistory{{"close", {50, 40}}};

  EXPECT_FALSE(crossunder_filter(asset_data));
}