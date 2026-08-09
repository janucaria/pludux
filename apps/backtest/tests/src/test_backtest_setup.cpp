#include <gtest/gtest.h>

#include <vector>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

TEST(BacktestSetupTest, DefaultsToIncompleteConfiguration)
{
  const auto setup = BacktestSetup{};
  const auto backtest = Backtest{};

  EXPECT_EQ(setup.strategy_handle(), StrategyStoreHandle{});
  EXPECT_EQ(setup.profile_handle(), ProfileStoreHandle{});
  EXPECT_TRUE(setup.inputs().empty());
  EXPECT_EQ(backtest.strategy_performance(), StrategyPerformanceConfig{});
  EXPECT_EQ(BacktestFailsafeSetup{}.activation(), FailsafeActivation::Always);
}

TEST(BacktestSetupTest, StoresReusableSetupConfiguration)
{
  const auto strategy = StrategyStoreHandle{1, 2};
  const auto profile = ProfileStoreHandle{3, 4};
  const auto inputs = std::vector<NumericInputNode>{NumericInputNode{
   "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 20.0}};
  const auto setup = BacktestSetup{strategy, profile, inputs};

  EXPECT_EQ(setup.strategy_handle(), strategy);
  EXPECT_EQ(setup.profile_handle(), profile);
  EXPECT_EQ(setup.inputs(), inputs);
}

TEST(BacktestSetupTest, EqualityAndRuleEquivalenceIncludeAllFields)
{
  const auto original = BacktestSetup{
   StrategyStoreHandle{1, 1},
   ProfileStoreHandle{2, 1},
   {NumericInputNode{
    "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 20.0}}};
  auto changed = original;

  EXPECT_EQ(original, changed);
  EXPECT_TRUE(original.equivalent_rules(changed));

  changed.inputs({NumericInputNode{
   "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 55.0}});
  EXPECT_NE(original, changed);
  EXPECT_FALSE(original.equivalent_rules(changed));
}

TEST(BacktestSetupTest, BacktestStoresMainAndOrderedFailsafeSetups)
{
  const auto main =
   BacktestSetup{StrategyStoreHandle{1, 1}, ProfileStoreHandle{2, 1}};
  const auto first = BacktestFailsafeSetup{
   BacktestSetup{StrategyStoreHandle{3, 1}, ProfileStoreHandle{4, 1}},
   FailsafeActivation::PreviousSetupFilteredPosition};
  const auto second = BacktestFailsafeSetup{
   BacktestSetup{StrategyStoreHandle{5, 1}, ProfileStoreHandle{6, 1}}};
  const auto backtest = Backtest{"Trend",
                                 WatchlistStoreHandle{7, 1},
                                 StrategyPerformanceConfig{},
                                 main,
                                 {first, second}};

  EXPECT_EQ(backtest.main_setup(), main);
  EXPECT_EQ(backtest.failsafe_setups(),
            (std::vector<BacktestFailsafeSetup>{first, second}));
  EXPECT_EQ(backtest.setup(1), first.setup());
  EXPECT_EQ(backtest.setup(2), second.setup());
  EXPECT_EQ(backtest.strategy_performance(), StrategyPerformanceConfig{});
}

TEST(BacktestSetupTest, BacktestOwnsSharedStrategyPerformanceConfiguration)
{
  const auto performance =
   StrategyPerformanceConfig{StrategyPerformanceHistoryPolicy{
    StrategyPerformanceHistoryMode::RollingWindow, 25, 0.99}};
  const auto original = Backtest{"Trend",
                                 WatchlistStoreHandle{7, 1},
                                 performance,
                                 BacktestSetup{},
                                 {BacktestFailsafeSetup{}}};

  EXPECT_EQ(original.strategy_performance(), performance);

  auto renamed = original;
  renamed.name("Renamed");
  EXPECT_NE(original, renamed);
  EXPECT_TRUE(original.equivalent_rules(renamed));

  auto changed = original;
  changed.strategy_performance(
   StrategyPerformanceConfig{StrategyPerformanceHistoryPolicy{
    StrategyPerformanceHistoryMode::ExponentialDecay, 100, 0.75}});
  EXPECT_NE(original, changed);
  EXPECT_FALSE(original.equivalent_rules(changed));

  changed = original;
  changed.failsafe_setups().front().activation(
   FailsafeActivation::PreviousSetupFilteredPosition);
  EXPECT_NE(original, changed);
  EXPECT_FALSE(original.equivalent_rules(changed));
}
