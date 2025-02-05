#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
#include <pludux/screener/data_method.hpp>
#include <pludux/series.hpp>
#include <stdexcept>

using namespace pludux::screener;

TEST(DataMethodTest, RunAllMethodClose)
{
  const auto field = "close";
  const auto offset = 1;
  const auto data_method = DataMethod{field, offset};
  const auto asset_data = pludux::AssetHistory{{"close", {4.0, 4.1, 4.2}}};

  const auto series = data_method(asset_data);
  const auto closes = asset_data["close"];
  ASSERT_EQ(series.size(), closes.size() - offset);
  EXPECT_EQ(series[0], closes[1]);
  EXPECT_EQ(series[1], closes[2]);
}

TEST(DataMethodTest, InvalidField)
{
  const auto field = "invalid";
  const auto offset = 1;
  const auto data_method = DataMethod{field, offset};
  const auto asset_data = pludux::AssetHistory{{"close", {4.0, 4.1, 4.2}}};

  EXPECT_THROW(data_method(asset_data), std::runtime_error);
}