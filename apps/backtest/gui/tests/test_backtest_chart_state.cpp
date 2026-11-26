module;

#include <gtest/gtest.h>

#include <optional>
#include <vector>

// This test target supplies its own primary interface for the chart partition.
export module pludux.apps.backtest;

export import :charts.backtest_chart_state;
import pludux.apps.backtest.portfolio_strategy_selections;
import pludux.backtest;

using pludux::apps::BacktestChartState;
using pludux::apps::BacktestChartViewportMode;
using pludux::apps::BacktestTopPlot;
using pludux::apps::is_drawdown_plot;
using pludux::apps::PortfolioStrategyKey;
using pludux::backtest::PortfolioStoreHandle;

TEST(BacktestChartState, DrawdownMetricsUseTheSeparateDrawdownPlot)
{
  EXPECT_TRUE(is_drawdown_plot(BacktestTopPlot::Drawdown));
  EXPECT_TRUE(is_drawdown_plot(BacktestTopPlot::PnlDrawdown));
  EXPECT_FALSE(is_drawdown_plot(BacktestTopPlot::Equity));
  EXPECT_FALSE(is_drawdown_plot(BacktestTopPlot::NetPnl));
  EXPECT_FALSE(is_drawdown_plot(BacktestTopPlot::ShadowReturn));
}

TEST(BacktestChartState, PortfolioInspectionUsesTheHoveredOrPinnedRow)
{
  auto state = BacktestChartState{};
  state.display(PortfolioStoreHandle{1, 1}, std::nullopt);
  EXPECT_FALSE(state.inspected_bar());
  ASSERT_TRUE(state.update_result_count(4));
  EXPECT_EQ(state.inspected_bar(), 3);

  state.update_inspection(1, true, false, false, false);
  EXPECT_EQ(state.inspected_bar(), 1);
  EXPECT_FALSE(state.pinned_bar());
  state.update_inspection(1, false, true, false, false);
  state.update_inspection(2, false, false, false, false);
  EXPECT_EQ(state.pinned_bar(), 1);
  EXPECT_EQ(state.inspected_bar(), 1);

  state.update_inspection(2, true, false, false, false);
  state.update_inspection(3, false, false, true, false);
  state.update_inspection(3, false, true, false, false);
  EXPECT_EQ(state.pinned_bar(), 1);

  state.update_inspection(2, true, false, false, false);
  state.update_inspection(2, false, true, false, false);
  EXPECT_FALSE(state.pinned_bar());
  EXPECT_EQ(state.inspected_bar(), 2);
  state.update_inspection(std::nullopt, false, false, false, false);
  EXPECT_EQ(state.inspected_bar(), 3);
}

TEST(BacktestChartState, ScopeChangesClearInspectionAndKeepTheirRowHeights)
{
  const auto portfolio = PortfolioStoreHandle{1, 1};
  const auto strategy = PortfolioStrategyKey{{{2, 1}, {3, 1}}, 0};
  auto state = BacktestChartState{};
  state.display(portfolio, std::nullopt);
  EXPECT_EQ(state.row_ratios(2), (std::vector{1.0f, 1.0f}));
  state.row_ratios(2)[0] = 2.0f;
  state.update_result_count(6);
  state.pin_bar(4);
  state.viewport_mode(BacktestChartViewportMode::FollowLatest);
  state.consume_fit_request();
  state.follow_bar_count(20);
  EXPECT_TRUE(state.consume_fit_request());
  state.display(portfolio, std::nullopt);
  EXPECT_EQ(state.pinned_bar(), 4);
  EXPECT_EQ(state.follow_bar_count(), 20);

  state.display(portfolio, strategy);
  state.update_result_count(3);
  EXPECT_EQ(state.inspected_bar(), 2);
  EXPECT_FALSE(state.pinned_bar());
  EXPECT_EQ(state.viewport_mode(), BacktestChartViewportMode::FitAll);
  EXPECT_EQ(state.row_ratios(2), (std::vector{1.0f, 4.0f}));
  state.row_ratios(2)[1] = 3.0f;

  state.display(portfolio, std::nullopt);
  EXPECT_EQ(state.row_ratios(2), (std::vector{2.0f, 1.0f}));
  state.reset_row_ratios();
  EXPECT_EQ(state.row_ratios(2), (std::vector{1.0f, 1.0f}));
  state.display(portfolio, strategy);
  EXPECT_EQ(state.row_ratios(2), (std::vector{1.0f, 3.0f}));
}

TEST(BacktestChartState, ReplacedResultsClearPendingAndPinnedRows)
{
  auto state = BacktestChartState{};
  state.display(PortfolioStoreHandle{1, 1}, std::nullopt);
  state.update_result_count(4);
  state.pin_bar(0);
  state.update_result_count(5);
  EXPECT_EQ(state.inspected_bar(), 0);
  state.update_result_count(4);
  EXPECT_EQ(state.inspected_bar(), 3);

  state.update_inspection(2, true, false, false, false);
  state.invalidate_results();
  EXPECT_FALSE(state.inspected_bar());
  state.update_result_count(4);
  state.update_inspection(std::nullopt, false, true, false, false);
  EXPECT_FALSE(state.pinned_bar());
  EXPECT_EQ(state.inspected_bar(), 3);
}
