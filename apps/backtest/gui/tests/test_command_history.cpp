#include <gtest/gtest.h>

#include <exception>
#include <optional>
#include <sstream>
#include <string>

import pludux.apps.backtest.application_state;
import pludux.apps.backtest.command_executor;
import pludux.apps.backtest.serialization;
import pludux.backtest;

namespace {

using pludux::apps::ApplicationState;
using pludux::apps::CommandExecutor;
using pludux::apps::EditCommand;
using pludux::apps::ExecutionEffect;
using pludux::apps::load_application_state_json;
using pludux::apps::RedoCommand;
using pludux::apps::ReplaceApplicationCommand;
using pludux::apps::save_application_state_json;
using pludux::apps::UndoCommand;
using pludux::apps::ViewCommand;
using pludux::backtest::Backtest;
using pludux::backtest::Portfolio;
using pludux::backtest::PortfolioResults;
using pludux::backtest::PortfolioTimeline;

auto add_backtest(ApplicationState& state, std::string name)
{
  auto backtest = Backtest{};
  backtest.name(std::move(name));
  return *state.add_backtest(std::move(backtest));
}

void rename_backtest(ApplicationState& state,
                     pludux::backtest::BacktestStoreHandle handle,
                     std::string name)
{
  auto backtest = state.get_backtest(handle);
  backtest.name(std::move(name));
  ASSERT_TRUE(state.update_backtest(handle, std::move(backtest)));
}

TEST(CommandHistory, ViewSelectionPreservesRedoAndAvoidsDocumentEffect)
{
  auto state = ApplicationState{};
  const auto first = add_backtest(state, "First");
  const auto second = add_backtest(state, "Second");
  state.select_backtest(first);
  auto executor = CommandExecutor{};

  executor.push(EditCommand{
   "Rename Backtest",
   [first](auto& candidate) { rename_backtest(candidate, first, "Changed"); },
   std::nullopt});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::DocumentChanged);
  executor.push(UndoCommand{});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::DocumentChanged);
  ASSERT_TRUE(executor.can_redo());

  executor.push(ViewCommand{
   [second](auto& candidate) { candidate.select_backtest(second); }});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::ViewChanged);
  EXPECT_EQ(state.selected_backtest_handle(), second);
  EXPECT_TRUE(executor.can_redo());
  EXPECT_FALSE(executor.can_undo());
}

TEST(CommandHistory, UndoAndRedoPreserveCurrentSelection)
{
  auto state = ApplicationState{};
  const auto edited = add_backtest(state, "Original");
  const auto selected = add_backtest(state, "Selected");
  auto executor = CommandExecutor{};

  executor.push(EditCommand{
   "Rename Backtest",
   [edited](auto& candidate) { rename_backtest(candidate, edited, "Changed"); },
   std::nullopt});
  executor.execute(state);
  state.select_backtest(selected);

  executor.push(UndoCommand{});
  executor.execute(state);
  EXPECT_EQ(state.get_backtest(edited).name(), "Original");
  EXPECT_EQ(state.selected_backtest_handle(), selected);

  executor.push(RedoCommand{});
  executor.execute(state);
  EXPECT_EQ(state.get_backtest(edited).name(), "Changed");
  EXPECT_EQ(state.selected_backtest_handle(), selected);
}

TEST(CommandHistory, UndoNormalizesSelectionWhenItsTargetDisappears)
{
  auto state = ApplicationState{};
  auto executor = CommandExecutor{};
  executor.push(
   EditCommand{"Add Backtest",
               [](auto& candidate) { add_backtest(candidate, "Added"); },
               std::nullopt});
  executor.execute(state);
  const auto added = state.get_backtest_handles().front();
  state.select_backtest(added);

  executor.push(UndoCommand{});
  executor.execute(state);
  EXPECT_EQ(state.selected_backtest_handle(),
            pludux::backtest::BacktestStoreHandle{});
}

TEST(CommandHistory, NoOpDoesNotCreateHistoryOrClearRedo)
{
  auto state = ApplicationState{};
  const auto handle = add_backtest(state, "Original");
  auto executor = CommandExecutor{};
  executor.push(EditCommand{
   "Rename Backtest",
   [handle](auto& candidate) { rename_backtest(candidate, handle, "Changed"); },
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
  const auto portfolio_handle = *state.add_portfolio(Portfolio{});
  auto timeline = PortfolioTimeline{};
  timeline.append(PortfolioTimeline::Row{.timestamp = 1, .capital = 42.0});
  ASSERT_TRUE(state.update_portfolio_results(
   portfolio_handle, PortfolioResults{std::move(timeline), {}}));
  auto executor = CommandExecutor{};

  executor.push(EditCommand{"Edit Portfolio",
                            [portfolio_handle](auto& candidate) {
                              auto portfolio =
                               candidate.get_portfolio(portfolio_handle);
                              portfolio.name("Changed");
                              candidate.update_portfolio(portfolio_handle,
                                                         std::move(portfolio));
                            },
                            std::nullopt});
  executor.execute(state);
  EXPECT_TRUE(state.get_portfolio_results(portfolio_handle).timeline().empty());

  executor.push(UndoCommand{});
  executor.execute(state);
  ASSERT_EQ(state.get_portfolio_results(portfolio_handle).timeline().size(),
            1U);
  EXPECT_EQ(
   state.get_portfolio_results(portfolio_handle).timeline().row(0).capital,
   42.0);
}

TEST(CommandHistory, ExplicitMergeKeyCoalescesAdjacentEdits)
{
  auto state = ApplicationState{};
  const auto handle = add_backtest(state, "Original");
  auto executor = CommandExecutor{};

  for(const auto* name : {"First", "Second", "Final"}) {
    executor.push(EditCommand{"Rename Backtest",
                              [handle, name](auto& candidate) {
                                rename_backtest(candidate, handle, name);
                              },
                              std::string{"backtest-name"}});
    executor.execute(state);
  }

  EXPECT_EQ(executor.undo_size(), 1U);
  executor.push(UndoCommand{});
  executor.execute(state);
  EXPECT_EQ(state.get_backtest(handle).name(), "Original");
  executor.push(RedoCommand{});
  executor.execute(state);
  EXPECT_EQ(state.get_backtest(handle).name(), "Final");
}

TEST(CommandHistory, ViewCommandEndsCoalescingSequence)
{
  auto state = ApplicationState{};
  const auto handle = add_backtest(state, "Original");
  auto executor = CommandExecutor{};
  executor.push(EditCommand{
   "Rename Backtest",
   [handle](auto& candidate) { rename_backtest(candidate, handle, "First"); },
   std::string{"backtest-name"}});
  executor.execute(state);
  executor.push(ViewCommand{[](auto&) {}});
  executor.execute(state);
  executor.push(EditCommand{
   "Rename Backtest",
   [handle](auto& candidate) { rename_backtest(candidate, handle, "Second"); },
   std::string{"backtest-name"}});
  executor.execute(state);

  EXPECT_EQ(executor.undo_size(), 2U);
}

TEST(CommandHistory, HistoryKeepsNewestOneHundredEntries)
{
  auto state = ApplicationState{};
  auto executor = CommandExecutor{};
  for(auto index = 0; index < 101; ++index) {
    executor.push(
     EditCommand{"Add Backtest",
                 [index](auto& candidate) {
                   add_backtest(candidate, "Backtest " + std::to_string(index));
                 },
                 std::nullopt});
    executor.execute(state);
  }
  EXPECT_EQ(executor.undo_size(), CommandExecutor::history_limit);

  for(auto index = 0; index < 100; ++index) {
    executor.push(UndoCommand{});
    executor.execute(state);
  }
  ASSERT_EQ(state.get_backtest_handles().size(), 1U);
  EXPECT_EQ(state.get_backtest(state.get_backtest_handles().front()).name(),
            "Backtest 0");
}

TEST(CommandHistory, LabelsFollowEntriesAndReplacementClearsHistory)
{
  auto state = ApplicationState{};
  auto executor = CommandExecutor{};
  executor.push(
   EditCommand{"Add Backtest",
               [](auto& candidate) { add_backtest(candidate, "Added"); },
               std::nullopt});
  executor.execute(state);
  ASSERT_NE(executor.undo_label(), nullptr);
  EXPECT_EQ(*executor.undo_label(), "Add Backtest");

  executor.push(UndoCommand{});
  executor.execute(state);
  ASSERT_NE(executor.redo_label(), nullptr);
  EXPECT_EQ(*executor.redo_label(), "Add Backtest");

  executor.push(ReplaceApplicationCommand{[] {
    auto replacement = ApplicationState{};
    add_backtest(replacement, "Replacement");
    return replacement;
  }});
  EXPECT_EQ(executor.execute(state), ExecutionEffect::ApplicationReplaced);
  EXPECT_FALSE(executor.can_undo());
  EXPECT_FALSE(executor.can_redo());
  EXPECT_EQ(state.get_backtest_handles().size(), 1U);
}

TEST(ApplicationStateSerialization, RoundTripsDocumentAndViewSeparately)
{
  auto state = ApplicationState{};
  const auto handle = add_backtest(state, "Selected");
  state.select_backtest(handle);
  state.imgui_ini_settings("layout-data");
  auto stream = std::stringstream{};
  save_application_state_json(stream, state);
  const auto serialized = stream.str();
  EXPECT_NE(serialized.find("\"documentState\""), std::string::npos);
  EXPECT_NE(serialized.find("\"viewState\""), std::string::npos);
  EXPECT_EQ(serialized.find("\"uiState\""), std::string::npos);

  auto input = std::stringstream{serialized};
  const auto loaded = load_application_state_json(input);
  ASSERT_EQ(loaded.get_backtest_handles().size(), 1U);
  EXPECT_EQ(loaded.selected_backtest_handle(),
            loaded.get_backtest_handles().front());
  EXPECT_EQ(loaded.imgui_ini_settings(), "layout-data");
}

TEST(ApplicationStateSerialization, RejectsRemovedUiStateSchema)
{
  auto stream = std::stringstream{};
  save_application_state_json(stream, ApplicationState{});
  auto serialized = stream.str();
  const auto key = serialized.find("\"documentState\"");
  ASSERT_NE(key, std::string::npos);
  serialized.replace(
   key, std::string{"\"documentState\""}.size(), "\"uiState\"");
  auto legacy = std::stringstream{serialized};
  EXPECT_THROW(load_application_state_json(legacy), std::exception);
}

} // namespace
