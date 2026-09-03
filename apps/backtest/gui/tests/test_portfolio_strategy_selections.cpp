#include <gtest/gtest.h>

#include <vector>

import pludux.apps.backtest.portfolio_strategy_selections;
import pludux.backtest;

using pludux::apps::PortfolioStrategyKey;
using pludux::apps::PortfolioStrategySelection;
using pludux::apps::PortfolioStrategySelections;
using namespace pludux::backtest;

TEST(PortfolioStrategySelections, RemembersOneStrategyPerPortfolio)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto run = BacktestRunKey{{2, 1}, {3, 1}};
  const auto main = PortfolioStrategyKey{run, 0};
  const auto failsafe = PortfolioStrategyKey{run, 1};
  auto selections = PortfolioStrategySelections{};

  selections.remember(portfolio, main);
  EXPECT_EQ(selections.lookup(portfolio), main);
  selections.remember(portfolio, failsafe);
  EXPECT_EQ(selections.lookup(portfolio), failsafe);
  EXPECT_EQ(selections.selections().size(), 1);
}

TEST(PortfolioStrategySelections, NormalizesToFirstOrderedValidStrategy)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto run = BacktestRunKey{{2, 1}, {3, 1}};
  const auto main = PortfolioStrategyKey{run, 0};
  const auto failsafe = PortfolioStrategyKey{run, 1};
  auto selections = PortfolioStrategySelections{};
  const auto strategies = std::vector{main, failsafe};

  EXPECT_EQ(selections.normalize(portfolio, strategies, [](auto) { return true; }),
            main);
  selections.remember(portfolio, failsafe);
  EXPECT_EQ(selections.normalize(portfolio, strategies, [](auto) { return true; }),
            failsafe);
}

TEST(PortfolioStrategySelections, FallsBackAfterRememberedStrategyIsRemoved)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto run = BacktestRunKey{{2, 1}, {3, 1}};
  const auto main = PortfolioStrategyKey{run, 0};
  const auto removed_failsafe = PortfolioStrategyKey{run, 2};
  auto selections = PortfolioStrategySelections{};
  selections.remember(portfolio, removed_failsafe);

  EXPECT_EQ(selections.normalize(
             portfolio, std::vector{main}, [](auto) { return true; }),
            main);
  EXPECT_EQ(selections.lookup(portfolio), main);
}

TEST(PortfolioStrategySelections, RemovesSystemAssetAndPortfolioReferences)
{
  const auto first_portfolio = PortfolioStoreHandle{1, 1};
  const auto second_portfolio = PortfolioStoreHandle{2, 1};
  const auto first = PortfolioStrategyKey{{{3, 1}, {5, 1}}, 0};
  const auto second = PortfolioStrategyKey{{{4, 1}, {6, 1}}, 1};
  auto selections = PortfolioStrategySelections{
   std::vector<PortfolioStrategySelection>{{first_portfolio, first},
                                         {second_portfolio, second}}};

  selections.remove_asset(first.run.asset_handle);
  EXPECT_FALSE(selections.lookup(first_portfolio));
  EXPECT_TRUE(selections.lookup(second_portfolio));
   selections.remove_system(second.run.system_handle);
  EXPECT_FALSE(selections.lookup(second_portfolio));

  selections.remember(first_portfolio, first);
  selections.remove_portfolio(first_portfolio);
  EXPECT_FALSE(selections.lookup(first_portfolio));
}

TEST(PortfolioStrategySelections, EmptyOrInvalidStrategiesHaveNoSelection)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto strategy = PortfolioStrategyKey{{{2, 1}, {3, 1}}, 0};
  auto selections = PortfolioStrategySelections{};

  EXPECT_FALSE(selections.normalize(portfolio,
                                     std::vector<PortfolioStrategyKey>{},
                                    [](auto) { return true; }));
  EXPECT_FALSE(selections.normalize(
    portfolio, std::vector{strategy}, [](auto) { return false; }));
}
