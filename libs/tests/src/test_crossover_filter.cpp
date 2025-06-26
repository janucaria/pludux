#include <gtest/gtest.h>
#include <pludux/asset_history.hpp>
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
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

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
  const auto asset_data = pludux::AssetHistory{{"close", {0}}};

  EXPECT_EQ(filter.signal()(asset_data)[0], signal_value);
}

TEST(CrossoverFilterTest, CrossoverConditionMet)
{
  auto signal_method = DataMethod{"close", 0};
  auto reference_method = ValueMethod{50.0};
  const auto crossover_filter =
   CrossoverFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = pludux::AssetHistory{{"close", {60, 50}}};

  EXPECT_TRUE(crossover_filter(asset_data));
}

TEST(CrossoverFilterTest, CrossoverConditionNotMet)
{
  auto signal_method = DataMethod{"close", 0};
  auto reference_method = ValueMethod{50.0};
  const auto crossover_filter =
   CrossoverFilter{std::move(signal_method), std::move(reference_method)};
  const auto asset_data = pludux::AssetHistory{{"close", {50, 40}}};

  EXPECT_FALSE(crossover_filter(asset_data));
}

TEST(CrossoverFilterTest, EqualityOperator)
{
  auto signal_method1 = DataMethod{"close", 0};
  auto reference_method1 = ValueMethod{50.0};
  const auto crossover_filter1 =
   CrossoverFilter{std::move(signal_method1), std::move(reference_method1)};

  auto signal_method2 = DataMethod{"close", 0};
  auto reference_method2 = ValueMethod{50.0};
  const auto crossover_filter2 =
   CrossoverFilter{std::move(signal_method2), std::move(reference_method2)};

  EXPECT_TRUE(crossover_filter1 == crossover_filter2);
  EXPECT_FALSE(crossover_filter1 != crossover_filter2);
  EXPECT_EQ(crossover_filter1, crossover_filter2);
}

TEST(CrossoverFilterTest, NotEqualOperator)
{
  auto signal_method1 = DataMethod{"close", 0};
  auto reference_method1 = ValueMethod{50.0};
  const auto crossover_filter1 =
   CrossoverFilter{std::move(signal_method1), std::move(reference_method1)};

  auto signal_method2 = DataMethod{"close", 0};
  auto reference_method2 = ValueMethod{60.0};
  const auto crossover_filter2 =
   CrossoverFilter{std::move(signal_method2), std::move(reference_method2)};

  EXPECT_TRUE(crossover_filter1 != crossover_filter2);
  EXPECT_FALSE(crossover_filter1 == crossover_filter2);
  EXPECT_NE(crossover_filter1, crossover_filter2);
}
