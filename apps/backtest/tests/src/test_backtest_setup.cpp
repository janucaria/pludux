#include <gtest/gtest.h>

#include <vector>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

TEST(StrategyTest, DefaultsToIncompleteConfiguration)
{
  const auto strategy = Strategy{};
  const auto system = System{};

  EXPECT_TRUE(strategy.name().empty());
  EXPECT_EQ(strategy.model_handle(), ModelStoreHandle{});
  EXPECT_EQ(strategy.profile_handle(), ProfileStoreHandle{});
  EXPECT_TRUE(strategy.inputs().empty());
  EXPECT_NE(node_cast<TrueNode>(strategy.entry_filter()), nullptr);
  EXPECT_EQ(system.model_performance(), ModelPerformanceConfig{});
  EXPECT_EQ(SystemFailsafeStrategy{}.activation(),
            FailsafeStrategyActivation::Always);
}

TEST(StrategyTest, StoresReusableStrategyConfiguration)
{
  const auto model_handle = ModelStoreHandle{1, 2};
  const auto profile = ProfileStoreHandle{3, 4};
  const auto inputs = std::vector<NumericInputNode>{NumericInputNode{
   "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 20.0}};
  const auto strategy =
   Strategy{"Breakout",
             model_handle,
             profile,
             inputs,
             ErasedNode<EntryFilterMethodContext>{FalseNode{}}};

  EXPECT_EQ(strategy.name(), "Breakout");
  EXPECT_EQ(strategy.model_handle(), model_handle);
  EXPECT_EQ(strategy.profile_handle(), profile);
  EXPECT_EQ(strategy.inputs(), inputs);
  EXPECT_NE(node_cast<FalseNode>(strategy.entry_filter()), nullptr);
}

TEST(StrategyTest, EqualityAndRuleEquivalenceIncludeAllFields)
{
  const auto original = Strategy{
   "Original",
   ModelStoreHandle{1, 1},
   ProfileStoreHandle{2, 1},
   {NumericInputNode{
    "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 20.0}}};
  auto changed = original;

  EXPECT_EQ(original, changed);
  EXPECT_TRUE(original.equivalent_rules(changed));

  changed.name("Renamed");
  EXPECT_NE(original, changed);
  EXPECT_TRUE(original.equivalent_rules(changed));

  changed = original;

  changed.inputs({NumericInputNode{
   "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 55.0}});
  EXPECT_NE(original, changed);
  EXPECT_FALSE(original.equivalent_rules(changed));

  changed = original;
  changed.entry_filter(ErasedNode<EntryFilterMethodContext>{FalseNode{}});
  EXPECT_NE(original, changed);
  EXPECT_FALSE(original.equivalent_rules(changed));
}

TEST(SystemTest, StoresMainAndOrderedFailsafeStrategies)
{
  const auto main = StrategyStoreHandle{1, 1};
  const auto first = SystemFailsafeStrategy{
   StrategyStoreHandle{3, 1},
   FailsafeStrategyActivation::PreviousStrategyEntryFilteredPosition};
  const auto second = SystemFailsafeStrategy{
   main};
  const auto system = System{"Trend",
                              WatchlistStoreHandle{7, 1},
                              ModelPerformanceConfig{},
                              main,
                              {first, second}};

  EXPECT_EQ(system.main_strategy_handle(), main);
  EXPECT_EQ(system.failsafe_strategies(),
            (std::vector<SystemFailsafeStrategy>{first, second}));
  EXPECT_EQ(system.strategy_handle(1), first.strategy_handle());
  EXPECT_EQ(system.strategy_handle(2), main);
  EXPECT_EQ(system.strategy_count(), 3U);
  EXPECT_EQ(system.model_performance(), ModelPerformanceConfig{});
}

TEST(SystemTest, OwnsSharedModelPerformanceConfiguration)
{
  const auto performance =
   ModelPerformanceConfig{ModelPerformanceHistoryPolicy{
    ModelPerformanceHistoryMode::RollingWindow, 25, 0.99}};
  const auto original = System{"Trend",
                               WatchlistStoreHandle{7, 1},
                               performance,
                               StrategyStoreHandle{},
                               {SystemFailsafeStrategy{}}};

  EXPECT_EQ(original.model_performance(), performance);

  auto renamed = original;
  renamed.name("Renamed");
  EXPECT_NE(original, renamed);
  EXPECT_TRUE(original.equivalent_rules(renamed));

  auto changed = original;
  changed.model_performance(
   ModelPerformanceConfig{ModelPerformanceHistoryPolicy{
    ModelPerformanceHistoryMode::ExponentialDecay, 100, 0.75}});
  EXPECT_NE(original, changed);
  EXPECT_FALSE(original.equivalent_rules(changed));

  changed = original;
  changed.failsafe_strategies().front().activation(
   FailsafeStrategyActivation::PreviousStrategyEntryFilteredPosition);
  EXPECT_NE(original, changed);
  EXPECT_FALSE(original.equivalent_rules(changed));
}

TEST(StrategyTest, StoresReferencesAndSupportsStoreCrud)
{
  const auto model = ModelStoreHandle{1, 2};
  const auto profile = ProfileStoreHandle{3, 4};
  auto store = Store{};
  const auto invalid = StrategyStoreHandle{};
  auto resolver = StoreDataResolver<Strategy, StrategyStoreHandle>{};
  auto strategies = std::vector<Strategy>{};

  EXPECT_FALSE(invalid.valid());
  EXPECT_FALSE(resolver.is_alive(invalid));
  EXPECT_FALSE(resolver.add(strategies, invalid, Strategy{}));
  EXPECT_EQ(store.get_strategy_if_present(invalid), nullptr);
  EXPECT_FALSE(store.remove_strategy(invalid));

  const auto handle = store.add_strategy(Strategy{"Trend", model, profile});
  ASSERT_TRUE(handle.has_value());
  EXPECT_EQ(*handle, (StrategyStoreHandle{0, 0}));
  EXPECT_NE(*handle, invalid);
  EXPECT_TRUE(handle->valid());

  const auto* strategy = store.get_strategy_if_present(*handle);
  ASSERT_NE(strategy, nullptr);
  EXPECT_EQ(strategy->name(), "Trend");
  EXPECT_TRUE(strategy->references_model(model));
  EXPECT_TRUE(strategy->references_profile(profile));
  EXPECT_FALSE(strategy->references_model(ModelStoreHandle{2, 1}));
  EXPECT_FALSE(strategy->references_profile(ProfileStoreHandle{4, 1}));

  EXPECT_TRUE(store.update_strategy(*handle, Strategy{"Revised", model, profile}));
  EXPECT_EQ(store.get_strategy(*handle).name(), "Revised");
  EXPECT_TRUE(store.remove_strategy(*handle));
  EXPECT_EQ(store.get_strategy_if_present(*handle), nullptr);
}
