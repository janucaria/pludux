#include <gtest/gtest.h>

#include <sstream>
#include <vector>

import pludux.apps.backtest.application_state;
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

  const auto configured = Backtest{"Test",
                                   watchlist_handle,
                                   state.get_strategy_handles().front(),
                                   state.get_profile_handles().front()};
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

  const auto configured = Backtest{"Test",
                                   watchlist_handle,
                                   state.get_strategy_handles().front(),
                                   state.get_profile_handles().front()};
  EXPECT_TRUE(state.is_backtest_ready(configured));
}

TEST(WatchlistStateTest, DeletingAssetPrunesWatchlists)
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
            (std::vector<AssetStoreHandle>{second}));
}

TEST(WatchlistStateTest, ExpandsBacktestsThenOrderedAssets)
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
   *state.add_backtest(Backtest{"First", first_watchlist, {}, {}});
  const auto second_backtest =
   *state.add_backtest(Backtest{"Second", second_watchlist, {}, {}});
  const auto portfolio = Portfolio{"Portfolio",
                                   1'000.0,
                                   {},
                                   {},
                                   10,
                                   10,
                                   {},
                                   InsufficientCashPolicy::Reject,
                                   {first_backtest, second_backtest}};

  EXPECT_EQ(state.expanded_backtest_runs(portfolio),
            (std::vector<BacktestRunKey>{{first_backtest, first_asset},
                                         {first_backtest, second_asset},
                                         {second_backtest, second_asset}}));
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
