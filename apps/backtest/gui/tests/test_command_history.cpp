#include <gtest/gtest.h>

#include <exception>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

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
using pludux::backtest::BacktestSetup;
using pludux::backtest::DrawdownAdjustment;
using pludux::backtest::FixedBudgetPositionSizing;
using pludux::backtest::Portfolio;
using pludux::backtest::PortfolioEntryComparator;
using pludux::backtest::PortfolioEntryComparatorOrder;
using pludux::backtest::PortfolioResults;
using pludux::backtest::PortfolioTimeline;
using pludux::backtest::PositionSizingNode;
using pludux::backtest::Profile;

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

TEST(ApplicationStateSerialization, RoundTripsOrderedBacktestSetups)
{
  auto state = ApplicationState{};
  const auto performance = pludux::backtest::StrategyPerformanceConfig{
   pludux::backtest::StrategyPerformanceHistoryPolicy{
    pludux::backtest::StrategyPerformanceHistoryMode::RollingWindow, 25, 0.99}};
  const auto main = BacktestSetup{pludux::backtest::StrategyStoreHandle{1, 1},
                                  pludux::backtest::ProfileStoreHandle{2, 1}};
  const auto failsafe = pludux::backtest::BacktestFailsafeSetup{
   BacktestSetup{pludux::backtest::StrategyStoreHandle{3, 1},
                 pludux::backtest::ProfileStoreHandle{4, 1}},
   pludux::backtest::FailsafeActivation::PreviousSetupFilteredPosition};
  ASSERT_TRUE(state.add_backtest(
   Backtest{"Ordered",
            {},
            performance,
            main,
            std::vector<pludux::backtest::BacktestFailsafeSetup>{failsafe}}));
  auto stream = std::stringstream{};

  save_application_state_json(stream, state);

  const auto serialized = stream.str();
  EXPECT_NE(serialized.find("\"mainSetup\""), std::string::npos);
  EXPECT_NE(serialized.find("\"failsafeSetups\""), std::string::npos);
  EXPECT_NE(serialized.find("\"PREVIOUS_SETUP_FILTERED_POSITION\""),
            std::string::npos);
  const auto performance_key = serialized.find("\"strategyPerformance\"");
  ASSERT_NE(performance_key, std::string::npos);
  EXPECT_EQ(serialized.find("\"strategyPerformance\"", performance_key + 1),
            std::string::npos);
  auto input = std::stringstream{serialized};
  const auto loaded = load_application_state_json(input);
  const auto& backtest =
   loaded.get_backtest(loaded.get_backtest_handles().front());
  EXPECT_EQ(backtest.main_setup(), main);
  EXPECT_EQ(backtest.failsafe_setups(),
            (std::vector<pludux::backtest::BacktestFailsafeSetup>{failsafe}));
  EXPECT_EQ(backtest.strategy_performance(), performance);
}

TEST(ApplicationStateSerialization, RejectsFailsafeWithoutActivation)
{
  auto state = ApplicationState{};
  auto backtest = Backtest{};
  backtest.failsafe_setups().emplace_back(BacktestSetup{});
  ASSERT_TRUE(state.add_backtest(std::move(backtest)));
  auto stream = std::stringstream{};
  save_application_state_json(stream, state);
  auto serialized = stream.str();

  const auto activation = serialized.find("\"activation\"");
  ASSERT_NE(activation, std::string::npos);
  serialized.replace(
   activation, std::string{"\"activation\""}.size(), "\"removedActivation\"");
  auto input = std::stringstream{serialized};

  EXPECT_THROW(load_application_state_json(input), std::exception);
}

TEST(ApplicationStateSerialization,
     RejectsPreviousPerSetupStrategyPerformanceShape)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_backtest(Backtest{}));
  auto stream = std::stringstream{};
  save_application_state_json(stream, state);
  auto serialized = stream.str();

  const auto root_key = serialized.find("\"strategyPerformance\"");
  ASSERT_NE(root_key, std::string::npos);
  serialized.replace(root_key,
                     std::string{"\"strategyPerformance\""}.size(),
                     "\"legacyStrategyPerformance\"");
  const auto main_setup = serialized.find("\"mainSetup\":{");
  ASSERT_NE(main_setup, std::string::npos);
  serialized.insert(main_setup + std::string{"\"mainSetup\":{"}.size(),
                    "\"strategyPerformance\":{},");
  auto input = std::stringstream{serialized};

  EXPECT_THROW(load_application_state_json(input), std::exception);
}

TEST(ApplicationStateSerialization, RejectsMissingRequiredBacktestSetupFields)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_backtest(Backtest{}));
  auto stream = std::stringstream{};
  save_application_state_json(stream, state);
  auto serialized = stream.str();
  const auto key = serialized.find("\"mainSetup\"");
  ASSERT_NE(key, std::string::npos);
  serialized.replace(
   key, std::string{"\"mainSetup\""}.size(), "\"removedMainSetup\"");
  auto input = std::stringstream{serialized};

  EXPECT_THROW(load_application_state_json(input), std::exception);
}

TEST(ApplicationStateSerialization, RoundTripsMaximumOpenTradesIncludingZero)
{
  auto state = ApplicationState{};
  auto portfolio = Portfolio{};
  portfolio.maximum_open_trades(0);
  ASSERT_TRUE(state.add_portfolio(std::move(portfolio)));
  auto stream = std::stringstream{};

  save_application_state_json(stream, state);

  const auto serialized = stream.str();
  EXPECT_NE(serialized.find("\"maximumOpenTrades\":0"), std::string::npos);
  auto input = std::stringstream{serialized};
  const auto loaded = load_application_state_json(input);
  ASSERT_EQ(loaded.get_portfolio_handles().size(), 1U);
  EXPECT_EQ(loaded.get_portfolio(loaded.get_portfolio_handles().front())
             .maximum_open_trades(),
            0);
}

TEST(ApplicationStateSerialization, RejectsMissingMaximumOpenTrades)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_portfolio(Portfolio{}));
  auto stream = std::stringstream{};
  save_application_state_json(stream, state);
  auto serialized = stream.str();
  const auto key = serialized.find("\"maximumOpenTrades\"");
  ASSERT_NE(key, std::string::npos);
  serialized.replace(key,
                     std::string{"\"maximumOpenTrades\""}.size(),
                     "\"removedMaximumOpenTrades\"");
  auto input = std::stringstream{serialized};

  EXPECT_THROW(load_application_state_json(input), std::exception);
}

TEST(ApplicationStateSerialization,
     RoundTripsMaximumCombinedLayersIncludingZero)
{
  auto state = ApplicationState{};
  auto portfolio = Portfolio{};
  portfolio.maximum_combined_layers(0);
  ASSERT_TRUE(state.add_portfolio(std::move(portfolio)));
  auto stream = std::stringstream{};

  save_application_state_json(stream, state);

  const auto serialized = stream.str();
  EXPECT_NE(serialized.find("\"maximumCombinedLayers\":0"), std::string::npos);
  auto input = std::stringstream{serialized};
  const auto loaded = load_application_state_json(input);
  ASSERT_EQ(loaded.get_portfolio_handles().size(), 1U);
  EXPECT_EQ(loaded.get_portfolio(loaded.get_portfolio_handles().front())
             .maximum_combined_layers(),
            0);
}

TEST(ApplicationStateSerialization, RejectsMissingMaximumCombinedLayers)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_portfolio(Portfolio{}));
  auto stream = std::stringstream{};
  save_application_state_json(stream, state);
  auto serialized = stream.str();
  const auto key = serialized.find("\"maximumCombinedLayers\"");
  ASSERT_NE(key, std::string::npos);
  serialized.replace(key,
                     std::string{"\"maximumCombinedLayers\""}.size(),
                     "\"removedMaximumCombinedLayers\"");
  auto input = std::stringstream{serialized};

  EXPECT_THROW(load_application_state_json(input), std::exception);
}

TEST(ApplicationStateSerialization, RoundTripsNotionalEquityReduction)
{
  auto state = ApplicationState{};
  auto portfolio = Portfolio{};
  portfolio.drawdown_adjustment(DrawdownAdjustment{true, 0.10, 0.0, 0.20});
  ASSERT_TRUE(state.add_portfolio(std::move(portfolio)));
  auto stream = std::stringstream{};

  save_application_state_json(stream, state);

  const auto serialized = stream.str();
  EXPECT_NE(serialized.find("\"notionalEquityReduction\":0.2"),
            std::string::npos);
  auto input = std::stringstream{serialized};
  const auto loaded = load_application_state_json(input);
  const auto& adjustment =
   loaded.get_portfolio(loaded.get_portfolio_handles().front())
    .drawdown_adjustment();
  EXPECT_DOUBLE_EQ(adjustment.notional_equity_reduction(), 0.20);
}

TEST(ApplicationStateSerialization, RejectsMissingNotionalEquityReduction)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_portfolio(Portfolio{}));
  auto stream = std::stringstream{};
  save_application_state_json(stream, state);
  auto serialized = stream.str();
  const auto key = serialized.find("\"notionalEquityReduction\"");
  ASSERT_NE(key, std::string::npos);
  serialized.replace(key,
                     std::string{"\"notionalEquityReduction\""}.size(),
                     "\"removedNotionalEquityReduction\"");
  auto input = std::stringstream{serialized};

  EXPECT_THROW(load_application_state_json(input), std::exception);
}

TEST(ApplicationStateSerialization, RoundTripsOrderedEntryComparators)
{
  auto state = ApplicationState{};
  auto portfolio = Portfolio{};
  portfolio.entry_comparators(
   {PortfolioEntryComparator{pludux::backtest::RequestedNotionalNode{},
                             PortfolioEntryComparatorOrder::HigherFirst},
    PortfolioEntryComparator{
     pludux::DivideNode{pludux::backtest::RequestedRiskWithFeesNode{},
                        pludux::ValueNode{2.0}},
     PortfolioEntryComparatorOrder::LowerFirst}});
  ASSERT_TRUE(state.add_portfolio(std::move(portfolio)));
  auto stream = std::stringstream{};

  save_application_state_json(stream, state);

  const auto serialized = stream.str();
  EXPECT_NE(serialized.find("\"entryComparators\""), std::string::npos);
  EXPECT_NE(serialized.find("\"HIGHER_FIRST\""), std::string::npos);
  EXPECT_NE(serialized.find("\"LOWER_FIRST\""), std::string::npos);
  EXPECT_NE(serialized.find("\"REQUESTED_NOTIONAL\""), std::string::npos);
  EXPECT_NE(serialized.find("\"REQUESTED_RISK_WITH_FEES\""), std::string::npos);
  auto input = std::stringstream{serialized};
  const auto loaded = load_application_state_json(input);
  const auto& loaded_portfolio =
   loaded.get_portfolio(loaded.get_portfolio_handles().front());
  ASSERT_EQ(loaded_portfolio.entry_comparators().size(), 2U);
  EXPECT_EQ(loaded_portfolio.entry_comparators()[0].order(),
            PortfolioEntryComparatorOrder::HigherFirst);
  EXPECT_EQ(loaded_portfolio.entry_comparators()[1].order(),
            PortfolioEntryComparatorOrder::LowerFirst);
}

TEST(ApplicationStateSerialization, RejectsMarketNodeInEntryComparator)
{
  auto state = ApplicationState{};
  auto portfolio = Portfolio{};
  portfolio.entry_comparators(
   {PortfolioEntryComparator{pludux::backtest::RequestedOrderPriceNode{},
                             PortfolioEntryComparatorOrder::HigherFirst}});
  ASSERT_TRUE(state.add_portfolio(std::move(portfolio)));
  auto stream = std::stringstream{};
  save_application_state_json(stream, state);
  auto serialized = stream.str();
  const auto method = serialized.find("REQUESTED_ORDER_PRICE");
  ASSERT_NE(method, std::string::npos);
  serialized.replace(
   method, std::string{"REQUESTED_ORDER_PRICE"}.size(), "CLOSE");
  auto input = std::stringstream{serialized};

  EXPECT_THROW(load_application_state_json(input), std::exception);
}

TEST(ApplicationStateSerialization, RejectsMissingEntryComparators)
{
  auto state = ApplicationState{};
  ASSERT_TRUE(state.add_portfolio(Portfolio{}));
  auto stream = std::stringstream{};
  save_application_state_json(stream, state);
  auto serialized = stream.str();
  const auto key = serialized.find("\"entryComparators\"");
  ASSERT_NE(key, std::string::npos);
  serialized.replace(key,
                     std::string{"\"entryComparators\""}.size(),
                     "\"removedEntryComparators\"");
  auto input = std::stringstream{serialized};

  EXPECT_THROW(load_application_state_json(input), std::exception);
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

TEST(ApplicationStateSerialization, UsesFixedBudgetWithoutLegacyAlias)
{
  auto state = ApplicationState{};
  state.add_profile(
   Profile{"Budget", PositionSizingNode{FixedBudgetPositionSizing{750.0}}});
  auto stream = std::stringstream{};
  save_application_state_json(stream, state);
  const auto serialized = stream.str();
  EXPECT_NE(serialized.find("\"FIXED_BUDGET\""), std::string::npos);
  EXPECT_NE(serialized.find("\"budget\":750.0"), std::string::npos);
  EXPECT_EQ(serialized.find("FIXED_NOTIONAL"), std::string::npos);

  auto input = std::stringstream{serialized};
  const auto loaded = load_application_state_json(input);
  ASSERT_EQ(loaded.get_profile_handles().size(), 1U);
  const auto profile = loaded.get_profile(loaded.get_profile_handles().front());
  const auto* budget = position_sizing_node_cast<FixedBudgetPositionSizing>(
   profile.position_sizing());
  ASSERT_NE(budget, nullptr);
  EXPECT_DOUBLE_EQ(budget->budget(), 750.0);

  auto legacy_json = serialized;
  const auto method = legacy_json.find("FIXED_BUDGET");
  ASSERT_NE(method, std::string::npos);
  legacy_json.replace(
   method, std::string{"FIXED_BUDGET"}.size(), "FIXED_NOTIONAL");
  auto legacy = std::stringstream{legacy_json};
  EXPECT_THROW(load_application_state_json(legacy), std::exception);
}

} // namespace
