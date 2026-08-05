#include <gtest/gtest.h>

#include <vector>

import pludux.apps.backtest.portfolio_backtest_selections;
import pludux.backtest;

namespace {

using pludux::apps::PortfolioBacktestSelection;
using pludux::apps::PortfolioBacktestSelections;
using pludux::backtest::BacktestStoreHandle;
using pludux::backtest::PortfolioStoreHandle;

TEST(PortfolioBacktestSelections, RemembersAndReplacesOneBacktestPerPortfolio)
{
  const auto portfolio = PortfolioStoreHandle{1, 2};
  const auto first = BacktestStoreHandle{3, 4};
  const auto replacement = BacktestStoreHandle{5, 6};
  auto selections = PortfolioBacktestSelections{};

  selections.remember(portfolio, first);
  EXPECT_EQ(selections.lookup(portfolio), first);

  selections.remember(portfolio, replacement);
  EXPECT_EQ(selections.lookup(portfolio), replacement);
  EXPECT_EQ(selections.selections().size(), 1U);
}

TEST(PortfolioBacktestSelections, RemembersReusedBacktestIndependently)
{
  const auto first_portfolio = PortfolioStoreHandle{1, 1};
  const auto second_portfolio = PortfolioStoreHandle{2, 1};
  const auto backtest = BacktestStoreHandle{3, 1};
  auto selections = PortfolioBacktestSelections{};

  selections.remember(first_portfolio, backtest);
  selections.remember(second_portfolio, backtest);

  EXPECT_EQ(selections.lookup(first_portfolio), backtest);
  EXPECT_EQ(selections.lookup(second_portfolio), backtest);
  EXPECT_EQ(selections.selections().size(), 2U);
}

TEST(PortfolioBacktestSelections, RemovesPortfolioAndAllBacktestReferences)
{
  const auto first_portfolio = PortfolioStoreHandle{1, 1};
  const auto second_portfolio = PortfolioStoreHandle{2, 1};
  const auto shared = BacktestStoreHandle{3, 1};
  const auto other = BacktestStoreHandle{4, 1};
  auto selections =
   PortfolioBacktestSelections{std::vector<PortfolioBacktestSelection>{
    {first_portfolio, shared}, {second_portfolio, shared}}};

  selections.remove_portfolio(first_portfolio);
  EXPECT_FALSE(selections.lookup(first_portfolio));
  EXPECT_EQ(selections.lookup(second_portfolio), shared);

  selections.remember(first_portfolio, other);
  selections.remove_backtest(shared);
  EXPECT_EQ(selections.lookup(first_portfolio), other);
  EXPECT_FALSE(selections.lookup(second_portfolio));
}

TEST(PortfolioBacktestSelections, ConstructorNormalizesDuplicatePortfolios)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto first = BacktestStoreHandle{2, 1};
  const auto replacement = BacktestStoreHandle{3, 1};
  const auto selections =
   PortfolioBacktestSelections{std::vector<PortfolioBacktestSelection>{
    {portfolio, first}, {portfolio, replacement}}};

  EXPECT_EQ(selections.lookup(portfolio), replacement);
  EXPECT_EQ(selections.selections().size(), 1U);
}

TEST(PortfolioBacktestSelections, NormalizationUsesFirstValidExecutionOrder)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto missing = BacktestStoreHandle{2, 1};
  const auto first_valid = BacktestStoreHandle{3, 1};
  const auto second_valid = BacktestStoreHandle{4, 1};
  const auto ordered = std::vector{missing, first_valid, second_valid};
  auto selections = PortfolioBacktestSelections{};

  const auto selected = selections.normalize(
   portfolio, ordered, [&](const auto handle) { return handle != missing; });

  EXPECT_EQ(selected, first_valid);
  EXPECT_EQ(selections.lookup(portfolio), first_valid);
}

TEST(PortfolioBacktestSelections, NormalizationReplacesInvalidRememberedValue)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto removed = BacktestStoreHandle{2, 1};
  const auto replacement = BacktestStoreHandle{3, 1};
  const auto ordered = std::vector{replacement};
  auto selections = PortfolioBacktestSelections{};
  selections.remember(portfolio, removed);

  EXPECT_EQ(
   selections.normalize(portfolio, ordered, [](const auto) { return true; }),
   replacement);
  EXPECT_EQ(selections.lookup(portfolio), replacement);
}

TEST(PortfolioBacktestSelections, EmptyOrEntirelyInvalidPortfolioHasNoSelection)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto missing = BacktestStoreHandle{2, 1};
  auto selections = PortfolioBacktestSelections{};
  selections.remember(portfolio, missing);

  EXPECT_FALSE(selections.normalize(portfolio,
                                    std::vector<BacktestStoreHandle>{},
                                    [](const auto) { return true; }));
  EXPECT_FALSE(selections.lookup(portfolio));

  EXPECT_FALSE(selections.normalize(
   portfolio, std::vector{missing}, [](const auto) { return false; }));
  EXPECT_FALSE(selections.lookup(portfolio));
}

} // namespace
