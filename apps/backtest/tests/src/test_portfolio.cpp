#include <gtest/gtest.h>

#include <vector>

import pludux.backtest;

using namespace pludux::backtest;

TEST(PortfolioTest, StoresSharedAccountConfigurationAndOrderedBacktests)
{
  const auto first = BacktestStoreHandle{1, 2};
  const auto second = BacktestStoreHandle{3, 4};
  const auto portfolio = Portfolio{"Balanced",
                                   100'000.0,
                                   MarketStoreHandle{5, 6},
                                   BrokerStoreHandle{7, 8},
                                   4,
                                   6,
                                   DrawdownAdjustment{true, 0.10, 0.25},
                                   InsufficientCashPolicy::CapToAvailableCash,
                                   {first, second}};

  EXPECT_EQ(portfolio.name(), "Balanced");
  EXPECT_DOUBLE_EQ(portfolio.initial_capital(), 100'000.0);
  EXPECT_EQ(portfolio.backtest_handles(),
            (std::vector<BacktestStoreHandle>{first, second}));
  EXPECT_EQ(portfolio.maximum_open_trades(), 4);
  EXPECT_EQ(portfolio.maximum_combined_layers(), 6);
  EXPECT_TRUE(portfolio.drawdown_adjustment().enabled());
  EXPECT_EQ(portfolio.insufficient_cash_policy(),
            InsufficientCashPolicy::CapToAvailableCash);
}

TEST(PortfolioTest, RejectsDuplicateBacktestHandles)
{
  const auto handle = BacktestStoreHandle{1, 1};
  EXPECT_THROW((Portfolio{"Duplicate",
                          1'000.0,
                          {},
                          {},
                          10,
                          10,
                          {},
                          InsufficientCashPolicy::Reject,
                          {handle, handle}}),
               std::invalid_argument);
}

TEST(PortfolioTest, AllowsIncompleteConfigurationWithoutBacktests)
{
  const auto portfolio =
   Portfolio{"", 1'000.0, {}, {}, 0, 0, {}, InsufficientCashPolicy::Reject, {}};

  EXPECT_TRUE(portfolio.name().empty());
  EXPECT_EQ(portfolio.market_handle(), MarketStoreHandle{});
  EXPECT_EQ(portfolio.broker_handle(), BrokerStoreHandle{});
  EXPECT_EQ(portfolio.maximum_open_trades(), 0);
  EXPECT_EQ(portfolio.maximum_combined_layers(), 0);
  EXPECT_TRUE(portfolio.backtest_handles().empty());
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

TEST(PortfolioTest, BacktestContainsOnlyReusableBacktestConfiguration)
{
  const auto backtest =
   Backtest{"BTC trend",
            WatchlistStoreHandle{1, 1},
            StrategyPerformanceConfig{},
            BacktestSetup{StrategyStoreHandle{2, 1}, ProfileStoreHandle{3, 1}}};
  EXPECT_EQ(backtest.name(), "BTC trend");
  EXPECT_EQ(backtest.watchlist_handle(), (WatchlistStoreHandle{1, 1}));
  EXPECT_EQ(backtest.main_setup().strategy_handle(),
            (StrategyStoreHandle{2, 1}));
  EXPECT_EQ(backtest.main_setup().profile_handle(), (ProfileStoreHandle{3, 1}));
}

TEST(PortfolioTest, ResultsAreIdentifiedByBacktestAndAsset)
{
  const auto backtest_handle = BacktestStoreHandle{1, 1};
  const auto first_asset = AssetStoreHandle{2, 1};
  const auto second_asset = AssetStoreHandle{3, 1};
  auto results =
   PortfolioResults{{},
                    {BacktestResults{backtest_handle, first_asset},
                     BacktestResults{backtest_handle, second_asset}}};

  ASSERT_NE(results.backtest({backtest_handle, first_asset}), nullptr);
  ASSERT_NE(results.backtest({backtest_handle, second_asset}), nullptr);
  EXPECT_NE(results.backtest({backtest_handle, first_asset}),
            results.backtest({backtest_handle, second_asset}));
}
