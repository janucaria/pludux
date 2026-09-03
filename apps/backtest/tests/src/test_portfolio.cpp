#include <gtest/gtest.h>

#include <stdexcept>
#include <vector>

import pludux.backtest;

using namespace pludux::backtest;
using pludux::ValueNode;

TEST(PortfolioTest, StoresSharedAccountConfigurationAndOrderedBacktests)
{
  const auto first = SystemStoreHandle{1, 2};
  const auto second = SystemStoreHandle{3, 4};
  const auto portfolio =
   Portfolio{"Balanced",
             100'000.0,
             MarketStoreHandle{5, 6},
             BrokerStoreHandle{7, 8},
             4,
             6,
             {PortfolioEntryComparator{
              ValueNode{42.0}, PortfolioEntryComparatorOrder::HigherFirst}},
             {first, second}};

  EXPECT_EQ(portfolio.name(), "Balanced");
  EXPECT_DOUBLE_EQ(portfolio.initial_capital(), 100'000.0);
  EXPECT_EQ(portfolio.system_handles(),
            (std::vector<SystemStoreHandle>{first, second}));
  EXPECT_EQ(portfolio.maximum_open_trades(), 4);
  EXPECT_EQ(portfolio.maximum_combined_layers(), 6);
  ASSERT_EQ(portfolio.entry_comparators().size(), 1U);
  EXPECT_EQ(portfolio.entry_comparators().front().order(),
            PortfolioEntryComparatorOrder::HigherFirst);
}

TEST(PortfolioTest, RejectsDuplicateBacktestHandles)
{
  const auto handle = SystemStoreHandle{1, 1};
  EXPECT_THROW(
   (Portfolio{"Duplicate", 1'000.0, {}, {}, 10, 10, {}, {handle, handle}}),
   std::invalid_argument);
}

TEST(PortfolioTest, AllowsIncompleteConfigurationWithoutBacktests)
{
  const auto portfolio = Portfolio{"", 1'000.0, {}, {}, 0, 0, {}, {}};

  EXPECT_TRUE(portfolio.name().empty());
  EXPECT_EQ(portfolio.market_handle(), MarketStoreHandle{});
  EXPECT_EQ(portfolio.broker_handle(), BrokerStoreHandle{});
  EXPECT_EQ(portfolio.maximum_open_trades(), 0);
  EXPECT_EQ(portfolio.maximum_combined_layers(), 0);
  EXPECT_TRUE(portfolio.system_handles().empty());
}

TEST(PortfolioTest, DefaultsMaximumOpenTradesToTen)
{
  auto portfolio = Portfolio{};

  EXPECT_EQ(portfolio.maximum_open_trades(), 10);
  portfolio.maximum_open_trades(0);
  EXPECT_EQ(portfolio.maximum_open_trades(), 0);
}

TEST(PortfolioTest, DefaultsMaximumCombinedLayersToTen)
{
  auto portfolio = Portfolio{};

  EXPECT_EQ(portfolio.maximum_combined_layers(), 10);
  portfolio.maximum_combined_layers(0);
  EXPECT_EQ(portfolio.maximum_combined_layers(), 0);
}

TEST(PortfolioTest, DefaultsToPortfolioOrderWithoutEntryComparators)
{
  const auto portfolio = Portfolio{};

  EXPECT_TRUE(portfolio.entry_comparators().empty());
}

TEST(ProfileTest, DefaultsCapitalProtectionPolicies)
{
  const auto profile = Profile{};
  const auto& adjustment = profile.drawdown_adjustment();

  EXPECT_FALSE(adjustment.enabled());
  EXPECT_DOUBLE_EQ(adjustment.drawdown_step(), 0.10);
  EXPECT_DOUBLE_EQ(adjustment.size_reduction(), 0.20);
  EXPECT_DOUBLE_EQ(adjustment.notional_equity_reduction(), 0.0);
  EXPECT_EQ(profile.insufficient_cash_policy(), InsufficientCashPolicy::Reject);
}

TEST(ProfileTest, StoresCapitalProtectionPolicies)
{
  const auto profile =
   Profile{"Aggressive",
           PositionSizingNode{FixedQuantityPositionSizing{2.0}},
           DrawdownAdjustment{true, 0.10, 0.25, 0.15},
           InsufficientCashPolicy::CapToAvailableCash};

  EXPECT_TRUE(profile.drawdown_adjustment().enabled());
  EXPECT_DOUBLE_EQ(profile.drawdown_adjustment().size_reduction(), 0.25);
  EXPECT_DOUBLE_EQ(profile.drawdown_adjustment().notional_equity_reduction(),
                   0.15);
  EXPECT_EQ(profile.insufficient_cash_policy(),
            InsufficientCashPolicy::CapToAvailableCash);
}

TEST(ProfileTest, CapitalProtectionParticipatesInRuleEquivalence)
{
  auto baseline = Profile{};
  auto drawdown = baseline;
  drawdown.drawdown_adjustment(DrawdownAdjustment{true, 0.10, 0.0, 0.20});
  auto cash = baseline;
  cash.insufficient_cash_policy(InsufficientCashPolicy::CapToAvailableCash);

  EXPECT_FALSE(baseline.equivalent_rules(drawdown));
  EXPECT_FALSE(baseline.equivalent_rules(cash));
}

TEST(ProfileTest, ValidatesDrawdownAdjustmentParameters)
{
  EXPECT_THROW((DrawdownAdjustment{true, 0.0, 0.20, 0.20}),
               std::invalid_argument);
  EXPECT_THROW((DrawdownAdjustment{true, 0.10, -0.20, 0.20}),
               std::invalid_argument);
  EXPECT_THROW((DrawdownAdjustment{true, 0.10, 0.20, -0.20}),
               std::invalid_argument);

  auto adjustment = DrawdownAdjustment{};
  EXPECT_THROW(adjustment.drawdown_step(0.0), std::invalid_argument);
  EXPECT_THROW(adjustment.size_reduction(-0.01), std::invalid_argument);
  EXPECT_THROW(adjustment.notional_equity_reduction(-0.01),
               std::invalid_argument);
}

TEST(PortfolioTest, EntryComparatorsParticipateInRuleEquivalence)
{
  auto lhs = Portfolio{};
  auto rhs = lhs;
  rhs.entry_comparators().emplace_back(
   ValueNode{1.0}, PortfolioEntryComparatorOrder::LowerFirst);

  EXPECT_NE(lhs, rhs);
  EXPECT_FALSE(lhs.equivalent_rules(rhs));
}

TEST(SystemTest, ContainsOnlyReusableStrategyConfiguration)
{
  const auto system =
   System{"BTC trend",
          WatchlistStoreHandle{1, 1},
          ModelPerformanceConfig{},
          StrategyStoreHandle{2, 1}};
  EXPECT_EQ(system.name(), "BTC trend");
  EXPECT_EQ(system.watchlist_handle(), (WatchlistStoreHandle{1, 1}));
  EXPECT_EQ(system.main_strategy_handle(), (StrategyStoreHandle{2, 1}));
}

TEST(PortfolioTest, ResultsAreIdentifiedByBacktestAndAsset)
{
  const auto system_handle = SystemStoreHandle{1, 1};
  const auto first_asset = AssetStoreHandle{2, 1};
  const auto second_asset = AssetStoreHandle{3, 1};
  auto results =
   PortfolioResults{{},
                     {BacktestResults{system_handle, first_asset},
                      BacktestResults{system_handle, second_asset}}};

  ASSERT_NE(results.backtest({system_handle, first_asset}), nullptr);
  ASSERT_NE(results.backtest({system_handle, second_asset}), nullptr);
  EXPECT_NE(results.backtest({system_handle, first_asset}),
            results.backtest({system_handle, second_asset}));
}
