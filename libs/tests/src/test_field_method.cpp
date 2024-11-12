#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/screener/field_method.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(FieldMethodTest, RunAllMethodClose)
{
  const auto field = "close";
  const auto offset = 1;
  const auto field_method = FieldMethod{field, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 1.0, 2.0, 3.0, 4.0, 5.0),
                                    pludux::Quote(0, 1.1, 2.1, 3.1, 4.1, 5.1),
                                    pludux::Quote(0, 1.2, 2.2, 3.2, 4.2, 5.2)});

  auto series = field_method.run_all(asset);
  ASSERT_EQ(series.size(), asset.quotes().size() - offset);
  EXPECT_EQ(series[0], asset.quotes()[1].close());
  EXPECT_EQ(series[1], asset.quotes()[0].close());
}

TEST(FieldMethodTest, RunAllMethodOpen)
{
  const auto field = "open";
  const auto offset = 1;
  const auto field_method = FieldMethod{field, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 1.0, 2.0, 3.0, 4.0, 5.0),
                                    pludux::Quote(0, 1.1, 2.1, 3.1, 4.1, 5.1),
                                    pludux::Quote(0, 1.2, 2.2, 3.2, 4.2, 5.2)});

  auto series = field_method.run_all(asset);
  ASSERT_EQ(series.size(), asset.quotes().size() - offset);
  EXPECT_EQ(series[0], asset.quotes()[1].open());
  EXPECT_EQ(series[1], asset.quotes()[0].open());
}

TEST(FieldMethodTest, RunAllMethodHigh)
{
  const auto field = "high";
  const auto offset = 1;
  const auto field_method = FieldMethod{field, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 1.0, 2.0, 3.0, 4.0, 5.0),
                                    pludux::Quote(0, 1.1, 2.1, 3.1, 4.1, 5.1),
                                    pludux::Quote(0, 1.2, 2.2, 3.2, 4.2, 5.2)});

  auto series = field_method.run_all(asset);
  ASSERT_EQ(series.size(), asset.quotes().size() - offset);
  EXPECT_EQ(series[0], asset.quotes()[1].high());
  EXPECT_EQ(series[1], asset.quotes()[0].high());
}

TEST(FieldMethodTest, RunAllMethodLow)
{
  const auto field = "low";
  const auto offset = 1;
  const auto field_method = FieldMethod{field, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 1.0, 2.0, 3.0, 4.0, 5.0),
                                    pludux::Quote(0, 1.1, 2.1, 3.1, 4.1, 5.1),
                                    pludux::Quote(0, 1.2, 2.2, 3.2, 4.2, 5.2)});

  auto series = field_method.run_all(asset);
  ASSERT_EQ(series.size(), asset.quotes().size() - offset);
  EXPECT_EQ(series[0], asset.quotes()[1].low());
  EXPECT_EQ(series[1], asset.quotes()[0].low());
}

TEST(FieldMethodTest, RunAllMethodVolume)
{
  const auto field = "volume";
  const auto offset = 1;
  const auto field_method = FieldMethod{field, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 1.0, 2.0, 3.0, 4.0, 5.0),
                                    pludux::Quote(0, 1.1, 2.1, 3.1, 4.1, 5.1),
                                    pludux::Quote(0, 1.2, 2.2, 3.2, 4.2, 5.2)});

  auto series = field_method.run_all(asset);
  ASSERT_EQ(series.size(), asset.quotes().size() - offset);
  EXPECT_EQ(series[0], asset.quotes()[1].volume());
  EXPECT_EQ(series[1], asset.quotes()[0].volume());
}

TEST(FieldMethodTest, InvalidField)
{
  const auto field = "invalid";
  const auto offset = 1;
  const auto field_method = FieldMethod{field, offset};
  const auto asset = pludux::Asset("",
                                   {pludux::Quote(0, 1.0, 2.0, 3.0, 4.0, 5.0),
                                    pludux::Quote(0, 1.1, 2.1, 3.1, 4.1, 5.1),
                                    pludux::Quote(0, 1.2, 2.2, 3.2, 4.2, 5.2)});

  EXPECT_THROW(field_method.run_all(asset), std::runtime_error);
}