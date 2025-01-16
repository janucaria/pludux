#include <gtest/gtest.h>
#include <pludux/asset_data_provider.hpp>
#include <pludux/screener/atr_method.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;
using pludux::Quote;

TEST(AtrMethodTest, RunOneMethod)
{
  const auto period = 5;
  const auto atr_method = AtrMethod{period};
  const auto asset = pludux::Asset("",
                                   {Quote{0, 925, 925, 815, 875, 100},
                                    Quote{0, 875, 875, 830, 830, 100},
                                    Quote{0, 815, 840, 800, 800, 100},
                                    Quote{0, 800, 840, 800, 835, 100},
                                    Quote{0, 840, 875, 820, 870, 100},
                                    Quote{0, 870, 875, 855, 875, 100},
                                    Quote{0, 870, 880, 845, 860, 100},
                                    Quote{0, 860, 875, 855, 860, 100},
                                    Quote{0, 860, 865, 850, 860, 100},
                                    Quote{0, 860, 865, 850, 855, 100}});

  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto result = atr_method(asset_data)[0];
  EXPECT_DOUBLE_EQ(result, 32.187840000000008);
}

TEST(AtrMethodTest, RunAllMethod)
{
  const auto period = 5;
  const auto atr_method = AtrMethod{period};
  const auto asset = pludux::Asset("",
                                   {Quote{0, 925, 925, 815, 875, 100},
                                    Quote{0, 875, 875, 830, 830, 100},
                                    Quote{0, 815, 840, 800, 800, 100},
                                    Quote{0, 800, 840, 800, 835, 100},
                                    Quote{0, 840, 875, 820, 870, 100},
                                    Quote{0, 870, 875, 855, 875, 100},
                                    Quote{0, 870, 880, 845, 860, 100},
                                    Quote{0, 860, 875, 855, 860, 100},
                                    Quote{0, 860, 865, 850, 860, 100},
                                    Quote{0, 860, 865, 850, 855, 100}});
  const auto asset_data = pludux::AssetDataProvider{asset};

  const auto result = atr_method(asset_data);

  ASSERT_EQ(result.size(), asset.quotes().size());
  EXPECT_DOUBLE_EQ(result[0], 32.187840000000008);
  EXPECT_DOUBLE_EQ(result[1], 36.484800000000007);
  EXPECT_DOUBLE_EQ(result[2], 41.856000000000009);
  EXPECT_DOUBLE_EQ(result[3], 47.320000000000007);
  EXPECT_DOUBLE_EQ(result[4], 50.400000000000006);
  EXPECT_DOUBLE_EQ(result[5], 58);
  EXPECT_TRUE(std::isnan(result[6]));
  EXPECT_TRUE(std::isnan(result[7]));
  EXPECT_TRUE(std::isnan(result[8]));
  EXPECT_TRUE(std::isnan(result[9]));
}
