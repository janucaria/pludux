#include <gtest/gtest.h>

#include <vector>

import pludux.apps.backtest.portfolio_backtest_setup_selections;
import pludux.backtest;

using pludux::apps::PortfolioBacktestSetupKey;
using pludux::apps::PortfolioBacktestSetupSelection;
using pludux::apps::PortfolioBacktestSetupSelections;
using namespace pludux::backtest;

TEST(PortfolioBacktestSetupSelections, RemembersOneSetupPerPortfolio)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto run = BacktestRunKey{{2, 1}, {3, 1}};
  const auto main = PortfolioBacktestSetupKey{run, 0};
  const auto failsafe = PortfolioBacktestSetupKey{run, 1};
  auto selections = PortfolioBacktestSetupSelections{};

  selections.remember(portfolio, main);
  EXPECT_EQ(selections.lookup(portfolio), main);
  selections.remember(portfolio, failsafe);
  EXPECT_EQ(selections.lookup(portfolio), failsafe);
  EXPECT_EQ(selections.selections().size(), 1);
}

TEST(PortfolioBacktestSetupSelections, NormalizesToFirstOrderedValidSetup)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto run = BacktestRunKey{{2, 1}, {3, 1}};
  const auto main = PortfolioBacktestSetupKey{run, 0};
  const auto failsafe = PortfolioBacktestSetupKey{run, 1};
  auto selections = PortfolioBacktestSetupSelections{};
  const auto setups = std::vector{main, failsafe};

  EXPECT_EQ(selections.normalize(portfolio, setups, [](auto) { return true; }),
            main);
  selections.remember(portfolio, failsafe);
  EXPECT_EQ(selections.normalize(portfolio, setups, [](auto) { return true; }),
            failsafe);
}

TEST(PortfolioBacktestSetupSelections, FallsBackAfterRememberedSetupIsRemoved)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto run = BacktestRunKey{{2, 1}, {3, 1}};
  const auto main = PortfolioBacktestSetupKey{run, 0};
  const auto removed_failsafe = PortfolioBacktestSetupKey{run, 2};
  auto selections = PortfolioBacktestSetupSelections{};
  selections.remember(portfolio, removed_failsafe);

  EXPECT_EQ(selections.normalize(
             portfolio, std::vector{main}, [](auto) { return true; }),
            main);
  EXPECT_EQ(selections.lookup(portfolio), main);
}

TEST(PortfolioBacktestSetupSelections,
     RemovesBacktestAssetAndPortfolioReferences)
{
  const auto first_portfolio = PortfolioStoreHandle{1, 1};
  const auto second_portfolio = PortfolioStoreHandle{2, 1};
  const auto first = PortfolioBacktestSetupKey{{{3, 1}, {5, 1}}, 0};
  const auto second = PortfolioBacktestSetupKey{{{4, 1}, {6, 1}}, 1};
  auto selections = PortfolioBacktestSetupSelections{
   std::vector<PortfolioBacktestSetupSelection>{{first_portfolio, first},
                                                {second_portfolio, second}}};

  selections.remove_asset(first.run.asset_handle);
  EXPECT_FALSE(selections.lookup(first_portfolio));
  EXPECT_TRUE(selections.lookup(second_portfolio));
  selections.remove_backtest(second.run.backtest_handle);
  EXPECT_FALSE(selections.lookup(second_portfolio));

  selections.remember(first_portfolio, first);
  selections.remove_portfolio(first_portfolio);
  EXPECT_FALSE(selections.lookup(first_portfolio));
}

TEST(PortfolioBacktestSetupSelections, EmptyOrInvalidSetupsHaveNoSelection)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto setup = PortfolioBacktestSetupKey{{{2, 1}, {3, 1}}, 0};
  auto selections = PortfolioBacktestSetupSelections{};

  EXPECT_FALSE(selections.normalize(portfolio,
                                    std::vector<PortfolioBacktestSetupKey>{},
                                    [](auto) { return true; }));
  EXPECT_FALSE(selections.normalize(
   portfolio, std::vector{setup}, [](auto) { return false; }));
}
