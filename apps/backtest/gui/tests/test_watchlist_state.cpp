#include <gtest/gtest.h>

#include <sstream>
#include <utility>
#include <vector>

import pludux.apps.backtest.application_state;
import pludux.apps.backtest.portfolio_strategy_selections;
import pludux.apps.backtest.serialization;
import pludux.backtest;

namespace {

using pludux::apps::ApplicationState;
using pludux::backtest::Asset;
using pludux::backtest::Portfolio;
using pludux::backtest::Profile;
using pludux::backtest::Strategy;
using pludux::backtest::System;
using pludux::backtest::Watchlist;

auto add_ready_strategy(ApplicationState& state, std::string name = "Strategy")
 -> pludux::backtest::StrategyStoreHandle
{
  state.add_model(pludux::backtest::Model{});
  state.add_profile(Profile{});
  state.add_strategy(Strategy{std::move(name), state.get_model_handles().back(),
                              state.get_profile_handles().back()});
  return state.get_strategy_handles().back();
}

TEST(WatchlistState, EmptyWatchlistIsSaveableButSystemIsNotReady)
{
  auto state = ApplicationState{};
  const auto watchlist = *state.add_watchlist(Watchlist{"Empty"});
  const auto strategy = add_ready_strategy(state);
  EXPECT_FALSE(state.is_system_ready(System{"Test", watchlist, {}, strategy}));
}

TEST(WatchlistState, SystemIsReadyWhenEveryWatchlistAssetExists)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"BTC"});
  state.add_asset(Asset{"ETH"});
  const auto watchlist = *state.add_watchlist(Watchlist{"Crypto", state.get_asset_handles()});
  const auto strategy = add_ready_strategy(state);
  EXPECT_TRUE(state.is_system_ready(System{"Test", watchlist, {}, strategy}));
}

TEST(WatchlistState, DeletingAssetRetainsMissingWatchlistReference)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"BTC"});
  state.add_asset(Asset{"ETH"});
  const auto first = state.get_asset_handles().front();
  const auto second = state.get_asset_handles().back();
  const auto watchlist = *state.add_watchlist(Watchlist{"Crypto", {first, second}});
  ASSERT_TRUE(state.remove_asset(first));
  EXPECT_EQ(state.get_watchlist(watchlist).asset_handles(),
            (std::vector<pludux::backtest::AssetStoreHandle>{first, second}));
}

TEST(WatchlistState, ExpandsSystemsThenAssetsThenStrategies)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"BTC"});
  state.add_asset(Asset{"ETH"});
  const auto first_asset = state.get_asset_handles().front();
  const auto second_asset = state.get_asset_handles().back();
  const auto first_watchlist = *state.add_watchlist(Watchlist{"First", {first_asset, second_asset}});
  const auto second_watchlist = *state.add_watchlist(Watchlist{"Second", {second_asset}});
  const auto main = add_ready_strategy(state, "Main");
  const auto first_failsafe = add_ready_strategy(state, "First failsafe");
  const auto second_failsafe = add_ready_strategy(state, "Second failsafe");
  const auto first_system = *state.add_system(System{"First", first_watchlist, {}, main,
                                                      {{first_failsafe}, {second_failsafe}}});
  const auto second_system = *state.add_system(System{"Second", second_watchlist, {}, main});
  const auto portfolio = Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {first_system, second_system}};
  EXPECT_EQ(state.expanded_system_strategies(portfolio),
            (std::vector<pludux::apps::PortfolioStrategyKey>{
             {{first_system, first_asset}, 0}, {{first_system, first_asset}, 1},
             {{first_system, first_asset}, 2}, {{first_system, second_asset}, 0},
             {{first_system, second_asset}, 1}, {{first_system, second_asset}, 2},
             {{second_system, second_asset}, 0}}));
}

TEST(ApplicationState, MissingStrategyRemainsReferencedAndInvalidatesPortfolio)
{
  auto state = pludux::apps::ApplicationState{};
  state.add_model(pludux::backtest::Model{});
  state.add_profile(pludux::backtest::Profile{});
  state.add_asset(pludux::backtest::Asset{"Asset"});
  const auto watchlist = *state.add_watchlist(
   pludux::backtest::Watchlist{"List", {state.get_asset_handles().front()}});
  state.add_strategy(pludux::backtest::Strategy{
   "Strategy", state.get_model_handles().front(), state.get_profile_handles().front()});
  const auto strategy = state.get_strategy_handles().front();
  const auto system = *state.add_system(
   pludux::backtest::System{"System", watchlist, {}, strategy});
  const auto portfolio = *state.add_portfolio(pludux::backtest::Portfolio{
   "Portfolio", 1'000.0, {}, {}, 10, 10, {}, {system}});

  auto timeline = pludux::backtest::PortfolioTimeline{};
  timeline.append({.timestamp = 1, .capital = 42.0});
  ASSERT_TRUE(state.update_portfolio_results(
   portfolio, pludux::backtest::PortfolioResults{std::move(timeline), {}}));
  ASSERT_TRUE(state.remove_strategy(strategy));

  EXPECT_EQ(state.get_system(system).main_strategy_handle(), strategy);
  EXPECT_FALSE(state.is_portfolio_ready(state.get_portfolio(portfolio)));
  EXPECT_TRUE(state.get_portfolio_results(portfolio).timeline().empty());
}

TEST(ApplicationState, MissingSystemRemainsReferencedAndInvalidatesPortfolio)
{
  auto state = pludux::apps::ApplicationState{};
  state.add_model(pludux::backtest::Model{});
  state.add_profile(pludux::backtest::Profile{});
  state.add_asset(pludux::backtest::Asset{"Asset"});
  const auto watchlist = *state.add_watchlist(
   pludux::backtest::Watchlist{"List", {state.get_asset_handles().front()}});
  state.add_strategy(pludux::backtest::Strategy{
   "Strategy", state.get_model_handles().front(), state.get_profile_handles().front()});
  const auto system = *state.add_system(pludux::backtest::System{
   "System", watchlist, {}, state.get_strategy_handles().front()});
  const auto portfolio = *state.add_portfolio(pludux::backtest::Portfolio{
   "Portfolio", 1'000.0, {}, {}, 10, 10, {}, {system}});
  auto timeline = pludux::backtest::PortfolioTimeline{};
  timeline.append({.timestamp = 1, .capital = 42.0});
  ASSERT_TRUE(state.update_portfolio_results(
   portfolio, pludux::backtest::PortfolioResults{std::move(timeline), {}}));

  ASSERT_TRUE(state.remove_system(system));

  EXPECT_EQ(state.get_portfolio(portfolio).system_handles().front(), system);
  EXPECT_FALSE(state.is_portfolio_ready(state.get_portfolio(portfolio)));
  EXPECT_TRUE(state.get_portfolio_results(portfolio).timeline().empty());
}

TEST(StrategyEditor, ChangingModelResetsInputsToNewModelDefaults)
{
  auto first_series = pludux::OrderedNamedRegistry<pludux::ErasedNode<
   pludux::ErasedSeriesMethodContext>>{};
  first_series.set("first", pludux::NumericInputNode{
   "First", pludux::NumericInputNode::ValueRepresentation::Decimal, 2.5});
  auto second_series = pludux::OrderedNamedRegistry<pludux::ErasedNode<
   pludux::ErasedSeriesMethodContext>>{};
  second_series.set("second", pludux::NumericInputNode{
   "Second", pludux::NumericInputNode::ValueRepresentation::UnsignedInteger, 14.0});
  const auto first = pludux::backtest::Model{
   "First", std::move(first_series), {}, {}, {}};
  const auto second = pludux::backtest::Model{
   "Second", std::move(second_series), {}, {}, {}};
  auto strategy = pludux::backtest::Strategy{};

  pludux::backtest::assign_strategy_model(
   strategy, pludux::backtest::ModelStoreHandle{1, 1}, first);
  auto overridden_inputs = strategy.inputs();
  overridden_inputs.front().value(99.0);
  strategy.inputs(std::move(overridden_inputs));
  pludux::backtest::assign_strategy_model(
   strategy, pludux::backtest::ModelStoreHandle{2, 1}, second);

  ASSERT_EQ(strategy.inputs().size(), 1U);
  EXPECT_EQ(strategy.inputs().front().label(), "Second");
  EXPECT_DOUBLE_EQ(strategy.inputs().front().value(), 14.0);
  EXPECT_EQ(strategy.model_handle(), (pludux::backtest::ModelStoreHandle{2, 1}));
}

TEST(WatchlistState, SetupSelectionSurvivesReorderAndFallsBackAfterRemoval)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"List", {asset}});
  const auto main = add_ready_strategy(state, "Main");
  const auto first = add_ready_strategy(state, "First");
  const auto second = add_ready_strategy(state, "Second");
  const auto system = *state.add_system(System{"System", watchlist, {}, main, {{first}, {second}}});
  const auto portfolio = *state.add_portfolio(Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {system}});
  const auto selected = pludux::apps::PortfolioStrategyKey{{system, asset}, 2};
  ASSERT_TRUE(state.select_portfolio_strategy(portfolio, selected));
  auto edited = state.get_system(system);
  std::swap(edited.failsafe_strategies()[0], edited.failsafe_strategies()[1]);
  ASSERT_TRUE(state.update_system(system, edited));
  EXPECT_EQ(state.selected_portfolio_strategy(), selected);
  edited.failsafe_strategies().pop_back();
  ASSERT_TRUE(state.update_system(system, std::move(edited)));
  EXPECT_EQ(state.selected_portfolio_strategy(),
            (pludux::apps::PortfolioStrategyKey{{system, asset}, 0}));
}

TEST(WatchlistState, MissingReferencesRemainAsDisabledStrategyRows)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"List", {asset}});
  const auto strategy = add_ready_strategy(state);
  const auto system = *state.add_system(System{"System", watchlist, {}, strategy});
  const auto portfolio = *state.add_portfolio(Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {system}});
  const auto selected = pludux::apps::PortfolioStrategyKey{{system, asset}, 0};
  ASSERT_TRUE(state.select_portfolio_strategy(portfolio, selected));
  ASSERT_TRUE(state.remove_asset(asset));
  EXPECT_EQ(state.expanded_system_strategies(state.get_portfolio(portfolio)), (std::vector{selected}));
  EXPECT_FALSE(state.selected_portfolio_strategy());
  ASSERT_TRUE(state.remove_system(system));
  EXPECT_EQ(state.expanded_system_strategies(state.get_portfolio(portfolio)),
            (std::vector<pludux::apps::PortfolioStrategyKey>{{{system, {}}, 0}}));
  EXPECT_FALSE(state.selected_portfolio_strategy());
}

TEST(WatchlistState, MissingStrategyDependencyFallsBackToFirstValidRow)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"List", {asset}});
  const auto main = add_ready_strategy(state, "Main");
  const auto failsafe = add_ready_strategy(state, "Failsafe");
  const auto system = *state.add_system(System{"System", watchlist, {}, main, {{failsafe}}});
  const auto portfolio = *state.add_portfolio(Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {system}});
  const auto selected = pludux::apps::PortfolioStrategyKey{{system, asset}, 1};
  ASSERT_TRUE(state.select_portfolio_strategy(portfolio, selected));
  ASSERT_TRUE(state.remove_strategy(failsafe));
  EXPECT_EQ(state.selected_portfolio_strategy(),
            (pludux::apps::PortfolioStrategyKey{{system, asset}, 0}));
  EXPECT_FALSE(state.select_portfolio_strategy(portfolio, selected));
}

TEST(WatchlistState, PersistsWatchlistStoreAndReusableReferences)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"BTC"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"Crypto", {asset}});
  const auto strategy = add_ready_strategy(state, "Momentum");
  const auto system = *state.add_system(System{"Trend", watchlist, {}, strategy});
  state.select_system(system);
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  const auto json = stream.str();
  EXPECT_NE(json.find("\"watchlists\""), std::string::npos);
  EXPECT_NE(json.find("\"systems\""), std::string::npos);
  EXPECT_NE(json.find("\"strategies\""), std::string::npos);
  EXPECT_NE(json.find("\"systemStoreDataResolver\""), std::string::npos);
  EXPECT_NE(json.find("\"strategyStoreDataResolver\""), std::string::npos);
  const auto loaded = pludux::apps::load_application_state_json(stream);
  ASSERT_EQ(loaded.get_watchlist_handles().size(), 1U);
  EXPECT_EQ(loaded.get_watchlist(loaded.get_watchlist_handles().front()).name(), "Crypto");
  EXPECT_EQ(loaded.get_system(loaded.get_system_handles().front()).name(), "Trend");
  EXPECT_EQ(loaded.get_system(loaded.get_system_handles().front()).main_strategy_handle(),
            loaded.get_strategy_handles().front());
  EXPECT_EQ(loaded.selected_system_handle(), loaded.get_system_handles().front());
}

} // namespace
