#include <gtest/gtest.h>
#include <pludux/asset_data_provider.hpp>
#include <pludux/screener/data_method.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(DataMethodTest, RunAllMethodClose)
{
  const auto field = "close";
  const auto offset = 1;
  const auto data_method = DataMethod{field, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 1.0, 2.0, 3.0, 4.0, 5.0),
                                    pludux::Quote(0, 1.1, 2.1, 3.1, 4.1, 5.1),
                                    pludux::Quote(0, 1.2, 2.2, 3.2, 4.2, 5.2)});
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto series = data_method(asset_data);
  const auto closes = asset.quotes().closes();
  ASSERT_EQ(series.size(), closes.size() - offset);
  EXPECT_EQ(series[0], closes[1]);
  EXPECT_EQ(series[1], closes[2]);
}

TEST(DataMethodTest, RunAllMethodOpen)
{
  const auto field = "open";
  const auto offset = 1;
  const auto data_method = DataMethod{field, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 1.0, 2.0, 3.0, 4.0, 5.0),
                                    pludux::Quote(0, 1.1, 2.1, 3.1, 4.1, 5.1),
                                    pludux::Quote(0, 1.2, 2.2, 3.2, 4.2, 5.2)});
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto series = data_method(asset_data);
  const auto opens = asset.quotes().opens();
  ASSERT_EQ(series.size(), opens.size() - offset);
  EXPECT_EQ(series[0], opens[1]);
  EXPECT_EQ(series[1], opens[2]);
}

TEST(DataMethodTest, RunAllMethodHigh)
{
  const auto field = "high";
  const auto offset = 1;
  const auto data_method = DataMethod{field, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 1.0, 2.0, 3.0, 4.0, 5.0),
                                    pludux::Quote(0, 1.1, 2.1, 3.1, 4.1, 5.1),
                                    pludux::Quote(0, 1.2, 2.2, 3.2, 4.2, 5.2)});
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto series = data_method(asset_data);
  const auto highs = asset.quotes().highs();
  ASSERT_EQ(series.size(), highs.size() - offset);
  EXPECT_EQ(series[0], highs[1]);
  EXPECT_EQ(series[1], highs[2]);
}

TEST(DataMethodTest, RunAllMethodLow)
{
  const auto field = "low";
  const auto offset = 1;
  const auto data_method = DataMethod{field, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 1.0, 2.0, 3.0, 4.0, 5.0),
                                    pludux::Quote(0, 1.1, 2.1, 3.1, 4.1, 5.1),
                                    pludux::Quote(0, 1.2, 2.2, 3.2, 4.2, 5.2)});
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto series = data_method(asset_data);
  const auto lows = asset.quotes().lows();
  ASSERT_EQ(series.size(), lows.size() - offset);
  EXPECT_EQ(series[0], lows[1]);
  EXPECT_EQ(series[1], lows[2]);
}

TEST(DataMethodTest, RunAllMethodVolume)
{
  const auto field = "volume";
  const auto offset = 1;
  const auto data_method = DataMethod{field, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 1.0, 2.0, 3.0, 4.0, 5.0),
                                    pludux::Quote(0, 1.1, 2.1, 3.1, 4.1, 5.1),
                                    pludux::Quote(0, 1.2, 2.2, 3.2, 4.2, 5.2)});
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto series = data_method(asset_data);
  const auto volumes = asset.quotes().volumes();
  ASSERT_EQ(series.size(), volumes.size() - offset);
  EXPECT_EQ(series[0], volumes[1]);
  EXPECT_EQ(series[1], volumes[2]);
}

TEST(DataMethodTest, InvalidField)
{
  const auto field = "invalid";
  const auto offset = 1;
  const auto data_method = DataMethod{field, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 1.0, 2.0, 3.0, 4.0, 5.0),
                                    pludux::Quote(0, 1.1, 2.1, 3.1, 4.1, 5.1),
                                    pludux::Quote(0, 1.2, 2.2, 3.2, 4.2, 5.2)});
  const auto asset_data = pludux::AssetDataProvider{asset};

  EXPECT_THROW(data_method(asset_data), std::runtime_error);
}