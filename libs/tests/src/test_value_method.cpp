#include <gtest/gtest.h>
#include <pludux/asset.hpp>
#include <pludux/screener/value_method.hpp>
#include <pludux/series.hpp>

using namespace pludux::screener;

TEST(ValueMethodTest, ConstructorInitialization)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};

  EXPECT_EQ(value_method.run_one(pludux::Asset{""}), value);
}

TEST(ValueMethodTest, RunAllMethod)
{
  const auto value = 42.0;
  const auto value_method = ValueMethod{value};
  const auto asset = pludux::Asset{""};

  auto series = value_method.run_all(asset);
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

  EXPECT_EQ(value_method.run_one(asset), value);
}