#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include "../src/scope_results.hpp"

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

TEST(PortfolioStrategySelections, PreservesPortfolioScopeAndValidStrategy)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto run = BacktestRunKey{{2, 1}, {3, 1}};
  const auto main = PortfolioStrategyKey{run, 0};
  const auto failsafe = PortfolioStrategyKey{run, 1};
  auto selections = PortfolioStrategySelections{};
  const auto strategies = std::vector{main, failsafe};

  EXPECT_FALSE(
   selections.normalize(portfolio, strategies, [](auto) { return true; }));
  selections.remember(portfolio, failsafe);
  EXPECT_EQ(selections.normalize(portfolio, strategies, [](auto) { return true; }),
            failsafe);
}

TEST(PortfolioStrategySelections, CollapsedContextRetainsItsRememberedChild)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto child = PortfolioStrategyKey{{{2, 1}, {3, 1}}, 0};
  auto selections = PortfolioStrategySelections{
   std::vector<PortfolioStrategySelection>{{portfolio, child, false}}};
  EXPECT_FALSE(selections.lookup(portfolio));
  EXPECT_EQ(selections.remembered(portfolio), child);
  EXPECT_FALSE(selections.normalize(
   portfolio, std::vector{child}, [](auto) { return true; }));
  EXPECT_EQ(selections.remembered(portfolio), child);
  selections.remember(portfolio, child);
  EXPECT_EQ(selections.lookup(portfolio), child);
  selections.deactivate(portfolio);
  EXPECT_FALSE(selections.lookup(portfolio));
  EXPECT_FALSE(selections.normalize(
   portfolio, std::vector{child}, [](auto) { return false; }));
  EXPECT_FALSE(selections.remembered(portfolio));
}

TEST(PortfolioStrategySelections,
     ReturnsToPortfolioScopeAfterRememberedStrategyIsRemoved)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto run = BacktestRunKey{{2, 1}, {3, 1}};
  const auto main = PortfolioStrategyKey{run, 0};
  const auto removed_failsafe = PortfolioStrategyKey{run, 2};
  auto selections = PortfolioStrategySelections{};
  selections.remember(portfolio, removed_failsafe);

  EXPECT_FALSE(selections.normalize(
   portfolio, std::vector{main}, [](auto) { return true; }));
  EXPECT_FALSE(selections.lookup(portfolio));
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

TEST(ScopeResults, PortfolioTradesUseCloseTimeAndDeterministicIdentity)
{
  using pludux::apps::portfolio_trade_before;
  using pludux::apps::ScopeTradeOrder;
  EXPECT_TRUE(
   portfolio_trade_before({true, 10, 0, 0, 1}, {false, 100, 0, 0, 1}));
  EXPECT_TRUE(portfolio_trade_before({true, 20, 0, 0, 1}, {true, 10, 0, 0, 2}));
  EXPECT_TRUE(
   portfolio_trade_before({false, 100, 0, 0, 1}, {false, 90, 0, 0, 2}));
  EXPECT_TRUE(
   portfolio_trade_before({false, 100, 0, 1, 1}, {false, 100, 1, 0, 1}));
  EXPECT_TRUE(
   portfolio_trade_before({false, 100, 0, 0, 1}, {false, 100, 0, 1, 1}));
  EXPECT_TRUE(
   portfolio_trade_before({false, 100, 0, 0, 1}, {false, 100, 0, 0, 2}));
  EXPECT_FALSE(
   portfolio_trade_before({false, 100, 0, 0, 1}, {false, 100, 0, 0, 1}));
}

TEST(ScopeResults, SummaryIncludesPyramidsPartialExitsFeesAndStrategyOwnership)
{
  using Type = TradeEvent::Type;
  auto timeline = BacktestTimeline{};
  // Entry fees are embedded in investment and therefore average cost.
  auto entry =
   TradeEvent{1, 1, 0, Type::entry, 10, 10, 10, 1, 0, 0, 0, 10, 101, 10.1};
  auto pyramid = TradeEvent{
   1, 2, 1, Type::scale_in, 20, 10, 10, 1, 10, 101, 10.1, 20, 202, 10.1};
  auto partial = TradeEvent{
   1, 3, 2, Type::scale_out, 30, 12, 5, 1, 20, 202, 10.1, 15, 151.5, 10.1};
  auto final = TradeEvent{
   1, 4, 3, Type::exit_signal, 40, 11, 15, 1, 15, 151.5, 10.1, 0, 0, 0};
  for(auto* event : {&entry, &pyramid, &partial, &final})
    event->strategy_index(1);
  auto other =
   TradeEvent{1, 5, 0, Type::entry, 10, 50, 1, 0, 0, 0, 0, 1, 50, 50};
  auto rejected = TradeEvent{0,
                             6,
                             0,
                             Type::rejected_insufficient_cash,
                             10,
                             100,
                             100,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0};
  rejected.strategy_index(1);
  timeline.append({.trade_events = {entry, other, rejected}});
  timeline.append({.trade_events = {pyramid, partial}});
  auto summary = pludux::apps::summarize_single_asset(timeline, 1);
  EXPECT_NEAR(summary.realized_pnl, 8.5, 1e-10);
  EXPECT_EQ(summary.closed_count, 0);
  EXPECT_DOUBLE_EQ(summary.deployed_capital, 0.0);
  timeline.append({.trade_events = {final}});
  summary = pludux::apps::summarize_single_asset(timeline, 1);
  EXPECT_NEAR(summary.realized_pnl, 21.0, 1e-10);
  EXPECT_NEAR(summary.completed_pnl, 21.0, 1e-10);
  EXPECT_DOUBLE_EQ(summary.deployed_capital, 202.0);
  EXPECT_EQ(summary.closed_count, 1);
  EXPECT_EQ(summary.wins, 1);
  EXPECT_EQ(summary.closed_duration, 30);
  EXPECT_EQ(pludux::apps::summarize_single_asset(timeline, 0).closed_count, 0);
}

TEST(ScopeResults, EmptyTimelineHasNoTradeStatistics)
{
  const auto summary =
   pludux::apps::summarize_single_asset(BacktestTimeline{}, 0);
  EXPECT_EQ(summary.closed_count, 0);
  EXPECT_EQ(summary.open_count, 0);
  EXPECT_DOUBLE_EQ(summary.completed_pnl, 0.0);
  EXPECT_DOUBLE_EQ(summary.deployed_capital, 0.0);
}

TEST(ScopeResults, SingleAssetPnlUsesActualFillsAndMarksForTheSelectedSetup)
{
  using pludux::apps::single_asset_pnl;
  using Type = TradeEvent::Type;
  auto entry =
   TradeEvent{1, 1, 0, Type::entry, 10, 10, 10, 1, 0, 0, 0, 10, 101, 10.1};
  auto pyramid = TradeEvent{
   1, 2, 1, Type::scale_in, 20, 10, 10, 1, 10, 101, 10.1, 20, 202, 10.1};
  auto partial = TradeEvent{
   1, 3, 2, Type::scale_out, 30, 12, 5, 1, 20, 202, 10.1, 15, 151.5, 10.1};
  auto final = TradeEvent{
   1, 4, 3, Type::exit_signal, 40, 11, 15, 1, 15, 151.5, 10.1, 0, 0, 0};
  const auto other_entry =
   TradeEvent{2, 5, 0, Type::entry, 40, 50, -1, 1, 0, 0, 0, -1, -49, 49};
  const auto other_exit =
   TradeEvent{2, 6, 1, Type::exit_signal, 50, 46, -1, 1, -1, -49, 49, 0, 0, 0};
  auto recovery_entry =
   TradeEvent{3, 7, 0, Type::entry, 50, 10, 10, 1, 0, 0, 0, 10, 101, 10.1};
  auto recovery_exit = TradeEvent{
   3, 8, 1, Type::take_profit, 50, 14, 10, 1, 10, 101, 10.1, 0, 0, 0};
  for(auto* event :
      {&entry, &pyramid, &partial, &final, &recovery_entry, &recovery_exit}) {
    event->strategy_index(1);
  }

  auto timeline = BacktestTimeline{};
  timeline.append({});
  timeline.append(
   {.trade_events = {entry},
    .open_position = OpenPositionSnapshot{1, 1, 10, 10, 10, 10, 101, 10, 1}});
  timeline.append(
   {.trade_events = {pyramid},
    .open_position = OpenPositionSnapshot{1, 1, 10, 20, 12, 20, 202, 10, 2}});
  timeline.append({.trade_events = {partial},
                   .open_position = OpenPositionSnapshot{
                    1, 1, 10, 30, 11, 15, 151.5, 10, 1.5}});
  timeline.append(
   {.trade_events = {final, other_entry},
    .open_position = OpenPositionSnapshot{2, 0, 40, 40, 45, -1, -49, 1, 1}});
  timeline.append(
   {.trade_events = {other_exit, recovery_entry, recovery_exit}});

  const auto points = single_asset_pnl(timeline, 1, 1'000.0);
  ASSERT_EQ(points.size(), timeline.size());
  const auto expected_amount = std::vector{0.0, -1.0, 38.0, 22.0, 21.0, 59.0};
  const auto expected_drawdown = std::vector{0.0, 0.1, 0.0, 1.6, 1.7, 0.0};
  for(auto index = std::size_t{}; index < points.size(); ++index) {
    SCOPED_TRACE(index);
    EXPECT_NEAR(points[index].amount, expected_amount[index], 1e-10);
    EXPECT_NEAR(points[index].percentage, expected_amount[index] / 10.0, 1e-10);
    EXPECT_NEAR(
     points[index].drawdown_percentage, expected_drawdown[index], 1e-10);
  }
  const auto other = single_asset_pnl(timeline, 0, 1'000.0);
  ASSERT_EQ(other.size(), timeline.size());
  EXPECT_DOUBLE_EQ(other[3].amount, 0.0);
  EXPECT_DOUBLE_EQ(other[4].amount, 4.0);
  EXPECT_DOUBLE_EQ(other[5].amount, 2.0);
  EXPECT_NEAR(other[5].drawdown_percentage, 0.2, 1e-10);
}

TEST(ScopeResults, EquityPercentageUsesInitialCapitalAsTheBaseline)
{
  using pludux::apps::percentage_of_initial_capital;
  EXPECT_DOUBLE_EQ(percentage_of_initial_capital(10'000.0, 10'000.0), 100.0);
  EXPECT_DOUBLE_EQ(percentage_of_initial_capital(10'250.0, 10'000.0), 102.5);
  EXPECT_DOUBLE_EQ(percentage_of_initial_capital(9'750.0, 10'000.0), 97.5);
  EXPECT_TRUE(std::isnan(percentage_of_initial_capital(100.0, 0.0)));
}

TEST(ScopeResults, SingleAssetPnlHasNoPercentagesWithoutInitialCapital)
{
  using pludux::apps::single_asset_pnl;
  auto timeline = BacktestTimeline{};
  EXPECT_TRUE(single_asset_pnl(timeline, 1, 0.0).empty());
  timeline.append(
   {.open_position = OpenPositionSnapshot{1, 1, 10, 30, 12, 5, 50.5, 10, 0.5}});
  const auto points = single_asset_pnl(timeline, 1, 0.0);
  ASSERT_EQ(points.size(), 1);
  EXPECT_NEAR(points[0].amount, 9.5, 1e-10);
  EXPECT_TRUE(std::isnan(points[0].percentage));
  EXPECT_TRUE(std::isnan(points[0].drawdown_percentage));
}

TEST(ScopeResults, OpenPositionKeepsOwnerAndRemainingFeeAdjustedPnl)
{
  auto timeline = BacktestTimeline{};
  timeline.append(
   {.open_position = OpenPositionSnapshot{1, 1, 10, 30, 12, 5, 50.5, 10, 0.5}});
  const auto owner = pludux::apps::summarize_single_asset(timeline, 1);
  EXPECT_EQ(owner.open_count, 1);
  EXPECT_NEAR(owner.unrealized_pnl, 9.5, 1e-10);
  EXPECT_EQ(owner.open_duration, 20);
  EXPECT_EQ(owner.closed_count, 0);
  EXPECT_DOUBLE_EQ(owner.deployed_capital, 0.0);
  EXPECT_EQ(pludux::apps::summarize_single_asset(timeline, 0).open_count, 0);
}

TEST(ScopeResults, OpenPositionMarkerFollowsLatestPositionOwner)
{
  using pludux::apps::has_open_position;
  auto timeline = BacktestTimeline{};
  EXPECT_FALSE(has_open_position(timeline, 0));
  timeline.append(
   {.open_position = OpenPositionSnapshot{1, 1, 10, 30, 12, 5, 50.5, 10, 0.5}});
  EXPECT_TRUE(has_open_position(timeline, 1));
  EXPECT_FALSE(has_open_position(timeline, 0));
  timeline.append({});
  EXPECT_FALSE(has_open_position(timeline, 1));
}

TEST(ScopeResults, AggregateEventsKeepExecutionOrderAndScopeFilters)
{
  using Type = TradeEvent::Type;
  auto first_timeline = BacktestTimeline{};
  auto second_timeline = BacktestTimeline{};
  auto early =
   TradeEvent{1, 1, 0, Type::entry, 10, 10, 1, 0, 0, 0, 0, 1, 10, 10};
  auto later =
   TradeEvent{1, 2, 1, Type::exit_signal, 20, 11, 1, 0, 1, 10, 10, 0, 0, 0};
  later.strategy_index(1);
  first_timeline.append({.trade_events = {early, later}});
  second_timeline.append({.trade_events = {later}});
  const auto first = BacktestRunKey{{1, 1}, {2, 1}};
  const auto second = BacktestRunKey{{3, 1}, {4, 1}};
  auto results = PortfolioResults{
   {},
   {BacktestResults{first.system_handle, first.asset_handle, first_timeline, 2},
    BacktestResults{
     second.system_handle, second.asset_handle, second_timeline, 2}}};
  results.append_events({{0, 0, 0}, {1, 0, 0}, {0, 0, 1}});
  auto selected = std::optional<PortfolioStrategyKey>{};
  auto type = std::optional<Type>{};
  EXPECT_EQ(pludux::apps::scope_event_indices(results, selected, type),
            (std::vector<std::size_t>{1, 2, 0}));
  type = Type::entry;
  EXPECT_EQ(pludux::apps::scope_event_indices(results, selected, type),
            (std::vector<std::size_t>{0}));
  type.reset();
  selected = PortfolioStrategyKey{first, 1};
  EXPECT_EQ(pludux::apps::scope_event_indices(results, selected, type),
            (std::vector<std::size_t>{2}));
}

TEST(ScopeResults, ShortTradeIncludesFeesAndCountsLosses)
{
  using Type = TradeEvent::Type;
  auto timeline = BacktestTimeline{};
  timeline.append(
   {.trade_events = {
     TradeEvent{1, 1, 0, Type::entry, 10, 10, -10, 1, 0, 0, 0, -10, -99, 9.9},
     TradeEvent{
      1, 2, 1, Type::exit_signal, 20, 11, -10, 1, -10, -99, 9.9, 0, 0, 0}}});
  const auto summary = pludux::apps::summarize_single_asset(timeline, 0);
  EXPECT_NEAR(summary.completed_pnl, -12.0, 1e-10);
  EXPECT_DOUBLE_EQ(summary.deployed_capital, 99.0);
  EXPECT_NEAR(summary.losses, 12.0, 1e-10);
  EXPECT_EQ(summary.losses_count, 1);
  EXPECT_EQ(summary.wins, 0);
}
