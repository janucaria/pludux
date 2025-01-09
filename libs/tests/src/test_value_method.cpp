#include <gtest/gtest.h>
#include <pludux/asset_data_provider.hpp>
#include <pludux/screener/value_method.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(ValueMethodTest, ConstructorInitialization)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset = pludux::Asset{""};
  const auto asset_data = pludux::AssetDataProvider{asset};

  EXPECT_EQ(value_method.run_one(asset_data), value);
}

TEST(ValueMethodTest, RunAllMethod)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset = pludux::Asset{""};
  const auto asset_data = pludux::AssetDataProvider{asset};

  auto series = value_method.run_all(asset_data);
  ASSERT_EQ(series.size(), asset.quotes().size());
  for (std::size_t i = 0; i < series.size(); ++i) {
    EXPECT_EQ(series[i], value);
  }
}

TEST(ValueMethodTest, RunOneMethod)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset = pludux::Asset{""};
  const auto asset_data = pludux::AssetDataProvider{asset};

  EXPECT_EQ(value_method.run_one(asset_data), value);
}