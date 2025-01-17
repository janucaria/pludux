#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/value_method.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(ValueMethodTest, ConstructorInitialization)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_EQ(value_method(asset_data)[0], value);
}

TEST(ValueMethodTest, RunAllMethod)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  auto series = value_method(asset_data);
  ASSERT_EQ(series.size(), asset_data.size());
  for(std::size_t i = 0; i < series.size(); ++i) {
    EXPECT_EQ(series[i], value);
  }
}

TEST(ValueMethodTest, RunOneMethod)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_EQ(value_method(asset_data)[0], value);
}