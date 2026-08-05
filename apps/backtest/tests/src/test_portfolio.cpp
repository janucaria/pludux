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
                                   DrawdownAdjustment{true, 0.10, 0.25},
                                   InsufficientCashPolicy::CapToAvailableCash,
                                   {first, second}};

  EXPECT_EQ(portfolio.name(), "Balanced");
  EXPECT_DOUBLE_EQ(portfolio.initial_capital(), 100'000.0);
  EXPECT_EQ(portfolio.backtest_handles(),
            (std::vector<BacktestStoreHandle>{first, second}));
  EXPECT_EQ(portfolio.maximum_open_trades(), 4);
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
                          {},
                          InsufficientCashPolicy::Reject,
                          {handle, handle}}),
               std::invalid_argument);
}

TEST(PortfolioTest, AllowsIncompleteConfigurationWithoutBacktests)
{
  const auto portfolio =
   Portfolio{"", 1'000.0, {}, {}, 0, {}, InsufficientCashPolicy::Reject, {}};

  EXPECT_TRUE(portfolio.name().empty());
  EXPECT_EQ(portfolio.market_handle(), MarketStoreHandle{});
  EXPECT_EQ(portfolio.broker_handle(), BrokerStoreHandle{});
  EXPECT_EQ(portfolio.maximum_open_trades(), 0);
  EXPECT_TRUE(portfolio.backtest_handles().empty());
}

TEST(PortfolioTest, DefaultsMaximumOpenTradesToTen)
{
  auto portfolio = Portfolio{};

  EXPECT_EQ(portfolio.maximum_open_trades(), 10);
  portfolio.maximum_open_trades(0);
  EXPECT_EQ(portfolio.maximum_open_trades(), 0);
}

TEST(PortfolioTest, BacktestContainsOnlyReusableBacktestConfiguration)
{
  const auto backtest = Backtest{"BTC trend",
                                 AssetStoreHandle{1, 1},
                                 StrategyStoreHandle{2, 1},
                                 ProfileStoreHandle{3, 1}};
  EXPECT_EQ(backtest.name(), "BTC trend");
  EXPECT_EQ(backtest.asset_handle(), (AssetStoreHandle{1, 1}));
  EXPECT_EQ(backtest.strategy_handle(), (StrategyStoreHandle{2, 1}));
  EXPECT_EQ(backtest.profile_handle(), (ProfileStoreHandle{3, 1}));
}
