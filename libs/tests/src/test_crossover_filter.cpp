#include <gtest/gtest.h>
#include <pludux/asset_data_provider.hpp>
#include <pludux/screener.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(CrossoverFilterTest, ReferenceMethod)
{
  const auto signal_value = 40.0;
  const auto reference_value = 50.0;
  auto signal_method = ValueMethod{signal_value};
  auto reference_method = ValueMethod{reference_value};
  const auto filter =
   CrossoverFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset = pludux::Asset{"", std::vector<pludux::Quote>(1)};
  const auto asset_data = pludux::AssetDataProvider{asset};

  EXPECT_EQ(filter.reference()(asset_data)[0], reference_value);
}

TEST(CrossoverFilterTest, SignalMethod)
{
  const auto signal_value = 40.0;
  const auto reference_value = 50.0;
  auto signal_method = ValueMethod{signal_value};
  auto reference_method = ValueMethod{reference_value};
  const auto filter =
   CrossoverFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset = pludux::Asset{"", std::vector<pludux::Quote>(1)};
  const auto asset_data = pludux::AssetDataProvider{asset};

  EXPECT_EQ(filter.signal()(asset_data)[0], signal_value);
}

TEST(CrossoverFilterTest, CrossoverConditionMet)
{
  auto signal_method = DataMethod{"close", 0};
  auto reference_method = ValueMethod{50.0};
  const auto filter =
   CrossoverFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset = pludux::Asset{
   "", std::vector<pludux::Quote>{{0, 0, 0, 0, 50, 0}, {0, 0, 0, 0, 60, 0}}};
  const auto asset_data = pludux::AssetDataProvider{asset};

  EXPECT_TRUE(filter(asset_data));
}

TEST(CrossoverFilterTest, CrossoverConditionNotMet)
{
  auto signal_method = DataMethod{"close", 0};
  auto reference_method = ValueMethod{50.0};
  const auto filter =
   CrossoverFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset = pludux::Asset{
   "", std::vector<pludux::Quote>{{0, 0, 0, 0, 40, 0}, {0, 0, 0, 0, 50, 0}}};
  const auto asset_data = pludux::AssetDataProvider{asset};

  EXPECT_FALSE(filter(asset_data));
}