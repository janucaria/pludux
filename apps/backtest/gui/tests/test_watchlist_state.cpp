#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

import pludux.apps.backtest.application_state;
import pludux.apps.backtest.load_asset_csv_action;
import pludux.apps.backtest.portfolio_strategy_selections;
import pludux.apps.backtest.serialization;
import pludux.backtest;

namespace {

using pludux::apps::ApplicationState;
using pludux::backtest::Asset;
using pludux::backtest::Portfolio;
using pludux::backtest::Profile;
using pludux::backtest::Strategy;
using pludux::backtest::StrategySetup;
using pludux::backtest::System;
using pludux::backtest::Watchlist;

auto add_strategy(ApplicationState& state, std::string name = "Strategy")
 -> pludux::backtest::StrategyStoreHandle
{
  auto strategy = Strategy{};
  strategy.name(std::move(name));
  state.add_strategy(std::move(strategy));
  return state.get_strategy_handles().back();
}

auto add_profile(ApplicationState& state)
 -> pludux::backtest::ProfileStoreHandle
{
  state.add_profile(Profile{});
  return state.get_profile_handles().back();
}

TEST(WatchlistState, EmptyWatchlistIsSaveableButSystemIsNotReady)
{
  auto state = ApplicationState{};
  const auto watchlist = *state.add_watchlist(Watchlist{"Empty"});
  const auto strategy = add_strategy(state);
  EXPECT_FALSE(state.is_system_ready(
   System{"Test", watchlist, add_profile(state), {}, StrategySetup{strategy}}));
}

TEST(WatchlistState, SystemIsReadyWhenEveryWatchlistAssetExists)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"BTC"});
  state.add_asset(Asset{"ETH"});
  const auto watchlist =
   *state.add_watchlist(Watchlist{"Crypto", state.get_asset_handles()});
  const auto strategy = add_strategy(state);
  EXPECT_TRUE(state.is_system_ready(
   System{"Test", watchlist, add_profile(state), {}, StrategySetup{strategy}}));
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

TEST(AssetImport, DesktopAndBrowserImportsMatchOnTheCompleteFilename)
{
  auto state = ApplicationState{};
  const auto path =
   std::filesystem::path{PLUDUX_BACKTEST_SAMPLE_DIR} / "random_sample.csv";
  pludux::apps::LoadAssetCsvAction{path}(state);
  const auto handles = state.get_asset_handles();
  ASSERT_EQ(handles.size(), 1u);
  EXPECT_EQ(state.get_asset(handles.front()).name(), "random_sample.csv");

  pludux::apps::LoadAssetCsvAction{"random_sample.csv", "Date,Close\n1,42\n"}(
   state);
  EXPECT_EQ(state.get_asset_handles(), handles);
  EXPECT_EQ(state.get_asset(handles.front()).size(), 1u);
}

TEST(WatchlistState, ReimportReplacesHistoryAndPreservesAssetBinding)
{
  auto state = ApplicationState{};
  auto first = Asset{"BTC"};
  auto original_csv = std::istringstream{"Date,Last\n1,10\n2,20\n"};
  pludux::update_asset_from_csv(first, original_csv);
  auto fields = first.field_resolver();
  fields.close_field("Last");
  first.field_resolver(std::move(fields));
  state.import_asset(std::move(first));
  state.import_asset(Asset{"ETH"});
  const auto handles = state.get_asset_handles();
  ASSERT_EQ(handles.size(), 2u);
  const auto watchlist = *state.add_watchlist(Watchlist{"Crypto", handles});

  auto replacement = Asset{"BTC"};
  auto updated_csv = std::istringstream{"Time,Last\n3,42\n"};
  pludux::update_asset_from_csv(replacement, updated_csv);
  state.import_asset(std::move(replacement));

  EXPECT_EQ(state.get_asset_handles(), handles);
  EXPECT_EQ(state.get_watchlist(watchlist).asset_handles(), handles);
  const auto& asset = state.get_asset(handles.front());
  ASSERT_EQ(asset.size(), 1u);
  EXPECT_EQ(asset.history().field_data().at("Last")[0], 42);
  EXPECT_EQ(asset.field_resolver().datetime_field(), "Time");
  EXPECT_EQ(asset.field_resolver().close_field(), "Last");
  EXPECT_EQ(state.get_asset(handles.back()).name(), "ETH");
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
  const auto main = add_strategy(state, "Main");
  const auto first_failsafe = add_strategy(state, "First failsafe");
  const auto second_failsafe = add_strategy(state, "Second failsafe");
  const auto first_system = *state.add_system(
   System{"First",
          first_watchlist,
          add_profile(state),
          {},
          StrategySetup{main},
          {{StrategySetup{first_failsafe}}, {StrategySetup{second_failsafe}}}});
  const auto second_system = *state.add_system(System{"Second", second_watchlist, add_profile(state), {}, StrategySetup{main}});
  const auto portfolio = Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {first_system, second_system}};
  EXPECT_EQ(state.expanded_system_strategies(portfolio),
            (std::vector<pludux::apps::PortfolioStrategyKey>{
             {{first_system, first_asset}, 0}, {{first_system, first_asset}, 1},
             {{first_system, first_asset}, 2}, {{first_system, second_asset}, 0},
             {{first_system, second_asset}, 1}, {{first_system, second_asset}, 2},
             {{second_system, second_asset}, 0}}));
}

TEST(ExternalReferences, PersistsDifferentBindingsForTheSameStrategy)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"BBCA"});
  state.add_asset(Asset{"IHSG", {{"Datetime", {1}}, {"Close", {7100}}}});
  state.add_asset(Asset{"Other index", {{"Datetime", {1}}, {"Close", {6800}}}});
  const auto bbca = state.get_asset_handles()[0];
  const auto ihsg = state.get_asset_handles()[1];
  const auto other = state.get_asset_handles()[2];
  const auto watchlist = *state.add_watchlist(Watchlist{"BBCA only", {bbca}});
  auto sample = std::ifstream{
   std::filesystem::path{PLUDUX_BACKTEST_SAMPLE_DIR} / "bbca_ihsg_filter.json"};
  ASSERT_TRUE(sample);
  state.add_strategy(pludux::backtest::parse_strategy("Market filter", sample));
  const auto strategy = state.get_strategy_handles().back();
  const auto profile = add_profile(state);
  auto first = System{"First", watchlist, profile, {}, StrategySetup{strategy}};
  first.external_bindings()["market"] = ihsg;
  auto second = first;
  second.name("Second");
  second.external_bindings()["market"] = other;
  EXPECT_FALSE(first.equivalent_rules(second));
  const auto first_handle = *state.add_system(std::move(first));
  const auto second_handle = *state.add_system(std::move(second));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  const auto restored = pludux::apps::load_application_state_json(stream);

  const auto& first_system = restored.get_system(first_handle);
  const auto& second_system = restored.get_system(second_handle);
  EXPECT_EQ(first_system.external_bindings().at("market"), ihsg);
  EXPECT_EQ(second_system.external_bindings().at("market"), other);
  EXPECT_TRUE(restored.is_system_ready(first_system));
  EXPECT_TRUE(restored.is_system_ready(second_system));
  EXPECT_DOUBLE_EQ(
   restored.compile_external_data(first_system)->value("market", "CLOSE", 1),
   7100);
  EXPECT_DOUBLE_EQ(
   restored.compile_external_data(second_system)->value("market", "CLOSE", 1),
   6800);
  EXPECT_EQ(restored.get_watchlist(watchlist).asset_handles(),
            (std::vector{bbca}));
}

TEST(ExternalReferences, ValidatesAndInvalidatesReferencesOutsideWatchlist)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"BBCA"});
  state.add_asset(Asset{"IHSG", {{"Datetime", {1}}, {"Close", {7100}}}});
  state.add_asset(Asset{"Without close", {{"Datetime", {1}}}});
  const auto bbca = state.get_asset_handles()[0];
  const auto ihsg = state.get_asset_handles()[1];
  const auto missing_field = state.get_asset_handles()[2];
  const auto watchlist = *state.add_watchlist(Watchlist{"BBCA only", {bbca}});
  const auto main = add_strategy(state, "Main");
  auto failsafe = Strategy{};
  failsafe.name("Failsafe");
  failsafe.series_nodes().set(
   "external_close", pludux::backtest::ExternFieldNode{"market", "CLOSE"});
  state.add_strategy(std::move(failsafe));
  const auto failsafe_handle = state.get_strategy_handles().back();
  auto system = System{"BBCA System",
                       watchlist,
                       add_profile(state),
                       {},
                       StrategySetup{main},
                       {{StrategySetup{failsafe_handle}}}};
  auto error = state.external_configuration_error(system);
  EXPECT_NE(error.find("BBCA System"), std::string::npos);
  EXPECT_NE(error.find("market"), std::string::npos);
  EXPECT_NE(error.find("CLOSE"), std::string::npos);
  EXPECT_FALSE(state.is_system_ready(system));
  system.external_bindings()["market"] = missing_field;
  EXPECT_NE(state.external_configuration_error(system).find("does not provide"),
            std::string::npos);
  EXPECT_FALSE(state.is_system_ready(system));
  system.external_bindings()["market"] = ihsg;
  ASSERT_TRUE(state.is_system_ready(system));
  const auto system_handle = *state.add_system(std::move(system));
  const auto portfolio = *state.add_portfolio(
   Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {system_handle}});
  const auto seed_results = [&] {
    auto timeline = pludux::backtest::PortfolioTimeline{};
    timeline.append({.timestamp = 1, .capital = 42.0});
    return state.update_portfolio_results(
     portfolio, pludux::backtest::PortfolioResults{std::move(timeline), {}});
  };

  ASSERT_TRUE(seed_results());
  state.import_asset(Asset{"IHSG", {{"Datetime", {1}}, {"Close", {7200}}}});
  EXPECT_TRUE(state.get_portfolio_results(portfolio).timeline().empty());
  const auto& bound_system = state.get_system(system_handle);
  EXPECT_DOUBLE_EQ(
   state.compile_external_data(bound_system)->value("market", "CLOSE", 1),
   7200);

  ASSERT_TRUE(seed_results());
  ASSERT_TRUE(state.remove_asset(ihsg));
  EXPECT_EQ(state.get_system(system_handle).external_bindings().at("market"),
            ihsg);
  EXPECT_TRUE(state.get_portfolio_results(portfolio).timeline().empty());
  state.add_asset(Asset{"IHSG", {{"Datetime", {1}}, {"Close", {7300}}}});
  EXPECT_FALSE(state.is_system_ready(state.get_system(system_handle)));
  EXPECT_NE(state.external_configuration_error(state.get_system(system_handle))
             .find("bound Asset is missing"),
            std::string::npos);
}

TEST(ApplicationState, MissingStrategyRemainsReferencedAndInvalidatesPortfolio)
{
  auto state = pludux::apps::ApplicationState{};
  state.add_asset(pludux::backtest::Asset{"Asset"});
  const auto watchlist = *state.add_watchlist(
   pludux::backtest::Watchlist{"List", {state.get_asset_handles().front()}});
  const auto strategy = add_strategy(state);
  const auto system = *state.add_system(pludux::backtest::System{
   "System", watchlist, add_profile(state), {}, StrategySetup{strategy}});
  const auto portfolio = *state.add_portfolio(pludux::backtest::Portfolio{
   "Portfolio", 1'000.0, {}, {}, 10, 10, {}, {system}});

  auto timeline = pludux::backtest::PortfolioTimeline{};
  timeline.append({.timestamp = 1, .capital = 42.0});
  ASSERT_TRUE(state.update_portfolio_results(
   portfolio, pludux::backtest::PortfolioResults{std::move(timeline), {}}));
  ASSERT_TRUE(state.remove_strategy(strategy));

  EXPECT_EQ(state.get_system(system).main_setup().strategy_handle(), strategy);
  EXPECT_FALSE(state.is_portfolio_ready(state.get_portfolio(portfolio)));
  EXPECT_TRUE(state.get_portfolio_results(portfolio).timeline().empty());
}

TEST(ApplicationState, MissingSystemRemainsReferencedAndInvalidatesPortfolio)
{
  auto state = pludux::apps::ApplicationState{};
  state.add_asset(pludux::backtest::Asset{"Asset"});
  const auto watchlist = *state.add_watchlist(
   pludux::backtest::Watchlist{"List", {state.get_asset_handles().front()}});
  const auto strategy = add_strategy(state);
  const auto system = *state.add_system(pludux::backtest::System{
   "System", watchlist, add_profile(state), {}, StrategySetup{strategy}});
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

TEST(SystemEditor, ChangingStrategyResetsSetupInputsToNewDefaults)
{
  auto first_series =
   pludux::OrderedNamedRegistry<pludux::backtest::StrategyNode>{};
  first_series.set("first", pludux::NumericInputNode{
   "First", pludux::NumericInputNode::ValueRepresentation::Decimal, 2.5});
  auto second_series =
   pludux::OrderedNamedRegistry<pludux::backtest::StrategyNode>{};
  second_series.set("second", pludux::NumericInputNode{
   "Second", pludux::NumericInputNode::ValueRepresentation::UnsignedInteger, 14.0});
  const auto first = pludux::backtest::Strategy{
   "First", std::move(first_series), {}, {}, {}};
  const auto second = pludux::backtest::Strategy{
   "Second", std::move(second_series), {}, {}, {}};
  auto setup = pludux::backtest::StrategySetup{};

  pludux::backtest::assign_setup_strategy(
   setup, pludux::backtest::StrategyStoreHandle{1, 1}, first);
  auto overridden_inputs = setup.inputs();
  overridden_inputs.front().value(99.0);
  setup.inputs(std::move(overridden_inputs));
  pludux::backtest::assign_setup_strategy(
   setup, pludux::backtest::StrategyStoreHandle{2, 1}, second);

  ASSERT_EQ(setup.inputs().size(), 1U);
  EXPECT_EQ(setup.inputs().front().label(), "Second");
  EXPECT_DOUBLE_EQ(setup.inputs().front().value(), 14.0);
  EXPECT_EQ(setup.strategy_handle(), (pludux::backtest::StrategyStoreHandle{2, 1}));
}

TEST(WatchlistState, SetupSelectionSurvivesReorderAndClearsAfterRemoval)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"List", {asset}});
  const auto main = add_strategy(state, "Main");
  const auto first = add_strategy(state, "First");
  const auto second = add_strategy(state, "Second");
  const auto system =
   *state.add_system(System{"System",
                            watchlist,
                            add_profile(state),
                            {},
                            StrategySetup{main},
                            {{StrategySetup{first}}, {StrategySetup{second}}}});
  const auto portfolio = *state.add_portfolio(
   Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {system}});
  const auto selected = pludux::apps::PortfolioStrategyKey{{system, asset}, 2};
  ASSERT_TRUE(state.select_portfolio_strategy(portfolio, selected));
  auto edited = state.get_system(system);
  std::swap(edited.failsafe_strategies()[0], edited.failsafe_strategies()[1]);
  ASSERT_TRUE(state.update_system(system, edited));
  EXPECT_EQ(state.selected_portfolio_strategy(), selected);
  edited.failsafe_strategies().pop_back();
  ASSERT_TRUE(state.update_system(system, std::move(edited)));
  EXPECT_FALSE(state.selected_portfolio_strategy());
}

TEST(WatchlistState, PortfolioScopeAndChildSelectionAreRememberedIndependently)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"List", {asset}});
  const auto strategy = add_strategy(state);
  const auto system = *state.add_system(System{
   "System", watchlist, add_profile(state), {}, StrategySetup{strategy}});
  const auto first =
   *state.add_portfolio(Portfolio{"First", 1000, {}, {}, 10, 10, {}, {system}});
  const auto second = *state.add_portfolio(
   Portfolio{"Second", 1000, {}, {}, 10, 10, {}, {system}});
  const auto child = pludux::apps::PortfolioStrategyKey{{system, asset}, 0};
  state.select_portfolio(first);
  EXPECT_FALSE(state.selected_portfolio_strategy());
  ASSERT_TRUE(state.select_portfolio_strategy(second, child));
  EXPECT_EQ(state.selected_portfolio_handle(), second);
  state.select_portfolio(first);
  EXPECT_FALSE(state.selected_portfolio_strategy());
  state.select_portfolio(second);
  EXPECT_EQ(state.selected_portfolio_strategy(), child);
  state.deselect_portfolio_strategy(second);
  state.select_portfolio(first);
  state.select_portfolio(second);
  EXPECT_FALSE(state.selected_portfolio_strategy());
  auto edited = state.get_system(system);
  edited.name("Renamed");
  ASSERT_TRUE(state.update_system(system, std::move(edited)));
  EXPECT_FALSE(state.selected_portfolio_strategy());
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  const auto restored = pludux::apps::load_application_state_json(stream);
  EXPECT_FALSE(restored.selected_portfolio_strategy());
}

TEST(WatchlistState,
     PortfolioExpansionSelectsSingleAssetAndCollapseSelectsPortfolio)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"List", {asset}});
  const auto main = add_strategy(state, "Main");
  const auto failsafe = add_strategy(state, "Failsafe");
  const auto system = *state.add_system(System{"System",
                                               watchlist,
                                               add_profile(state),
                                               {},
                                               StrategySetup{main},
                                               {{StrategySetup{failsafe}}}});
  const auto portfolio = *state.add_portfolio(
   Portfolio{"Portfolio", 1000, {}, {}, 10, 10, {}, {system}});
  state.select_portfolio_context(portfolio, false);
  EXPECT_FALSE(state.selected_portfolio_strategy());
  state.select_portfolio_context(portfolio, true);
  EXPECT_EQ(state.selected_portfolio_strategy(),
            (pludux::apps::PortfolioStrategyKey{{system, asset}, 0}));
  const auto other = pludux::apps::PortfolioStrategyKey{{system, asset}, 1};
  ASSERT_TRUE(state.select_portfolio_strategy(portfolio, other));
  state.select_portfolio_context(portfolio, true);
  EXPECT_EQ(state.selected_portfolio_strategy(), other);
  state.select_portfolio_context(portfolio, false);
  EXPECT_EQ(state.selected_portfolio_handle(), portfolio);
  EXPECT_FALSE(state.selected_portfolio_strategy());
  state.select_portfolio_context(portfolio, true);
  EXPECT_EQ(state.selected_portfolio_strategy(), other);
  state.select_portfolio_context(portfolio, false);
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  auto restored = pludux::apps::load_application_state_json(stream);
  EXPECT_FALSE(restored.selected_portfolio_strategy());
  restored.select_portfolio_context(portfolio, true);
  EXPECT_EQ(restored.selected_portfolio_strategy(), other);
  // A removed remembered child falls back to the first valid child.
  state.select_portfolio_context(portfolio, false);
  ASSERT_TRUE(state.remove_strategy(failsafe));
  state.select_portfolio_context(portfolio, true);
  EXPECT_EQ(state.selected_portfolio_strategy(),
            (pludux::apps::PortfolioStrategyKey{{system, asset}, 0}));
  ASSERT_TRUE(state.remove_asset(asset));
  state.select_portfolio_context(portfolio, true);
  EXPECT_FALSE(state.selected_portfolio_strategy());
}

TEST(WatchlistState, MissingReferencesRemainAsDisabledStrategyRows)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"List", {asset}});
  const auto strategy = add_strategy(state);
  const auto system = *state.add_system(System{
   "System", watchlist, add_profile(state), {}, StrategySetup{strategy}});
  const auto portfolio = *state.add_portfolio(
   Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {system}});
  const auto selected = pludux::apps::PortfolioStrategyKey{{system, asset}, 0};
  ASSERT_TRUE(state.select_portfolio_strategy(portfolio, selected));
  ASSERT_TRUE(state.remove_asset(asset));
  EXPECT_EQ(state.expanded_system_strategies(state.get_portfolio(portfolio)),
            (std::vector{selected}));
  EXPECT_FALSE(state.selected_portfolio_strategy());
  ASSERT_TRUE(state.remove_system(system));
  EXPECT_EQ(state.expanded_system_strategies(state.get_portfolio(portfolio)),
            (std::vector<pludux::apps::PortfolioStrategyKey>{{{system, {}}, 0}}));
  EXPECT_FALSE(state.selected_portfolio_strategy());
}

TEST(WatchlistState, MissingStrategyDependencyReturnsToPortfolioScope)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"List", {asset}});
  const auto main = add_strategy(state, "Main");
  const auto failsafe = add_strategy(state, "Failsafe");
  const auto system = *state.add_system(System{"System",
                                               watchlist,
                                               add_profile(state),
                                               {},
                                               StrategySetup{main},
                                               {{StrategySetup{failsafe}}}});
  const auto portfolio = *state.add_portfolio(
   Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {system}});
  const auto selected = pludux::apps::PortfolioStrategyKey{{system, asset}, 1};
  ASSERT_TRUE(state.select_portfolio_strategy(portfolio, selected));
  ASSERT_TRUE(state.remove_strategy(failsafe));
  EXPECT_FALSE(state.selected_portfolio_strategy());
  EXPECT_FALSE(state.select_portfolio_strategy(portfolio, selected));
}

TEST(WatchlistState, DeletingSharedProfileDisablesBothSetupsUntilReassigned)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"List", {asset}});
  const auto strategy = add_strategy(state);
  const auto profile = add_profile(state);
  const auto system = *state.add_system(System{"Turtle",
                                               watchlist,
                                               profile,
                                               {},
                                               StrategySetup{strategy},
                                               {{StrategySetup{strategy}}}});
  const auto portfolio = *state.add_portfolio(
   Portfolio{"Portfolio", 1000, {}, {}, 10, 10, {}, {system}});
  const auto main = pludux::apps::PortfolioStrategyKey{{system, asset}, 0};
  const auto failsafe = pludux::apps::PortfolioStrategyKey{{system, asset}, 1};
  ASSERT_TRUE(state.is_system_ready(state.get_system(system)));
  ASSERT_TRUE(state.select_portfolio_strategy(portfolio, failsafe));

  ASSERT_TRUE(state.remove_profile(profile));
  EXPECT_EQ(state.get_system(system).profile_handle(), profile);
  EXPECT_FALSE(state.is_system_ready(state.get_system(system)));
  EXPECT_FALSE(state.selected_portfolio_strategy());
  EXPECT_FALSE(state.select_portfolio_strategy(portfolio, main));
  EXPECT_FALSE(state.select_portfolio_strategy(portfolio, failsafe));

  auto edited = state.get_system(system);
  edited.profile_handle(add_profile(state));
  ASSERT_TRUE(state.update_system(system, std::move(edited)));
  EXPECT_TRUE(state.is_system_ready(state.get_system(system)));
  EXPECT_TRUE(state.select_portfolio_strategy(portfolio, main));
  EXPECT_TRUE(state.select_portfolio_strategy(portfolio, failsafe));
}

TEST(WatchlistState, PersistsWatchlistStoreAndReusableReferences)
{
  auto state = ApplicationState{};
  state.add_asset(Asset{"BTC"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist = *state.add_watchlist(Watchlist{"Crypto", {asset}});
  const auto strategy = add_strategy(state, "Momentum");
  const auto system = *state.add_system(
   System{"Trend", watchlist, add_profile(state), {}, StrategySetup{strategy}});
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
  EXPECT_EQ(loaded.get_system(loaded.get_system_handles().front()).main_setup().strategy_handle(),
            loaded.get_strategy_handles().front());
  EXPECT_EQ(loaded.selected_system_handle(), loaded.get_system_handles().front());
}

} // namespace
