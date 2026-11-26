#include <gtest/gtest.h>

#include <limits>
#include <vector>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

TEST(StrategySetupTest, DefaultsToIncompleteConfiguration)
{
  const auto system = System{};
  EXPECT_EQ(system.profile_handle(), ProfileStoreHandle{});
  EXPECT_EQ(system.main_setup().strategy_handle(), StrategyStoreHandle{});
  EXPECT_TRUE(system.main_setup().inputs().empty());
  EXPECT_NE(node_cast<TrueNode>(system.main_setup().entry_filter()), nullptr);
  EXPECT_EQ(system.strategy_performance(), StrategyPerformanceConfig{});
  EXPECT_EQ(SystemFailsafeStrategy{}.activation(),
            FailsafeStrategyActivation::Always);
}

TEST(SystemTest,
     MainAndFailsafeUseSameStrategyWithIndependentOverridesAndOneProfile)
{
  const auto strategy = StrategyStoreHandle{1, 2};
  const auto profile = ProfileStoreHandle{3, 4};
  const auto input = [](double period) {
    return NumericInputNode{
     "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, period};
  };
  const auto main = StrategySetup{strategy, {input(20.0)}, FalseNode{}};
  const auto failsafe = SystemFailsafeStrategy{
   StrategySetup{strategy, {input(55.0)}},
   FailsafeStrategyActivation::PreviousStrategyEntryFilteredPosition};
  const auto other =
   SystemFailsafeStrategy{StrategySetup{StrategyStoreHandle{7, 1}}};
  const auto system = System{"Breakout",
                             WatchlistStoreHandle{8, 1},
                             profile,
                             {},
                             main,
                             {failsafe, other}};

  EXPECT_EQ(system.profile_handle(), profile);
  EXPECT_EQ(system.strategy_count(), 3U);
  EXPECT_EQ(system.main_setup(), main);
  EXPECT_EQ(system.strategy_setup(1), failsafe.setup());
  EXPECT_EQ(system.strategy_setup(2), other.setup());
  EXPECT_EQ(system.failsafe_strategies().front().activation(),
            FailsafeStrategyActivation::PreviousStrategyEntryFilteredPosition);
  EXPECT_EQ(system.strategy_setup(0).strategy_handle(),
            system.strategy_setup(1).strategy_handle());
  EXPECT_DOUBLE_EQ(system.strategy_setup(0).inputs().front().value(), 20.0);
  EXPECT_DOUBLE_EQ(system.strategy_setup(1).inputs().front().value(), 55.0);
  EXPECT_NE(node_cast<FalseNode>(system.strategy_setup(0).entry_filter()),
            nullptr);
  EXPECT_NE(node_cast<TrueNode>(system.strategy_setup(1).entry_filter()),
            nullptr);
}

TEST(SystemTest, RuleEquivalenceIncludesSetupFieldsAndSharedPolicy)
{
  const auto profile = ProfileStoreHandle{1, 2};
  const auto performance =
   StrategyPerformanceConfig{StrategyPerformanceHistoryPolicy{
    StrategyPerformanceHistoryMode::RollingWindow, 25, 0.99}};
  const auto original = System{"Trend",
                               WatchlistStoreHandle{7, 1},
                               profile,
                               performance,
                               StrategySetup{StrategyStoreHandle{2, 1}},
                               {SystemFailsafeStrategy{}}};
  EXPECT_EQ(original.strategy_performance(), performance);
  auto changed = original;
  changed.name("Renamed");
  EXPECT_NE(original, changed);
  EXPECT_TRUE(original.equivalent_rules(changed));

  changed = original;
  changed.profile_handle(ProfileStoreHandle{3, 4});
  EXPECT_FALSE(original.equivalent_rules(changed));
  changed = original;
  changed.strategy_performance(StrategyPerformanceConfig{});
  EXPECT_FALSE(original.equivalent_rules(changed));
  changed = original;
  changed.failsafe_strategies().front().activation(
   FailsafeStrategyActivation::PreviousStrategyEntryFilteredPosition);
  EXPECT_FALSE(original.equivalent_rules(changed));
  changed = original;
  auto main = original.main_setup();
  main.inputs({NumericInputNode{
   "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 55.0}});
  changed.main_setup() = main;
  EXPECT_FALSE(original.equivalent_rules(changed));
  changed = original;
  main = original.main_setup();
  main.entry_filter(FalseNode{});
  changed.main_setup() = main;
  EXPECT_FALSE(original.equivalent_rules(changed));
}

TEST(StrategyTest, StoresReusableTradingRulesAndRejectsInvalidHandles)
{
  auto store = Store{};
  const auto invalid = StrategyStoreHandle{};
  auto resolver = StoreDataResolver<Strategy, StrategyStoreHandle>{};
  auto strategies = std::vector<Strategy>{};

  EXPECT_FALSE(invalid.valid());
  EXPECT_FALSE(resolver.is_alive(invalid));
  EXPECT_FALSE(resolver.add(strategies, invalid, Strategy{}));
  EXPECT_EQ(store.get_strategy_if_present(invalid), nullptr);
  EXPECT_FALSE(store.remove_strategy(invalid));

  auto strategy = Strategy{};
  strategy.name("Trend");
  const auto handle = store.add_strategy(strategy);
  ASSERT_TRUE(handle.has_value());
  EXPECT_EQ(*handle, (StrategyStoreHandle{0, 0}));
  ASSERT_NE(store.get_strategy_if_present(*handle), nullptr);
  EXPECT_EQ(store.get_strategy(*handle), strategy);
  strategy.name("Revised");
  EXPECT_TRUE(store.update_strategy(*handle, strategy));
  EXPECT_EQ(store.get_strategy(*handle).name(), "Revised");
  EXPECT_TRUE(store.remove_strategy(*handle));
  EXPECT_EQ(store.get_strategy_if_present(*handle), nullptr);
}

TEST(StoreDataResolverTest, RetiresSlotsBeforeGenerationWrap)
{
  auto resolver = StoreDataResolver<Strategy, StrategyStoreHandle>{};
  auto strategies = std::vector<Strategy>{};
  const auto exhausted =
   StrategyStoreHandle{0, std::numeric_limits<StoreGeneration>::max()};
  ASSERT_TRUE(resolver.add(strategies, exhausted, Strategy{}));
  ASSERT_TRUE(resolver.remove(strategies, exhausted));

  const auto next = resolver.add(strategies, Strategy{});
  EXPECT_EQ(next, (StrategyStoreHandle{1, 0}));
  EXPECT_FALSE(resolver.is_alive(exhausted));
  EXPECT_FALSE(resolver.is_alive(StrategyStoreHandle{0, 0}));
}
