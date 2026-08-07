#include <gtest/gtest.h>

#include <vector>

import pludux.apps.backtest.portfolio_backtest_selections;
import pludux.backtest;

using pludux::apps::PortfolioBacktestSelection;
using pludux::apps::PortfolioBacktestSelections;
using namespace pludux::backtest;

TEST(PortfolioBacktestSelections, RemembersOneRunPerPortfolio)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto first = BacktestRunKey{{2, 1}, {3, 1}};
  const auto second = BacktestRunKey{{2, 1}, {4, 1}};
  auto selections = PortfolioBacktestSelections{};

  selections.remember(portfolio, first);
  EXPECT_EQ(selections.lookup(portfolio), first);
  selections.remember(portfolio, second);
  EXPECT_EQ(selections.lookup(portfolio), second);
  EXPECT_EQ(selections.selections().size(), 1);
}

TEST(PortfolioBacktestSelections, NormalizesToFirstOrderedValidRun)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto first = BacktestRunKey{{2, 1}, {3, 1}};
  const auto second = BacktestRunKey{{2, 1}, {4, 1}};
  auto selections = PortfolioBacktestSelections{};
  const auto runs = std::vector{first, second};

  EXPECT_EQ(selections.normalize(portfolio, runs, [](auto) { return true; }),
            first);
  selections.remember(portfolio, second);
  EXPECT_EQ(selections.normalize(portfolio, runs, [](auto) { return true; }),
            second);
}

TEST(PortfolioBacktestSelections, RemovesBacktestAssetAndPortfolioReferences)
{
  const auto first_portfolio = PortfolioStoreHandle{1, 1};
  const auto second_portfolio = PortfolioStoreHandle{2, 1};
  const auto first = BacktestRunKey{{3, 1}, {5, 1}};
  const auto second = BacktestRunKey{{4, 1}, {6, 1}};
  auto selections =
   PortfolioBacktestSelections{std::vector<PortfolioBacktestSelection>{
    {first_portfolio, first}, {second_portfolio, second}}};

  selections.remove_asset(first.asset_handle);
  EXPECT_FALSE(selections.lookup(first_portfolio));
  EXPECT_TRUE(selections.lookup(second_portfolio));
  selections.remove_backtest(second.backtest_handle);
  EXPECT_FALSE(selections.lookup(second_portfolio));

  selections.remember(first_portfolio, first);
  selections.remove_portfolio(first_portfolio);
  EXPECT_FALSE(selections.lookup(first_portfolio));
}

TEST(PortfolioBacktestSelections, EmptyOrInvalidRunsHaveNoSelection)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto run = BacktestRunKey{{2, 1}, {3, 1}};
  auto selections = PortfolioBacktestSelections{};

  EXPECT_FALSE(selections.normalize(
   portfolio, std::vector<BacktestRunKey>{}, [](auto) { return true; }));
  EXPECT_FALSE(selections.normalize(
   portfolio, std::vector{run}, [](auto) { return false; }));
}
