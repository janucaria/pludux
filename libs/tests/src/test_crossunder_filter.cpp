#include <gtest/gtest.h>
#include <pludux/asset_data_provider.hpp>
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
  const auto asset = pludux::Asset{""};
  const auto asset_data = pludux::AssetDataProvider{asset};

  EXPECT_EQ(filter.reference().run_one(asset_data), reference_value);
}

TEST(CrossunderFilterTest, SignalMethod)
{
  const auto signal_value = 40.0;
  const auto reference_value = 50.0;
  auto signal_method = ValueMethod{signal_value};
  auto reference_method = ValueMethod{reference_value};
  const auto filter =
   CrossunderFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset = pludux::Asset{""};
  const auto asset_data = pludux::AssetDataProvider{asset};

  EXPECT_EQ(filter.signal().run_one(asset_data), signal_value);
}

TEST(CrossunderFilterTest, CrossunderConditionMet)
{
  auto signal_method = DataMethod{"close", 0};
  auto reference_method = ValueMethod{50.0};
  const auto filter =
   CrossunderFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset = pludux::Asset{
   "", std::vector<pludux::Quote>{{0, 0, 0, 0, 60, 0}, {0, 0, 0, 0, 40, 0}}};
  const auto asset_data = pludux::AssetDataProvider{asset};

  EXPECT_TRUE(filter(asset_data));
}

TEST(CrossunderFilterTest, CrossunderConditionNotMet)
{
  auto signal_method = DataMethod{"close", 0};
  auto reference_method = ValueMethod{50.0};
  const auto filter =
   CrossunderFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset = pludux::Asset{
   "", std::vector<pludux::Quote>{{0, 0, 0, 0, 40, 0}, {0, 0, 0, 0, 50, 0}}};
  const auto asset_data = pludux::AssetDataProvider{asset};

  EXPECT_FALSE(filter(asset_data));
}