#include <gtest/gtest.h>

#include <jsoncons/json.hpp>

#include <exception>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

import pludux.apps.backtest.application_state;
import pludux.apps.backtest.command_executor;
import pludux.apps.backtest.serialization;
import pludux.apps.backtest.portfolio_strategy_selections;
import pludux.apps.backtest.view_state;
import pludux.backtest;

namespace {

using pludux::apps::ApplicationState;
using pludux::apps::CommandExecutor;
using pludux::apps::EditCommand;
using pludux::apps::ExecutionEffect;
using pludux::apps::RedoCommand;
using pludux::apps::ReplaceApplicationCommand;
using pludux::apps::UndoCommand;
using pludux::apps::ViewCommand;
using pludux::backtest::CallTable;
using pludux::backtest::IntrabarPath;
using pludux::backtest::Portfolio;
using pludux::backtest::PortfolioResults;
using pludux::backtest::PortfolioTimeline;
using pludux::backtest::StrategySetup;

auto example_calls() -> CallTable
{
  return CallTable{
   "Desk calls",
   {"Timing", "Symbol", "Bullish", "Entry Low", "Entry High", "SL1", "SL2"},
   {{"2025-01-06", "ABC", "true", "98", "102", "90", "85"},
    {"2025-01-07", "XYZ", "false", "", "200", "210", "220"}}};
}

auto add_strategy(ApplicationState& state, std::string name)
{
  auto strategy = pludux::backtest::Strategy{};
  strategy.name(std::move(name));
  state.add_strategy(std::move(strategy));
  return state.get_strategy_handles().back();
}

void rename_strategy(ApplicationState& state,
                     pludux::backtest::StrategyStoreHandle handle,
                     std::string name)
{
  auto strategy = state.get_strategy(handle);
  strategy.name(std::move(name));
  ASSERT_TRUE(state.update_strategy(handle, std::move(strategy)));
}

TEST(CommandHistory, ViewSelectionPreservesRedoAndAvoidsDocumentEffect)
{
  auto state = ApplicationState{};
  const auto first = add_strategy(state, "First");
  const auto second = add_strategy(state, "Second");
  state.select_strategy(first);
  auto executor = CommandExecutor{};
  executor.push(EditCommand{
   "Rename",
   [first](auto& candidate) { rename_strategy(candidate, first, "Changed"); },
   std::nullopt});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::DocumentChanged);
  executor.push(UndoCommand{});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::DocumentChanged);
  ASSERT_TRUE(executor.can_redo());
  executor.push(ViewCommand{
   [second](auto& candidate) { candidate.select_strategy(second); }});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::ViewChanged);
  EXPECT_EQ(state.selected_strategy_handle(), second);
  EXPECT_TRUE(executor.can_redo());
  EXPECT_FALSE(executor.can_undo());
}

TEST(CommandHistory, UndoAndRedoPreserveCurrentSelection)
{
  auto state = ApplicationState{};
  const auto edited = add_strategy(state, "Original");
  const auto selected = add_strategy(state, "Selected");
  auto executor = CommandExecutor{};
  executor.push(EditCommand{
   "Rename",
   [edited](auto& candidate) { rename_strategy(candidate, edited, "Changed"); },
   std::nullopt});
  executor.execute(state);
  state.select_strategy(selected);
  executor.push(UndoCommand{});
  executor.execute(state);
  EXPECT_EQ(state.get_strategy(edited).name(), "Original");
  EXPECT_EQ(state.selected_strategy_handle(), selected);
  executor.push(RedoCommand{});
  executor.execute(state);
  EXPECT_EQ(state.get_strategy(edited).name(), "Changed");
  EXPECT_EQ(state.selected_strategy_handle(), selected);
}

TEST(CommandHistory, UndoNormalizesSelectionWhenItsTargetDisappears)
{
  auto state = ApplicationState{};
  auto executor = CommandExecutor{};
  executor.push(
   EditCommand{"Add",
               [](auto& candidate) { add_strategy(candidate, "Added"); },
               std::nullopt});
  executor.execute(state);
  const auto added = state.get_strategy_handles().front();
  state.select_strategy(added);
  executor.push(UndoCommand{});
  executor.execute(state);
  EXPECT_EQ(state.selected_strategy_handle(),
            pludux::backtest::StrategyStoreHandle{});
}

TEST(CommandHistory, NoOpDoesNotCreateHistoryOrClearRedo)
{
  auto state = ApplicationState{};
  const auto strategy = add_strategy(state, "Original");
  auto executor = CommandExecutor{};
  executor.push(EditCommand{"Rename",
                            [strategy](auto& candidate) {
                              rename_strategy(candidate, strategy, "Changed");
                            },
                            std::nullopt});
  executor.execute(state);
  executor.push(UndoCommand{});
  executor.execute(state);
  ASSERT_TRUE(executor.can_redo());
  executor.push(EditCommand{"No-op", [](auto&) {}, std::nullopt});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::None);
  EXPECT_TRUE(executor.can_redo());
  EXPECT_FALSE(executor.can_undo());
}

TEST(CommandHistory, UndoRestoresCachedPortfolioResults)
{
  auto state = ApplicationState{};
  const auto portfolio = *state.add_portfolio(Portfolio{});
  auto timeline = PortfolioTimeline{};
  timeline.append({.timestamp = 1, .capital = 42.0});
  ASSERT_TRUE(state.update_portfolio_results(
   portfolio, PortfolioResults{std::move(timeline), {}}));
  auto executor = CommandExecutor{};
  executor.push(EditCommand{"Edit Portfolio",
                            [portfolio](auto& candidate) {
                              auto value = candidate.get_portfolio(portfolio);
                              value.name("Changed");
                              ASSERT_TRUE(candidate.update_portfolio(
                               portfolio, std::move(value)));
                            },
                            std::nullopt});
  executor.execute(state);
  EXPECT_TRUE(state.get_portfolio_results(portfolio).timeline().empty());
  executor.push(UndoCommand{});
  executor.execute(state);
  ASSERT_EQ(state.get_portfolio_results(portfolio).timeline().size(), 1U);
  EXPECT_DOUBLE_EQ(
   state.get_portfolio_results(portfolio).timeline().row(0).capital, 42.0);
}

TEST(CommandHistory, IntrabarPathEditsInvalidateResultsAndSupportUndoRedo)
{
  auto state = ApplicationState{};
  const auto handle = state.add_portfolio(Portfolio{});
  ASSERT_TRUE(handle);
  auto timeline = PortfolioTimeline{};
  timeline.append({.timestamp = 1, .capital = 42.0});
  ASSERT_TRUE(state.update_portfolio_results(
   *handle, PortfolioResults{std::move(timeline), {}}));

  auto executor = CommandExecutor{};
  executor.push(EditCommand{"Set Intrabar Path",
                            [handle = *handle](auto& candidate) {
                              auto portfolio = candidate.get_portfolio(handle);
                              portfolio.intrabar_path(IntrabarPath::HighFirst);
                              ASSERT_TRUE(candidate.update_portfolio(
                               handle, std::move(portfolio)));
                            },
                            std::nullopt});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::DocumentChanged);
  EXPECT_EQ(state.get_portfolio(*handle).intrabar_path(),
            IntrabarPath::HighFirst);
  EXPECT_TRUE(state.get_portfolio_results(*handle).timeline().empty());

  executor.push(UndoCommand{});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::DocumentChanged);
  EXPECT_EQ(state.get_portfolio(*handle).intrabar_path(),
            IntrabarPath::CandleDirection);

  executor.push(RedoCommand{});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::DocumentChanged);
  EXPECT_EQ(state.get_portfolio(*handle).intrabar_path(),
            IntrabarPath::HighFirst);
}

TEST(CommandHistory, ExplicitMergeKeyCoalescesAdjacentEdits)
{
  auto state = ApplicationState{};
  const auto strategy = add_strategy(state, "Original");
  auto executor = CommandExecutor{};
  for(const auto* name : {"First", "Second", "Final"}) {
    executor.push(EditCommand{"Rename",
                              [strategy, name](auto& candidate) {
                                rename_strategy(candidate, strategy, name);
                              },
                              std::string{"strategy-name"}});
    executor.execute(state);
  }
  EXPECT_EQ(executor.undo_size(), 1U);
  executor.push(UndoCommand{});
  executor.execute(state);
  EXPECT_EQ(state.get_strategy(strategy).name(), "Original");
  executor.push(RedoCommand{});
  executor.execute(state);
  EXPECT_EQ(state.get_strategy(strategy).name(), "Final");
}

TEST(CommandHistory, ViewCommandEndsCoalescingSequence)
{
  auto state = ApplicationState{};
  const auto strategy = add_strategy(state, "Original");
  auto executor = CommandExecutor{};
  for(const auto* name : {"First", "Second"}) {
    executor.push(EditCommand{"Rename",
                              [strategy, name](auto& candidate) {
                                rename_strategy(candidate, strategy, name);
                              },
                              std::string{"strategy-name"}});
    executor.execute(state);
    if(std::string{name} == "First") {
      executor.push(ViewCommand{[](auto&) {}});
      executor.execute(state);
    }
  }
  EXPECT_EQ(executor.undo_size(), 2U);
}

TEST(CommandHistory, HistoryKeepsNewestOneHundredEntries)
{
  auto state = ApplicationState{};
  auto executor = CommandExecutor{};
  for(auto index = 0; index < 101; ++index) {
    executor.push(
     EditCommand{"Add",
                 [index](auto& candidate) {
                   add_strategy(candidate, "Strategy " + std::to_string(index));
                 },
                 std::nullopt});
    executor.execute(state);
  }
  EXPECT_EQ(executor.undo_size(), CommandExecutor::history_limit);
  for(auto index = 0; index < 100; ++index) {
    executor.push(UndoCommand{});
    executor.execute(state);
  }
  ASSERT_EQ(state.get_strategy_handles().size(), 1U);
  EXPECT_EQ(state.get_strategy(state.get_strategy_handles().front()).name(),
            "Strategy 0");
}

TEST(CommandHistory, ApplicationReplacementIsUndoableAndRedoable)
{
  auto state = ApplicationState{};
  auto executor = CommandExecutor{};
  executor.push(
   EditCommand{"Add",
               [](auto& candidate) { add_strategy(candidate, "Added"); },
               std::nullopt});
  executor.execute(state);
  state.select_strategy(state.get_strategy_handles().front());
  state.imgui_ini_settings("original-layout");
  executor.push(ReplaceApplicationCommand{[] {
    auto replacement = ApplicationState{};
    const auto strategy = add_strategy(replacement, "Replacement");
    replacement.select_strategy(strategy);
    replacement.imgui_ini_settings("replacement-layout");
    return replacement;
  }});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::ApplicationReplaced);
  EXPECT_EQ(state.get_strategy(state.get_strategy_handles().front()).name(),
            "Replacement");
  executor.push(UndoCommand{});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::ApplicationReplaced);
  EXPECT_EQ(state.get_strategy(state.get_strategy_handles().front()).name(),
            "Added");
  EXPECT_EQ(state.imgui_ini_settings(), "original-layout");
  executor.push(RedoCommand{});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::ApplicationReplaced);
  EXPECT_EQ(state.get_strategy(state.get_strategy_handles().front()).name(),
            "Replacement");
  EXPECT_EQ(state.imgui_ini_settings(), "replacement-layout");
}

TEST(CommandHistory, StrategyEditsInvalidateDependentPortfolioResults)
{
  auto state = pludux::apps::ApplicationState{};
  state.add_strategy(pludux::backtest::Strategy{});
  state.add_profile(pludux::backtest::Profile{});
  state.add_asset(pludux::backtest::Asset{"Asset"});
  const auto watchlist = *state.add_watchlist(
   pludux::backtest::Watchlist{"List", {state.get_asset_handles().front()}});
  const auto strategy = state.get_strategy_handles().front();
  const auto system = *state.add_system(
   pludux::backtest::System{"System",
                            watchlist,
                            state.get_profile_handles().front(),
                            {},
                            StrategySetup{strategy}});
  const auto portfolio = *state.add_portfolio(pludux::backtest::Portfolio{
   "Portfolio", 1'000.0, {}, {}, 10, 10, {}, {system}});
  auto timeline = pludux::backtest::PortfolioTimeline{};
  timeline.append({.timestamp = 1, .capital = 42.0});
  ASSERT_TRUE(state.update_portfolio_results(
   portfolio, pludux::backtest::PortfolioResults{std::move(timeline), {}}));

  auto executor = pludux::apps::CommandExecutor{};
  executor.push(pludux::apps::EditCommand{
   "Rename Strategy",
   [strategy](auto& candidate) {
     auto value = candidate.get_strategy(strategy);
     value.name("Edited");
     ASSERT_TRUE(candidate.update_strategy(strategy, std::move(value)));
   },
   std::nullopt});
  EXPECT_EQ(executor.execute(state),
            pludux::apps::ExecutionEffect::DocumentChanged);
  EXPECT_EQ(state.get_portfolio_results(portfolio),
            pludux::backtest::PortfolioResults{});
}

TEST(CommandHistory, UnrelatedEditClearsPartialResultsBeforeRunnerReplacement)
{
  auto state = pludux::apps::ApplicationState{};
  const auto portfolio = *state.add_portfolio(pludux::backtest::Portfolio{});
  auto timeline = pludux::backtest::PortfolioTimeline{};
  timeline.append({.timestamp = 1, .capital = 42.0});
  ASSERT_TRUE(state.update_portfolio_results(
   portfolio, pludux::backtest::PortfolioResults{std::move(timeline), {}}));
  ASSERT_EQ(state.get_portfolio_results(portfolio).timeline().size(), 1U);

  auto executor = pludux::apps::CommandExecutor{};
  executor.push(pludux::apps::EditCommand{
   "Add Unrelated Profile",
   [](auto& candidate) {
     candidate.add_profile(pludux::backtest::Profile{"Unrelated"});
   },
   std::nullopt});

  EXPECT_EQ(executor.execute(state),
            pludux::apps::ExecutionEffect::DocumentChanged);
  EXPECT_EQ(state.get_portfolio_results(portfolio),
            pludux::backtest::PortfolioResults{});
}

TEST(CommandHistory, ReimportingCallsInvalidatesAndUndoRestoresPortfolioResults)
{
  auto state = ApplicationState{};
  const auto original = example_calls();
  const auto call = state.add_call(original);
  ASSERT_TRUE(call);
  auto system = pludux::backtest::System{};
  system.call_handle(*call);
  const auto system_handle = state.add_system(std::move(system));
  ASSERT_TRUE(system_handle);
  const auto portfolio = state.add_portfolio(
   Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {*system_handle}});
  ASSERT_TRUE(portfolio);
  auto timeline = PortfolioTimeline{};
  timeline.append({.timestamp = 1, .capital = 42.0});
  ASSERT_TRUE(state.update_portfolio_results(
   *portfolio, PortfolioResults{std::move(timeline), {}}));
  auto replacement = original;
  replacement.rows[0][5] = "93";
  auto executor = CommandExecutor{};
  executor.push(EditCommand{"Reimport Calls",
                            [call = *call, replacement](auto& candidate) {
                              ASSERT_TRUE(
                               candidate.update_call(call, replacement));
                            },
                            std::nullopt});

  ASSERT_EQ(executor.execute(state), ExecutionEffect::DocumentChanged);
  EXPECT_EQ(state.get_call(*call), replacement);
  EXPECT_TRUE(state.get_portfolio_results(*portfolio).timeline().empty());
  executor.push(UndoCommand{});
  ASSERT_EQ(executor.execute(state), ExecutionEffect::DocumentChanged);
  EXPECT_EQ(state.get_call(*call), original);
  ASSERT_EQ(state.get_portfolio_results(*portfolio).timeline().size(), 1U);
  EXPECT_DOUBLE_EQ(
   state.get_portfolio_results(*portfolio).timeline().row(0).capital, 42.0);
  executor.push(RedoCommand{});
  ASSERT_EQ(executor.execute(state), ExecutionEffect::DocumentChanged);
  EXPECT_EQ(state.get_call(*call), replacement);
  EXPECT_TRUE(state.get_portfolio_results(*portfolio).timeline().empty());
  EXPECT_EQ(state.get_system(*system_handle).call_handle(), *call);
}

TEST(ApplicationStateCalls, RevalidatesTableStrategySetupAndWatchlist)
{
  using namespace pludux::backtest;
  auto state = ApplicationState{};
  state.add_profile(Profile{});
  state.add_asset(Asset{"ABC.csv"});
  const auto watchlist =
   *state.add_watchlist(Watchlist{"Desk", {state.get_asset_handles().front()}});
  auto series = pludux::OrderedNamedRegistry<StrategyNode>{};
  series.set("stop", CallNode{"stop"});
  auto position = Strategy::Position{};
  position.entry(Strategy::Entry{pludux::FalseNode{},
                                 pludux::OpenNode{},
                                 1,
                                 PriceExitsStart::CurrentBar,
                                 EntrySource::Call});
  state.add_strategy(Strategy{"Strat", series, position, {}, {}});
  const auto strategy = state.get_strategy_handles().front();
  auto setup = StrategySetup{strategy};
  setup.call_mappings()["stop"] = "SL1";
  const auto table = *state.add_call(example_calls());
  auto system =
   System{"Desk", watchlist, state.get_profile_handles().front(), {}, setup};
  system.call_handle(table);
  system.call_columns(
   {"Timing", "Symbol", "Bullish", "Entry Low", "Entry High"});
  EXPECT_TRUE(state.is_system_ready(system));
  EXPECT_EQ(state.compiled_calls(system)->outside_watchlist,
            (std::vector<std::size_t>{3}));
  auto changed = example_calls();
  changed.name = "Renamed";
  EXPECT_TRUE(state.update_call(table, changed));
  EXPECT_TRUE(state.is_system_ready(system));
  changed.headers[5] = "New stop";
  EXPECT_TRUE(state.update_call(table, changed));
  EXPECT_FALSE(state.is_system_ready(system));
  system.main_setup().call_mappings()["stop"] = "New stop";
  EXPECT_TRUE(state.is_system_ready(system));
  state.add_asset(Asset{"XYZ.csv"});
  ASSERT_TRUE(state.update_watchlist(
   watchlist, Watchlist{"Desk", {state.get_asset_handles().back()}}));
  EXPECT_EQ(state.compiled_calls(system)->outside_watchlist,
            (std::vector<std::size_t>{2}));
  series.set("target", CallNode{"target"});
  ASSERT_TRUE(state.update_strategy(
   strategy, Strategy{"Strat", series, position, {}, {}}));
  EXPECT_FALSE(state.is_system_ready(system));
  system.main_setup().call_mappings()["target"] = "SL2";
  EXPECT_TRUE(state.is_system_ready(system));
  EXPECT_TRUE(state.remove_call(table));
  EXPECT_FALSE(state.is_system_ready(system));
}

TEST(CommandHistory, StrategyInputSynchronizationIsUndoable)
{
  auto original_series = pludux::OrderedNamedRegistry<
   pludux::ErasedNode<pludux::backtest::BacktestMethodContext>>{};
  original_series.set(
   "input",
   pludux::NumericInputNode{
    "Original", pludux::NumericInputNode::ValueRepresentation::Decimal, 2.5});
  auto state = pludux::apps::ApplicationState{};
  state.add_strategy(pludux::backtest::Strategy{
   "Strategy", std::move(original_series), {}, {}, {}});
  const auto strategy = state.get_strategy_handles().front();
  state.add_profile(pludux::backtest::Profile{});
  auto setup = StrategySetup{};
  pludux::backtest::assign_setup_strategy(
   setup, strategy, state.get_strategy(strategy));
  auto inputs = setup.inputs();
  inputs.front().value(99.0);
  setup.inputs(std::move(inputs));
  auto alternate = setup;
  inputs = alternate.inputs();
  inputs.front().value(55.0);
  alternate.inputs(std::move(inputs));
  const auto system = *state.add_system(pludux::backtest::System{
   "Turtle", {}, state.get_profile_handles().front(), {}, setup, {{alternate}}});

  auto executor = pludux::apps::CommandExecutor{};
  executor.push(pludux::apps::EditCommand{
   "Remove Strategy Input",
   [strategy](auto& candidate) {
     ASSERT_TRUE(candidate.update_strategy(
      strategy, pludux::backtest::Strategy{"Strategy", {}, {}, {}, {}}));
   },
   std::nullopt});
  EXPECT_EQ(executor.execute(state),
            pludux::apps::ExecutionEffect::DocumentChanged);
  EXPECT_TRUE(state.get_system(system).main_setup().inputs().empty());
  EXPECT_TRUE(state.get_system(system).strategy_setup(1).inputs().empty());

  executor.push(pludux::apps::UndoCommand{});
  EXPECT_EQ(executor.execute(state),
            pludux::apps::ExecutionEffect::DocumentChanged);
  ASSERT_EQ(state.get_system(system).main_setup().inputs().size(), 1U);
  EXPECT_EQ(state.get_system(system).main_setup().inputs().front().label(),
            "Original");
  EXPECT_DOUBLE_EQ(
   state.get_system(system).main_setup().inputs().front().value(), 99.0);
  ASSERT_EQ(state.get_system(system).strategy_setup(1).inputs().size(), 1U);
  EXPECT_DOUBLE_EQ(
   state.get_system(system).strategy_setup(1).inputs().front().value(), 55.0);
  EXPECT_EQ(
   pludux::backtest::collect_strategy_inputs(state.get_strategy(strategy))
    .size(),
   1U);

  executor.push(pludux::apps::RedoCommand{});
  EXPECT_EQ(executor.execute(state),
            pludux::apps::ExecutionEffect::DocumentChanged);
  EXPECT_TRUE(state.get_system(system).main_setup().inputs().empty());
  EXPECT_TRUE(state.get_system(system).strategy_setup(1).inputs().empty());
}

TEST(ApplicationStateSerialization, RoundTripsStoredStrategyReferences)
{
  auto state = pludux::apps::ApplicationState{};
  add_strategy(state, "Stored");
  const auto strategy = state.get_strategy_handles().front();
  ASSERT_TRUE(
   state.add_system(pludux::backtest::System{"System",
                                             {},
                                             {},
                                             {},
                                             StrategySetup{strategy},
                                             {{StrategySetup{strategy}}}}));
  auto json = std::stringstream{};
  pludux::apps::save_application_state_json(json, state);
  EXPECT_NE(json.str().find("\"strategies\""), std::string::npos);
  EXPECT_NE(json.str().find("\"strategyStoreDataResolver\""),
            std::string::npos);
  const auto loaded = pludux::apps::load_application_state_json(json);
  ASSERT_EQ(loaded.get_strategy_handles().size(), 1U);
  const auto& system = loaded.get_system(loaded.get_system_handles().front());
  EXPECT_EQ(system.main_setup().strategy_handle(), strategy);
  EXPECT_EQ(system.alternate_strategies().front().setup().strategy_handle(),
            strategy);
}

TEST(ApplicationStateSerialization, RoundTripsStandaloneStrategySelection)
{
  auto state = pludux::apps::ApplicationState{};
  add_strategy(state, "Selected");
  const auto strategy = state.get_strategy_handles().front();
  state.select_strategy(strategy);

  auto json = std::stringstream{};
  pludux::apps::save_application_state_json(json, state);

  EXPECT_NE(json.str().find("\"selectedStrategyHandle\""), std::string::npos);
  const auto loaded = pludux::apps::load_application_state_json(json);
  EXPECT_EQ(loaded.selected_strategy_handle(), strategy);
}

TEST(ApplicationStateSerialization, RoundTripsCallTableAndSetupMappings)
{
  auto state = ApplicationState{};
  const auto table = example_calls();
  const auto call = state.add_call(table);
  ASSERT_TRUE(call);
  auto series = pludux::OrderedNamedRegistry<pludux::backtest::StrategyNode>{};
  series.set("call stop", pludux::backtest::CallNode{"stop"});
  state.add_strategy(
   pludux::backtest::Strategy{"Calls", std::move(series), {}, {}, {}});
  const auto strategy = state.get_strategy_handles().front();
  auto main_setup = StrategySetup{strategy};
  auto alternate_setup = StrategySetup{strategy};
  main_setup.call_mappings() = {{"stop", "SL1"}};
  alternate_setup.call_mappings() = {{"stop", "SL2"}};
  const auto columns = pludux::backtest::CallColumns{
   "Timing", "Symbol", "Bullish", "Entry Low", "Entry High"};
  auto system = pludux::backtest::System{
   "System", {}, {}, {}, main_setup, {{alternate_setup}}};
  system.call_handle(*call);
  system.call_columns(columns);
  system.entry_priority(pludux::backtest::EntryPriority::SignalFirst);
  ASSERT_TRUE(state.add_system(std::move(system)));
  auto json = std::stringstream{};

  pludux::apps::save_application_state_json(json, state);
  const auto loaded = pludux::apps::load_application_state_json(json);

  ASSERT_EQ(loaded.get_call_handles().size(), 1U);
  EXPECT_EQ(loaded.get_call_handles().front(), *call);
  EXPECT_EQ(loaded.get_call(*call), table);
  ASSERT_EQ(loaded.get_system_handles().size(), 1U);
  const auto& restored = loaded.get_system(loaded.get_system_handles().front());
  EXPECT_EQ(restored.call_handle(), *call);
  EXPECT_EQ(restored.call_columns(), columns);
  EXPECT_EQ(restored.entry_priority(),
            pludux::backtest::EntryPriority::SignalFirst);
  EXPECT_EQ(restored.main_setup().call_mappings(), main_setup.call_mappings());
  ASSERT_EQ(restored.alternate_strategies().size(), 1U);
  EXPECT_EQ(restored.alternate_strategies().front().setup().call_mappings(),
            alternate_setup.call_mappings());
}

TEST(ApplicationStateSerialization, RoundTripsDocumentAndViewSeparately)
{
  auto state = ApplicationState{};
  const auto strategy = add_strategy(state, "Selected");
  state.select_strategy(strategy);
  state.imgui_ini_settings("layout-data");
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  const auto serialized = stream.str();
  EXPECT_NE(serialized.find("\"documentState\""), std::string::npos);
  EXPECT_NE(serialized.find("\"viewState\""), std::string::npos);
  EXPECT_EQ(serialized.find("\"uiState\""), std::string::npos);
  const auto loaded = pludux::apps::load_application_state_json(stream);
  ASSERT_EQ(loaded.get_strategy_handles().size(), 1U);
  EXPECT_EQ(loaded.selected_strategy_handle(),
            loaded.get_strategy_handles().front());
  EXPECT_EQ(loaded.imgui_ini_settings(), "layout-data");
}

TEST(ApplicationStateSerialization, WritesNullForUnsetOptionalHandles)
{
  auto state = ApplicationState{};
  const auto system = state.add_system(pludux::backtest::System{});
  const auto portfolio = state.add_portfolio(Portfolio{});
  ASSERT_TRUE(system);
  ASSERT_TRUE(portfolio);
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);

  const auto saved = jsoncons::ojson::parse(stream.str());
  const auto& workspace = saved.at("pludux");
  const auto& view = workspace.at("viewState");
  EXPECT_TRUE(view.at("selectedPortfolioHandle").is_null());
  EXPECT_TRUE(view.at("selectedSystemHandle").is_null());
  EXPECT_TRUE(view.at("selectedStrategyHandle").is_null());
  const auto& arena = workspace.at("store").at("arena");
  const auto& saved_system = arena.at("systems").at(0);
  EXPECT_TRUE(saved_system.at("watchlist").is_null());
  EXPECT_TRUE(saved_system.at("profile").is_null());
  EXPECT_TRUE(saved_system.at("callTable").is_null());
  EXPECT_TRUE(saved_system.at("mainSetup").at("strategy").is_null());
  const auto& saved_portfolio = arena.at("portfolios").at(0);
  EXPECT_TRUE(saved_portfolio.at("market").is_null());
  EXPECT_TRUE(saved_portfolio.at("broker").is_null());

  const auto loaded = pludux::apps::load_application_state_json(stream);
  EXPECT_EQ(loaded.view_state(), pludux::apps::ViewState{});
  EXPECT_EQ(loaded.get_system(*system), state.get_system(*system));
  EXPECT_EQ(loaded.get_portfolio(*portfolio), state.get_portfolio(*portfolio));
}

TEST(ApplicationStateSerialization, PreservesExact64BitHandleGenerations)
{
  constexpr auto generation =
   pludux::backtest::StoreGeneration{9'007'199'254'740'993};
  auto state = ApplicationState{};
  state.select_strategy(add_strategy(state, "Selected"));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  auto saved = jsoncons::ojson::parse(stream.str());
  auto& workspace = saved.at("pludux");
  workspace.at("viewState").at("selectedStrategyHandle")["generation"] =
   generation;
  workspace.at("documentState").at("strategyHandles").at(0)["generation"] =
   generation;
  workspace.at("store")
   .at("descriptor")
   .at("strategyStoreDataResolver")
   .at("slots")
   .at(0)["generation"] = generation;

  auto input = std::stringstream{saved.to_string()};
  const auto loaded = pludux::apps::load_application_state_json(input);
  const auto expected = pludux::backtest::StrategyStoreHandle{0, generation};
  EXPECT_EQ(loaded.selected_strategy_handle(), expected);
  ASSERT_EQ(loaded.get_strategy_handles().size(), 1U);
  EXPECT_EQ(loaded.get_strategy_handles().front(), expected);
  ASSERT_NE(loaded.get_strategy_if_present(expected), nullptr);
  EXPECT_EQ(loaded.get_strategy(expected).name(), "Selected");

  auto output = std::stringstream{};
  pludux::apps::save_application_state_json(output, loaded);
  EXPECT_NE(output.str().find("\"generation\":9007199254740993"),
            std::string::npos);
}

TEST(ApplicationStateSerialization, RejectsInvalidHandleAndArenaIndices)
{
  auto state = ApplicationState{};
  state.select_strategy(add_strategy(state, "Selected"));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  const auto saved = jsoncons::ojson::parse(stream.str());
  const auto expect_rejected = [](const jsoncons::ojson& json) {
    auto input = std::stringstream{json.to_string()};
    EXPECT_THROW(pludux::apps::load_application_state_json(input),
                 std::exception);
  };

  for(const auto* value :
      {"4294967295", "4294967296", "18446744073709551615"}) {
    SCOPED_TRACE(value);
    auto json = saved;
    json.at("pludux")
     .at("viewState")
     .at("selectedStrategyHandle")["slotIndex"] = jsoncons::ojson::parse(value);
    expect_rejected(json);
  }

  auto json = saved;
  json.at("pludux").at("documentState").at("strategyHandles").at(0) =
   jsoncons::ojson::null();
  expect_rejected(json);

  for(const auto* field : {"valueToSlotIndices", "freeSlotIndices"}) {
    SCOPED_TRACE(field);
    for(const auto* indices :
        {"[-1]", "[0.5]", "[4294967295]", "[4294967296]"}) {
      SCOPED_TRACE(indices);
      json = saved;
      json.at("pludux")
       .at("store")
       .at("descriptor")
       .at("strategyStoreDataResolver")[field] =
       jsoncons::ojson::parse(indices);
      expect_rejected(json);
    }
  }
}

TEST(ApplicationStateSerialization, DefaultsAbsentViewStateAndIgnoresUnknownKeys)
{
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, ApplicationState{});
  auto json = jsoncons::ojson::parse(stream.str());
  json.at("pludux").erase("viewState");
  json.at("pludux")["unknown"] = 42;
  auto input = std::stringstream{json.to_string()};

  const auto loaded = pludux::apps::load_application_state_json(input);
  EXPECT_TRUE(loaded.imgui_ini_settings().empty());
  EXPECT_FALSE(loaded.selected_portfolio_handle().valid());
}

TEST(ApplicationStateSerialization, DefaultsIndividualViewStateFields)
{
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, ApplicationState{});
  const auto saved = jsoncons::ojson::parse(stream.str());
  for(const auto* field : {"imguiIniSettings",
                           "selectedPortfolioHandle",
                           "portfolioStrategySelections",
                           "selectedSystemHandle",
                           "selectedStrategyHandle"}) {
    SCOPED_TRACE(field);
    auto json = saved;
    json.at("pludux").at("viewState").erase(field);
    auto input = std::stringstream{json.to_string()};
    const auto loaded = pludux::apps::load_application_state_json(input);
    EXPECT_EQ(loaded.view_state(), pludux::apps::ViewState{});
  }
}

TEST(ApplicationStateSerialization, RequiresWorkspaceBookkeeping)
{
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, ApplicationState{});
  const auto saved = jsoncons::ojson::parse(stream.str());
  const auto expect_rejected = [](jsoncons::ojson json) {
    auto input = std::stringstream{json.to_string()};
    EXPECT_THROW(pludux::apps::load_application_state_json(input),
                 std::exception);
  };

  for(const auto* field : {"$version", "store", "documentState"}) {
    SCOPED_TRACE(field);
    auto json = saved;
    json.at("pludux").erase(field);
    expect_rejected(std::move(json));
  }
  for(const auto* field : {"descriptor", "arena"}) {
    SCOPED_TRACE(field);
    auto json = saved;
    json.at("pludux").at("store").erase(field);
    expect_rejected(std::move(json));
  }
  for(const auto* field : {"systems",
                           "portfolios",
                           "assets",
                           "watchlists",
                           "markets",
                           "brokers",
                           "profiles",
                           "strategies"}) {
    SCOPED_TRACE(field);
    auto json = saved;
    json.at("pludux").at("store").at("arena").erase(field);
    expect_rejected(std::move(json));
  }
  for(const auto* field : {"portfolioHandles",
                           "systemHandles",
                           "assetHandles",
                           "watchlistHandles",
                           "marketHandles",
                           "brokerHandles",
                           "profileHandles",
                           "strategyHandles"}) {
    SCOPED_TRACE(field);
    auto json = saved;
    json.at("pludux").at("documentState").erase(field);
    expect_rejected(std::move(json));
  }
}

TEST(ApplicationStateSerialization, RejectsMalformedSuppliedDefaults)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_portfolio(Portfolio{}));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  const auto saved = jsoncons::ojson::parse(stream.str());
  for(const auto* invalid :
      {"null", "\"10\"", "2.5", "-1", "4294967296", "9007199254740993.0"}) {
    SCOPED_TRACE(invalid);
    auto json = saved;
    json.at("pludux")
     .at("store")
     .at("arena")
     .at("portfolios")[0]["maximumOpenTrades"] =
     jsoncons::ojson::parse(invalid);
    auto input = std::stringstream{json.to_string()};
    EXPECT_THROW(pludux::apps::load_application_state_json(input),
                 std::exception);
  }
}

TEST(ApplicationStateSerialization, LoadsCurrentShapeWithDifferentVersion)
{
  auto state = ApplicationState{};
  const auto portfolio = *state.add_portfolio(Portfolio{});
  auto timeline = PortfolioTimeline{};
  timeline.append({.timestamp = 1, .capital = 42.0});
  ASSERT_TRUE(state.update_portfolio_results(
   portfolio, PortfolioResults{std::move(timeline), {}}));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  auto json = jsoncons::ojson::parse(stream.str());
  json.at("pludux")["$version"] = "different-version";
  auto input = std::stringstream{json.to_string()};

  const auto loaded = pludux::apps::load_application_state_json(input);
  ASSERT_EQ(loaded.get_portfolio_handles().size(), 1U);
  EXPECT_EQ(loaded.get_portfolio_results(loaded.get_portfolio_handles().front()),
            PortfolioResults{});
}

TEST(ApplicationStateSerialization, ResourceFieldsDefaultFromFreshObjects)
{
  auto state = ApplicationState{};
  state.add_market(pludux::backtest::Market{"Nondefault", 25.0, 5.0});
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  auto json = jsoncons::ojson::parse(stream.str());
  auto& market = json.at("pludux").at("store").at("arena").at("markets")[0];
  market.erase("name");
  market.erase("minOrderQuantity");
  market.erase("quantityStep");
  market["minimumQuantity"] = 99.0;
  auto input = std::stringstream{json.to_string()};

  const auto loaded = pludux::apps::load_application_state_json(input);
  EXPECT_EQ(loaded.get_market(loaded.get_market_handles().front()),
            pludux::backtest::Market{});
}

TEST(ApplicationStateSerialization, UsesStrictJsonSyntax)
{
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, ApplicationState{});
  auto commented = std::stringstream{"/* comment */" + stream.str()};
  EXPECT_THROW(pludux::apps::load_application_state_json(commented),
               std::exception);

  auto json = stream.str();
  json.insert(json.size() - 1, ",");
  auto trailing = std::stringstream{json};
  EXPECT_THROW(pludux::apps::load_application_state_json(trailing),
               std::exception);
}

TEST(ApplicationStateSerialization, RoundTripsPortfolioStrategySelection)
{
  auto state = ApplicationState{};
  state.add_asset(pludux::backtest::Asset{"Asset"});
  const auto asset = state.get_asset_handles().front();
  const auto watchlist =
   *state.add_watchlist(pludux::backtest::Watchlist{"List", {asset}});
  state.add_profile(pludux::backtest::Profile{});
  const auto strategy = add_strategy(state, "Strategy");
  const auto system = *state.add_system(
   pludux::backtest::System{"System",
                            watchlist,
                            state.get_profile_handles().front(),
                            {},
                            StrategySetup{strategy}});
  const auto portfolio = *state.add_portfolio(
   Portfolio{"Portfolio", 1'000.0, {}, {}, 10, 10, {}, {system}});
  const auto selected = pludux::apps::PortfolioStrategyKey{{system, asset}, 0};
  ASSERT_TRUE(state.select_portfolio_strategy(portfolio, selected));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  const auto serialized = stream.str();
  EXPECT_NE(serialized.find("\"portfolioStrategySelections\""),
            std::string::npos);
  EXPECT_NE(serialized.find("\"strategy\":0"), std::string::npos);
  EXPECT_EQ(serialized.find("portfolioStrategySetupSelections"),
            std::string::npos);
  const auto loaded = pludux::apps::load_application_state_json(stream);
  EXPECT_EQ(loaded.selected_portfolio_strategy(), selected);

  auto legacy = serialized;
  const auto active_field = std::string{",\"active\":true"};
  const auto active_position = legacy.find(active_field);
  ASSERT_NE(active_position, std::string::npos);
  legacy.erase(active_position, active_field.size());
  auto legacy_stream = std::stringstream{legacy};
  const auto legacy_loaded =
   pludux::apps::load_application_state_json(legacy_stream);
  EXPECT_EQ(legacy_loaded.selected_portfolio_strategy(), selected);
}

TEST(ApplicationStateSerialization, IgnoresLegacyPortfolioStrategySelections)
{
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, ApplicationState{});
  auto json = stream.str();
  const auto key = json.find("portfolioStrategySelections");
  ASSERT_NE(key, std::string::npos);
  json.replace(key,
               std::string{"portfolioStrategySelections"}.size(),
               "portfolioStrategySetupSelections");
  auto input = std::stringstream{json};
  EXPECT_NO_THROW(pludux::apps::load_application_state_json(input));
}

TEST(ApplicationStateSerialization, RoundTripsOrderedSystemStrategies)
{
  using pludux::backtest::AlternateStrategyActivation;

  auto state = ApplicationState{};
  add_strategy(state, "Main");
  add_strategy(state, "Alternate");
  const auto main = state.get_strategy_handles()[0];
  const auto alternate = state.get_strategy_handles()[1];
  const auto performance = pludux::backtest::StrategyPerformanceConfig{
   pludux::backtest::StrategyPerformanceHistoryPolicy{
    pludux::backtest::StrategyPerformanceHistoryMode::RollingWindow, 25, 0.99}};
  ASSERT_TRUE(state.add_system(
   pludux::backtest::System{"Ordered",
                            {},
                            {},
                            performance,
                            StrategySetup{main},
                            {{StrategySetup{alternate},
                              AlternateStrategyActivation::Failsafe}}}));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  const auto json = stream.str();
  EXPECT_NE(json.find("\"mainSetup\""), std::string::npos);
  EXPECT_NE(json.find("\"failsafeStrategies\""), std::string::npos);
  EXPECT_NE(json.find("PREVIOUS_STRATEGY_ENTRY_FILTERED_POSITION"),
            std::string::npos);
  EXPECT_NE(json.find("\"DISTRIBUTION.BETA_BERNOULLI\""), std::string::npos);
  EXPECT_NE(json.find("\"DISTRIBUTION.GAMMA_INVERSE_GAMMA\""),
            std::string::npos);
  const auto loaded = pludux::apps::load_application_state_json(stream);
  const auto& system = loaded.get_system(loaded.get_system_handles().front());
  EXPECT_EQ(system.main_setup().strategy_handle(), main);
  EXPECT_EQ(system.alternate_strategies().front().setup().strategy_handle(), alternate);
  EXPECT_EQ(system.alternate_strategies().front().activation(),
            AlternateStrategyActivation::Failsafe);
  EXPECT_EQ(system.strategy_performance(), performance);

  for(const auto& [canonical, legacy] :
      {std::pair{"DISTRIBUTION.BETA_BERNOULLI", "BETA_BERNOULLI"},
       std::pair{"DISTRIBUTION.GAMMA_INVERSE_GAMMA", "GAMMA_INVERSE_GAMMA"}}) {
    auto legacy_json = json;
    const auto method = legacy_json.find(canonical);
    ASSERT_NE(method, std::string::npos);
    legacy_json.replace(method, std::string{canonical}.size(), legacy);
    auto legacy_stream = std::stringstream{legacy_json};
    EXPECT_THROW(pludux::apps::load_application_state_json(legacy_stream),
                 std::exception);
  }
}

TEST(ApplicationStateSerialization, DefaultsMissingSystemFields)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_system(pludux::backtest::System{}));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  auto json = stream.str();
  const auto key = json.find("\"mainSetup\"");
  ASSERT_NE(key, std::string::npos);
  json.replace(
   key, std::string{"\"mainSetup\""}.size(), "\"removedMainSetup\"");
  auto input = std::stringstream{json};
  const auto loaded = pludux::apps::load_application_state_json(input);
  EXPECT_EQ(loaded.get_system(loaded.get_system_handles().front()),
            pludux::backtest::System{});
}

TEST(ApplicationStateSerialization, DefaultsAlternateWithoutActivation)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_system(pludux::backtest::System{"", {}, {}, {}, StrategySetup{}, {{}}}));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  auto json = stream.str();
  const auto key = json.find("\"activation\"");
  ASSERT_NE(key, std::string::npos);
  json.replace(
   key, std::string{"\"activation\""}.size(), "\"removedActivation\"");
  auto input = std::stringstream{json};
  const auto loaded = pludux::apps::load_application_state_json(input);
  EXPECT_EQ(loaded.get_system(loaded.get_system_handles().front())
             .alternate_strategies()
             .front(),
            pludux::backtest::SystemAlternateStrategy{});
}

TEST(CommandHistory, StandaloneStrategyCrudAndReorderAreUndoable)
{
  auto state = pludux::apps::ApplicationState{};
  auto executor = pludux::apps::CommandExecutor{};
  executor.push(pludux::apps::EditCommand{
   "Add Strategy",
   [](auto& candidate) {
     add_strategy(candidate, "First");
     add_strategy(candidate, "Second");
   },
   std::nullopt});
  EXPECT_EQ(executor.execute(state),
            pludux::apps::ExecutionEffect::DocumentChanged);
  ASSERT_EQ(state.get_strategy_handles().size(), 2U);
  const auto first = state.get_strategy_handles().front();

  executor.push(pludux::apps::EditCommand{
   "Move Strategy Down",
   [](auto& candidate) { candidate.reorder_list_strategy(0, 1); },
   std::nullopt});
  EXPECT_EQ(executor.execute(state),
            pludux::apps::ExecutionEffect::DocumentChanged);
  EXPECT_EQ(state.get_strategy_handles().back(), first);

  executor.push(pludux::apps::UndoCommand{});
  EXPECT_EQ(executor.execute(state),
            pludux::apps::ExecutionEffect::DocumentChanged);
  EXPECT_EQ(state.get_strategy_handles().front(), first);
}

TEST(ApplicationStateSerialization, RoundTripsPortfolioLimitsIncludingZero)
{
  auto state = ApplicationState{};
  auto portfolio = Portfolio{};
  portfolio.maximum_open_trades(0);
  portfolio.maximum_combined_layers(0);
  ASSERT_TRUE(state.add_portfolio(std::move(portfolio)));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  const auto json = stream.str();
  EXPECT_NE(json.find("\"maximumOpenTrades\":0"), std::string::npos);
  EXPECT_NE(json.find("\"maximumCombinedLayers\":0"), std::string::npos);
  const auto loaded = pludux::apps::load_application_state_json(stream);
  const auto& value =
   loaded.get_portfolio(loaded.get_portfolio_handles().front());
  EXPECT_EQ(value.maximum_open_trades(), 0);
  EXPECT_EQ(value.maximum_combined_layers(), 0);
}

TEST(ApplicationStateSerialization, DefaultsMissingPortfolioLimits)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_portfolio(Portfolio{}));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  for(const auto* field : {"maximumOpenTrades", "maximumCombinedLayers"}) {
    auto json = stream.str();
    const auto key = json.find(field);
    ASSERT_NE(key, std::string::npos);
    json.replace(key, std::string{field}.size(), "removed");
    auto input = std::stringstream{json};
    const auto loaded = pludux::apps::load_application_state_json(input);
    const auto& portfolio =
     loaded.get_portfolio(loaded.get_portfolio_handles().front());
    EXPECT_EQ(portfolio.maximum_open_trades(),
              Portfolio{}.maximum_open_trades());
    EXPECT_EQ(portfolio.maximum_combined_layers(),
              Portfolio{}.maximum_combined_layers());
  }
}

TEST(ApplicationStateSerialization, RoundTripsPortfolioIntrabarPath)
{
  auto state = ApplicationState{};
  auto portfolio = Portfolio{};
  portfolio.intrabar_path(IntrabarPath::LowFirst);
  const auto handle = state.add_portfolio(portfolio);
  ASSERT_TRUE(handle);

  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  const auto saved = jsoncons::ojson::parse(stream.str());
  const auto& value =
   saved.at("pludux").at("store").at("arena").at("portfolios").at(0);
  EXPECT_EQ(value.at("intrabarPath").as<std::string>(), "LOW_FIRST");

  const auto loaded = pludux::apps::load_application_state_json(stream);
  EXPECT_EQ(loaded.get_portfolio(*handle), portfolio);
}

TEST(ApplicationStateSerialization, IgnoresLegacyStrategyIntrabarPath)
{
  auto state = ApplicationState{};
  const auto strategy = add_strategy(state, "Legacy");
  const auto portfolio = state.add_portfolio(Portfolio{});
  ASSERT_TRUE(portfolio);
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  auto saved = jsoncons::ojson::parse(stream.str());
  auto& arena = saved.at("pludux").at("store").at("arena");
  arena.at("portfolios").at(0).erase("intrabarPath");
  auto& config = arena.at("strategies").at(0).at("config");
  config["execution"] = jsoncons::ojson::object();
  config["execution"]["intrabarPath"] = "HIGH_FIRST";

  auto input = std::stringstream{saved.to_string()};
  const auto loaded = pludux::apps::load_application_state_json(input);
  EXPECT_EQ(loaded.get_strategy(strategy), state.get_strategy(strategy));
  EXPECT_EQ(loaded.get_portfolio(*portfolio).intrabar_path(),
            IntrabarPath::CandleDirection);
}

TEST(ApplicationStateSerialization, RejectsInvalidPortfolioIntrabarPath)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_portfolio(Portfolio{}));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  auto saved = jsoncons::ojson::parse(stream.str());
  auto& portfolio =
   saved.at("pludux").at("store").at("arena").at("portfolios").at(0);
  for(const auto* value : {"null", R"("INVALID")", "2"}) {
    SCOPED_TRACE(value);
    portfolio["intrabarPath"] = jsoncons::ojson::parse(value);
    auto input = std::stringstream{saved.to_string()};
    EXPECT_THROW(pludux::apps::load_application_state_json(input),
                 std::exception);
  }
}

TEST(ApplicationStateSerialization, RoundTripsProfileCapitalPolicy)
{
  auto state = ApplicationState{};
  auto profile = pludux::backtest::Profile{};
  profile.drawdown_adjustment(
   pludux::backtest::DrawdownAdjustment{true, 0.10, 0.0, 0.20});
  profile.insufficient_cash_policy(
   pludux::backtest::InsufficientCashPolicy::CapToAvailableCash);
  state.add_profile(std::move(profile));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  const auto json = stream.str();
  EXPECT_NE(json.find("\"notionalEquityReduction\":0.2"), std::string::npos);
  EXPECT_EQ(json.find("\"executionFilter\""), std::string::npos);
  const auto loaded = pludux::apps::load_application_state_json(stream);
  const auto& loaded_profile =
   loaded.get_profile(loaded.get_profile_handles().front());
  EXPECT_DOUBLE_EQ(
   loaded_profile.drawdown_adjustment().notional_equity_reduction(), 0.20);
  EXPECT_EQ(loaded_profile.insufficient_cash_policy(),
            pludux::backtest::InsufficientCashPolicy::CapToAvailableCash);
}

TEST(ApplicationStateSerialization,
     DefaultsMissingProfileCapitalFieldsAndIgnoresLegacyFilter)
{
  auto state = ApplicationState{};
  state.add_profile(pludux::backtest::Profile{});
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  for(const auto* field :
      {"notionalEquityReduction", "insufficientCashPolicy"}) {
    auto json = stream.str();
    const auto key = json.find(field);
    ASSERT_NE(key, std::string::npos);
    json.replace(key, std::string{field}.size(), "removed");
    auto input = std::stringstream{json};
    const auto loaded = pludux::apps::load_application_state_json(input);
    EXPECT_EQ(loaded.get_profile(loaded.get_profile_handles().front()),
              pludux::backtest::Profile{});
  }
  auto json = stream.str();
  const auto key = json.find("\"positionSizing\"");
  ASSERT_NE(key, std::string::npos);
  json.insert(key, "\"executionFilter\":{\"method\":\"ALWAYS\"},");
  auto input = std::stringstream{json};
  EXPECT_NO_THROW(pludux::apps::load_application_state_json(input));
}

TEST(ApplicationStateSerialization, OmitsProfileCapitalProtectionFromPortfolio)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_portfolio(Portfolio{}));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);

  const auto json = stream.str();
  EXPECT_EQ(json.find("\"drawdownAdjustment\""), std::string::npos);
  EXPECT_EQ(json.find("\"insufficientCashPolicy\""), std::string::npos);
}

TEST(ApplicationStateSerialization, RoundTripsOrderedPortfolioEntryComparators)
{
  auto state = ApplicationState{};
  const auto comparators =
   std::vector<pludux::backtest::PortfolioEntryComparator>{
    {pludux::MultiplyNode<pludux::backtest::RequestedOrderMethodContext>{
      pludux::backtest::RequestedOrderDirectionNode{},
      pludux::DivideNode<pludux::backtest::RequestedOrderMethodContext>{
       pludux::SubtractNode<pludux::backtest::RequestedOrderMethodContext>{
        pludux::backtest::RequestedOrderPriceNode{},
        pludux::LookbackNode<pludux::backtest::RequestedOrderMethodContext>{
         pludux::CloseNode{}, 63}},
       pludux::backtest::RequestedOrderRiskDistanceNode{}}},
     pludux::backtest::PortfolioEntryComparatorOrder::HigherFirst},
    {pludux::DivideNode<pludux::backtest::RequestedOrderMethodContext>{
      pludux::backtest::RequestedRiskWithFeesNode{}, pludux::ValueNode{2.0}},
     pludux::backtest::PortfolioEntryComparatorOrder::LowerFirst},
    {pludux::backtest::StrategyPerformanceNode{
      pludux::backtest::StrategyPerformanceMetric::BayesianWinProbabilityLower95},
     pludux::backtest::PortfolioEntryComparatorOrder::HigherFirst}};
  auto portfolio = Portfolio{};
  portfolio.entry_comparators(comparators);
  ASSERT_TRUE(state.add_portfolio(std::move(portfolio)));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);

  const auto json = stream.str();
  EXPECT_NE(json.find("\"entryComparators\""), std::string::npos);
  EXPECT_NE(json.find("\"HIGHER_FIRST\""), std::string::npos);
  EXPECT_NE(json.find("\"LOWER_FIRST\""), std::string::npos);
  EXPECT_NE(json.find("\"OPERATOR.LOOKBACK\""), std::string::npos);
  EXPECT_NE(json.find("\"MARKET_DATA.CLOSE\""), std::string::npos);
  EXPECT_NE(json.find("\"REQUESTED_ORDER.RISK_WITH_FEES\""), std::string::npos);
  EXPECT_NE(json.find("\"STRATEGY_PERFORMANCE.VALUE\""), std::string::npos);
  EXPECT_NE(json.find("\"BAYESIAN_WIN_PROBABILITY_LOWER_95\""),
            std::string::npos);

  const auto loaded = pludux::apps::load_application_state_json(stream);
  const auto& loaded_portfolio =
   loaded.get_portfolio(loaded.get_portfolio_handles().front());
  EXPECT_EQ(loaded_portfolio.entry_comparators(), comparators);
}

TEST(ApplicationStateSerialization, RejectsMalformedPortfolioEntryComparator)
{
  auto state = ApplicationState{};
  auto portfolio = Portfolio{};
  portfolio.entry_comparators(
   {{pludux::backtest::RequestedOrderPriceNode{},
     pludux::backtest::PortfolioEntryComparatorOrder::HigherFirst}});
  ASSERT_TRUE(state.add_portfolio(std::move(portfolio)));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  auto json = stream.str();
  const auto method = json.find("REQUESTED_ORDER.PRICE");
  ASSERT_NE(method, std::string::npos);
  json.replace(
   method, std::string{"REQUESTED_ORDER.PRICE"}.size(), "SERIES.REFERENCE");

  auto input = std::stringstream{json};
  EXPECT_THROW(pludux::apps::load_application_state_json(input),
               std::exception);
}

TEST(ApplicationStateSerialization, DefaultsMissingPortfolioEntryComparators)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_portfolio(Portfolio{}));
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  auto json = stream.str();
  const auto key = json.find("\"entryComparators\"");
  ASSERT_NE(key, std::string::npos);
  json.replace(key,
               std::string{"\"entryComparators\""}.size(),
               "\"removedEntryComparators\"");

  auto input = std::stringstream{json};
  const auto loaded = pludux::apps::load_application_state_json(input);
  EXPECT_EQ(loaded.get_portfolio(loaded.get_portfolio_handles().front())
             .entry_comparators(),
            Portfolio{}.entry_comparators());
}

TEST(ApplicationStateSerialization, RejectsRemovedUiStateSchema)
{
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, ApplicationState{});
  auto json = stream.str();
  const auto key = json.find("\"documentState\"");
  ASSERT_NE(key, std::string::npos);
  json.replace(key, std::string{"\"documentState\""}.size(), "\"uiState\"");
  auto input = std::stringstream{json};
  EXPECT_THROW(pludux::apps::load_application_state_json(input),
               std::exception);
}

TEST(ApplicationStateSerialization, UsesFixedBudgetWithoutLegacyAlias)
{
  auto state = ApplicationState{};
  state.add_profile(pludux::backtest::Profile{
   "Budget",
   pludux::backtest::PositionSizingNode{
    pludux::backtest::FixedBudgetPositionSizing{750.0}}});
  state.add_profile(pludux::backtest::Profile{
   "Risk",
   pludux::backtest::PositionSizingNode{
    pludux::backtest::RiskDistancePositionSizing{0.01}}});
  state.add_profile(pludux::backtest::Profile{
   "Quantity",
   pludux::backtest::PositionSizingNode{
    pludux::backtest::FixedQuantityPositionSizing{10.0}}});
  state.add_profile(pludux::backtest::Profile{
   "Equity",
   pludux::backtest::PositionSizingNode{
    pludux::backtest::EquityFractionPositionSizing{0.25}}});
  state.add_profile(pludux::backtest::Profile{
   "Kelly",
   pludux::backtest::PositionSizingNode{
    pludux::backtest::StrategyPerformanceBayesianKellySizing{}}});
  auto stream = std::stringstream{};
  pludux::apps::save_application_state_json(stream, state);
  const auto json = stream.str();
  EXPECT_NE(json.find("\"POSITION_SIZING.FIXED_BUDGET\""), std::string::npos);
  EXPECT_NE(json.find("\"POSITION_SIZING.RISK_DISTANCE\""), std::string::npos);
  EXPECT_NE(json.find("\"POSITION_SIZING.FIXED_QUANTITY\""), std::string::npos);
  EXPECT_NE(json.find("\"POSITION_SIZING.EQUITY_FRACTION\""),
            std::string::npos);
  EXPECT_NE(json.find("\"POSITION_SIZING.STRATEGY_PERFORMANCE_BAYESIAN_KELLY\""),
            std::string::npos);
  EXPECT_NE(json.find("\"budget\":750.0"), std::string::npos);
  EXPECT_EQ(json.find("FIXED_NOTIONAL"), std::string::npos);
  const auto loaded = pludux::apps::load_application_state_json(stream);
  const auto* budget =
   position_sizing_node_cast<pludux::backtest::FixedBudgetPositionSizing>(
    loaded.get_profile(loaded.get_profile_handles().front()).position_sizing());
  ASSERT_NE(budget, nullptr);
  EXPECT_DOUBLE_EQ(budget->budget(), 750.0);

  auto legacy_json = json;
  const auto method = legacy_json.find("POSITION_SIZING.FIXED_BUDGET");
  ASSERT_NE(method, std::string::npos);
  legacy_json.replace(
   method, std::string{"POSITION_SIZING.FIXED_BUDGET"}.size(), "FIXED_BUDGET");
  auto legacy = std::stringstream{legacy_json};
  EXPECT_THROW(pludux::apps::load_application_state_json(legacy),
               std::exception);
}

} // namespace
