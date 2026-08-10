#include <gtest/gtest.h>

#include <sstream>
#include <utility>
#include <vector>

import pludux.apps.backtest.application_state;
import pludux.apps.backtest.portfolio_backtest_setup_selections;
import pludux.apps.backtest.serialization;
import pludux.backtest;

using namespace pludux;
using namespace pludux::apps;
using namespace pludux::backtest;

TEST(WatchlistStateTest, EmptyWatchlistIsSaveableButBacktestIsNotReady)
{
  auto state = ApplicationState{};
  const auto watchlist_handle = *state.add_watchlist(Watchlist{"Empty"});
  state.add_strategy(Strategy{});
  state.add_profile(Profile{});

  const auto configured =
   Backtest{"Test",
            watchlist_handle,
            StrategyPerformanceConfig{},
            BacktestSetup{state.get_strategy_handles().front(),
                          state.get_profile_handles().front()}};
  EXPECT_FALSE(state.is_backtest_ready(configured));
}

TEST(WatchlistStateTest, BacktestIsReadyWhenEveryWatchlistAssetExists)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"BTC"});
  state.add_asset(Asset{"ETH"});
  const auto watchlist_handle =
   *state.add_watchlist(Watchlist{"Crypto", state.get_asset_handles()});
  state.add_strategy(Strategy{});
  state.add_profile(Profile{});

  const auto configured =
   Backtest{"Test",
            watchlist_handle,
            StrategyPerformanceConfig{},
            BacktestSetup{state.get_strategy_handles().front(),
                          state.get_profile_handles().front()}};
  EXPECT_TRUE(state.is_backtest_ready(configured));
}

TEST(WatchlistStateTest, DeletingAssetRetainsMissingWatchlistReference)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"BTC"});
  state.add_asset(Asset{"ETH"});
  const auto first = state.get_asset_handles().front();
  const auto second = state.get_asset_handles().back();
  const auto watchlist_handle =
   *state.add_watchlist(Watchlist{"Crypto", {first, second}});

  ASSERT_TRUE(state.remove_asset(first));
  EXPECT_EQ(state.get_watchlist(watchlist_handle).asset_handles(),
            (std::vector<AssetStoreHandle>{first, second}));
}

TEST(WatchlistStateTest, ExpandsBacktestsThenAssetsThenSetups)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"BTC"});
  state.add_asset(Asset{"ETH"});
  const auto first_asset = state.get_asset_handles().front();
  const auto second_asset = state.get_asset_handles().back();
  const auto first_watchlist =
   *state.add_watchlist(Watchlist{"First", {first_asset, second_asset}});
  const auto second_watchlist =
   *state.add_watchlist(Watchlist{"Second", {second_asset}});
  const auto first_backtest =
   *state.add_backtest(Backtest{"First",
                                first_watchlist,
                                StrategyPerformanceConfig{},
                                BacktestSetup{},
                                {BacktestSetup{}, BacktestSetup{}}});
  const auto second_backtest = *state.add_backtest(Backtest{
   "Second", second_watchlist, StrategyPerformanceConfig{}, BacktestSetup{}});
  const auto portfolio = Portfolio{
   "Portfolio", 1'000.0, {}, {}, 10, 10, {}, {first_backtest, second_backtest}};

  EXPECT_EQ(state.expanded_backtest_setups(portfolio),
            (std::vector<PortfolioBacktestSetupKey>{
             {{first_backtest, first_asset}, 0},
             {{first_backtest, first_asset}, 1},
             {{first_backtest, first_asset}, 2},
             {{first_backtest, second_asset}, 0},
             {{first_backtest, second_asset}, 1},
             {{first_backtest, second_asset}, 2},
             {{second_backtest, second_asset}, 0}}));
}

TEST(WatchlistStateTest, SetupSelectionSurvivesReorderAndFallsBackAfterRemoval)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"List", {asset}});
  state.add_strategy(Strategy{});
  state.add_profile(Profile{});
  const auto setup = BacktestSetup{state.get_strategy_handles().front(),
                                   state.get_profile_handles().front()};
  const auto backtest = *state.add_backtest(
   Backtest{"Backtest", watchlist, {}, setup, {setup, setup}});
  const auto portfolio = *state.add_portfolio(
   Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {backtest}});
  const auto selected = PortfolioBacktestSetupKey{{backtest, asset}, 2};
  ASSERT_TRUE(state.select_portfolio_backtest_setup(portfolio, selected));

  auto edited = state.get_backtest(backtest);
  std::swap(edited.failsafe_setups()[0], edited.failsafe_setups()[1]);
  ASSERT_TRUE(state.update_backtest(backtest, edited));
  EXPECT_EQ(state.selected_portfolio_backtest_setup(), selected);

  edited.failsafe_setups().pop_back();
  ASSERT_TRUE(state.update_backtest(backtest, std::move(edited)));
  EXPECT_EQ(state.selected_portfolio_backtest_setup(),
            (PortfolioBacktestSetupKey{{backtest, asset}, 0}));
}

TEST(WatchlistStateTest, MissingReferencesRemainAsDisabledSetupRows)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"List", {asset}});
  state.add_strategy(Strategy{});
  state.add_profile(Profile{});
  const auto setup = BacktestSetup{state.get_strategy_handles().front(),
                                   state.get_profile_handles().front()};
  const auto backtest =
   *state.add_backtest(Backtest{"Backtest", watchlist, {}, setup});
  const auto portfolio_handle = *state.add_portfolio(
   Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {backtest}});
  const auto selected = PortfolioBacktestSetupKey{{backtest, asset}, 0};
  ASSERT_TRUE(
   state.select_portfolio_backtest_setup(portfolio_handle, selected));

  ASSERT_TRUE(state.remove_asset(asset));
  EXPECT_EQ(
   state.expanded_backtest_setups(state.get_portfolio(portfolio_handle)),
   (std::vector{selected}));
  EXPECT_FALSE(state.selected_portfolio_backtest_setup());

  ASSERT_TRUE(state.remove_backtest(backtest));
  EXPECT_EQ(
   state.expanded_backtest_setups(state.get_portfolio(portfolio_handle)),
   (std::vector<PortfolioBacktestSetupKey>{{{backtest, {}}, 0}}));
  EXPECT_FALSE(state.selected_portfolio_backtest_setup());
}

TEST(WatchlistStateTest, MissingSetupDependencyFallsBackToFirstValidRow)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"List", {asset}});
  state.add_strategy(Strategy{});
  state.add_strategy(Strategy{});
  state.add_profile(Profile{});
  const auto main = BacktestSetup{state.get_strategy_handles()[0],
                                  state.get_profile_handles().front()};
  const auto failsafe = BacktestSetup{state.get_strategy_handles()[1],
                                      state.get_profile_handles().front()};
  const auto backtest =
   *state.add_backtest(Backtest{"Backtest", watchlist, {}, main, {failsafe}});
  const auto portfolio = *state.add_portfolio(
   Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {backtest}});
  const auto selected = PortfolioBacktestSetupKey{{backtest, asset}, 1};
  ASSERT_TRUE(state.select_portfolio_backtest_setup(portfolio, selected));

  ASSERT_TRUE(state.remove_strategy(state.get_strategy_handles()[1]));
  EXPECT_EQ(state.selected_portfolio_backtest_setup(),
            (PortfolioBacktestSetupKey{{backtest, asset}, 0}));
  EXPECT_FALSE(state.select_portfolio_backtest_setup(portfolio, selected));
}

TEST(WatchlistStateTest, PersistsWatchlistStoreAndDocumentOrder)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"BTC"});
  const auto asset = state.get_asset_handles().front();
  state.add_watchlist(Watchlist{"Crypto", {asset}});

  auto output = std::stringstream{};
  save_application_state_json(output, state);
  auto input = std::stringstream{output.str()};
  const auto loaded = load_application_state_json(input);

  ASSERT_EQ(loaded.get_watchlist_handles().size(), 1);
  const auto& watchlist =
   loaded.get_watchlist(loaded.get_watchlist_handles().front());
  EXPECT_EQ(watchlist.name(), "Crypto");
  EXPECT_EQ(watchlist.asset_handles(), (std::vector<AssetStoreHandle>{asset}));
}
