module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <format>
#include <limits>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include "../ui/pludux_icons.hpp"

export module pludux.apps.backtest:windows.backtest_chart_window;

import pludux.backtest;
import pludux.apps.backtest.application_state;
import pludux.apps.backtest.portfolio_strategy_selections;
import :backtest_execution_status;
import :charts.backtest_chart_state;
import :window_context;

namespace pludux::apps {

struct PlotValue {
  std::string label;
  double value;
  std::uint32_t color;
};

struct PlotValueGroup {
  std::string label;
  std::vector<PlotValue> values;
};

using LegendSegment = std::pair<std::string, ImU32>;
using LegendLine = std::vector<LegendSegment>;

class StrategyPlotContext {
public:
  StrategyPlotContext(const SeriesEvaluationResults& series_results,
                      std::size_t results_size,
                      std::string item_id,
                      std::string item_label,
                      std::size_t inspected_index,
                      std::vector<PlotValue>& values)
  : series_results_{series_results}
  , results_size_{results_size}
  , item_id_{std::move(item_id)}
  , item_label_{std::move(item_label)}
  , inspected_index_{inspected_index}
  , values_{values}
  {
  }

  void render_plot_line(this const StrategyPlotContext& self,
                        const std::vector<double>& data,
                        std::uint32_t color)
  {
    const auto plot_spec =
     ImPlotSpec{ImPlotProp_LineColor,
                ImGui::ColorConvertU32ToFloat4(static_cast<ImU32>(color))};
    const auto plot_id = std::format("##{}", self.item_id_);
    ImPlot::PlotLine(
     plot_id.c_str(), data.data(), data.size(), 1.0, 0.0, plot_spec);
    self.record_value(data, color);
  }

  void render_plot_histogram(this const StrategyPlotContext& self,
                             const std::vector<double>& data,
                             std::uint32_t color)
  {
    const auto plot_spec =
     ImPlotSpec{ImPlotProp_FillColor,
                ImGui::ColorConvertU32ToFloat4(static_cast<ImU32>(color))};
    const auto plot_id = std::format("##{}", self.item_id_);
    ImPlot::PlotBars(
     plot_id.c_str(), data.data(), data.size(), 0.8, 0.0, plot_spec);
    self.record_value(data, color);
  }

  void render_plot_momentum_histogram(this const StrategyPlotContext& self,
                                      const std::vector<double>& data,
                                      const std::vector<std::uint32_t>& colors)
  {
    auto unique_colors = std::vector<std::uint32_t>{};
    for(const auto color : colors) {
      if(std::ranges::find(unique_colors, color) == unique_colors.end()) {
        unique_colors.push_back(color);
      }
    }

    ImGui::PushID(self.item_id_.c_str());
    for(auto color_index = std::size_t{0}; color_index < unique_colors.size();
        ++color_index) {
      const auto color = unique_colors[color_index];
      auto color_data = std::vector<double>(
       data.size(), std::numeric_limits<double>::quiet_NaN());
      for(auto i = std::size_t{0}; i < data.size() && i < colors.size(); ++i) {
        if(colors[i] == color) {
          color_data[i] = data[i];
        }
      }

      const auto plot_spec =
       ImPlotSpec{ImPlotProp_FillColor,
                  ImGui::ColorConvertU32ToFloat4(static_cast<ImU32>(color))};
      const auto label =
       std::format("##momentum_histogram_color_{}", color_index);
      ImPlot::PlotBars(label.c_str(),
                       color_data.data(),
                       color_data.size(),
                       0.8,
                       0.0,
                       plot_spec);
    }
    ImGui::PopID();

    const auto color = self.inspected_index_ < colors.size()
                        ? colors[self.inspected_index_]
                        : std::uint32_t{0xFFFFFFFF};
    self.record_value(data, color);
  }

  auto series_results(this const StrategyPlotContext& self,
                      const std::string& name)
   -> std::optional<std::reference_wrapper<const std::vector<double>>>
  {
    if(const auto results = self.series_results_.results(name)) {
      return results;
    }

    return std::nullopt;
  }

  auto results_size(this const StrategyPlotContext& self) -> std::size_t
  {
    return self.results_size_;
  }

private:
  void record_value(this const StrategyPlotContext& self,
                    const std::vector<double>& data,
                    std::uint32_t color)
  {
    const auto value = self.inspected_index_ < data.size()
                        ? data[self.inspected_index_]
                        : std::numeric_limits<double>::quiet_NaN();
    self.values_.push_back(PlotValue{self.item_label_, value, color});
  }

  const SeriesEvaluationResults& series_results_;
  std::size_t results_size_;
  std::string item_id_;
  std::string item_label_;
  std::size_t inspected_index_;
  std::vector<PlotValue>& values_;
};

export class BacktestChartWindow {
public:
  BacktestChartWindow()
  : bullish_color_{0.5, 1, 0, 1}
  , bearish_color_{1, 0, 0.5, 1}
  , risk_color_{1, 0, 0, 0.25}
  , reward_color_{0, 1, 0, 0.25}
  , trailing_stop_color_{1, 0., 0., 0.9}
  {
  }

  void render(this BacktestChartWindow& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto portfolio_handle = app_state.selected_portfolio_handle();
    const auto portfolio_ptr =
     app_state.get_portfolio_if_present(portfolio_handle);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{2.0f, 2.0f});
    ImGui::Begin("Chart", nullptr);
    ImGui::PopStyleVar();

    if(!portfolio_ptr) {
       self.chart_state_.clear_display_backtest_strategy();
      self.render_status_panel(
       PLUDUX_ICON_CHART,
       "No portfolio selected",
       "Select a portfolio to inspect its execution and "
       "market data.");
      ImGui::End();
      return;
    }

    const auto& portfolio = *portfolio_ptr;
    const auto selected_strategy = app_state.selected_portfolio_strategy();
    if(!selected_strategy) {
       self.chart_state_.clear_display_backtest_strategy();
      self.render_status_panel(
       PLUDUX_ICON_WARNING,
        "No portfolio backtest strategies",
        "Add or select a valid strategy in this portfolio.");
      ImGui::End();
      return;
    }
    const auto run = selected_strategy->run;
    const auto strategy_index = selected_strategy->strategy_index;
     const auto system_handle = run.system_handle;
    const auto* system_ptr = app_state.get_system_if_present(system_handle);
    if(!system_ptr) {
       self.chart_state_.clear_display_backtest_strategy();
      self.render_status_panel(
       PLUDUX_ICON_WARNING,
        "Missing portfolio strategy",
        "Select an available strategy in the portfolio tree.");
      ImGui::End();
      return;
    }
    const auto& system = *system_ptr;
    if(strategy_index >= system.strategy_count()) {
       self.chart_state_.clear_display_backtest_strategy();
      self.render_status_panel(PLUDUX_ICON_WARNING,
                                "Missing backtest strategy",
                                "Select an available strategy in the portfolio "
                               "tree.");
      ImGui::End();
      return;
    }
    self.chart_state_.display_backtest_strategy(portfolio_handle, *selected_strategy);

    const auto execution_status =
     context.backtest_execution_status(portfolio_handle);

    if(!app_state.is_portfolio_ready(portfolio)) {
      self.render_incomplete_status(app_state, portfolio);
      ImGui::End();
      return;
    }

    const auto asset_handle = run.asset_handle;
    const auto& asset = app_state.get_asset(asset_handle);
    const auto* strategy = app_state.get_strategy_if_present(
     system.strategy_handle(strategy_index));
    if(!strategy) {
      self.render_incomplete_status(app_state, portfolio);
      ImGui::End();
      return;
    }
    const auto& model = app_state.get_model(strategy->model_handle());

    if(asset.size() == 0) {
      self.render_status_panel(
       PLUDUX_ICON_WARNING,
       "No market data",
       "The selected asset is valid but contains no bars to chart.");
      ImGui::End();
      return;
    }

    const auto& portfolio_results =
     app_state.get_portfolio_results(portfolio_handle);
    const auto* backtest_results = portfolio_results.backtest(run);
    if(!backtest_results) {
      self.render_status_panel(PLUDUX_ICON_WAITING,
                               "Waiting for results",
                               "The backtest result is not available yet.");
      ImGui::End();
      return;
    }
    const auto& backtest_timelines = backtest_results->timeline();
    const auto& backtest_series_results =
      backtest_results->series_results(strategy_index);
    const auto timeline_size = backtest_timelines.size();
    auto backtest_timestamps = std::vector<double>(timeline_size);
    for(auto index = std::size_t{}; index < timeline_size; ++index) {
      backtest_timestamps[index] =
       static_cast<double>(backtest_timelines.market_timestamp(index));
    }

    if(timeline_size == 0) {
      const auto failed = execution_status && execution_status->phase() ==
                                               BacktestExecutionPhase::Failed;
      self.render_status_panel(
       failed ? PLUDUX_ICON_WARNING : PLUDUX_ICON_WAITING,
       failed ? "Backtest failed before producing results"
              : "Waiting for results",
       failed ? execution_status->error_message()
              : "The chart will populate as soon as the first bar is "
                "processed.");
      ImGui::End();
      return;
    }

    const auto& union_timeline = portfolio_results.timeline();
    if(!union_timeline.empty() &&
       backtest_timelines.market_timestamp(timeline_size - 1) <
        union_timeline.row(union_timeline.size() - 1).timestamp) {
      ImGui::SameLine();
      ImGui::TextDisabled("Stale price: valued at the backtest's last close");
    }

    const auto inspected_index =
     self.chart_state_.pinned_bar()
      .or_else([&self] { return self.hovered_bar_; })
      .value_or(timeline_size - 1);
    const auto chart_position = ImGui::GetCursorScreenPos();
    const auto chart_size = ImGui::GetContentRegionAvail();

    const auto& plots = model.plots();
    const auto no_overlays_view = std::views::filter(
     [](const auto& plot_group) { return !plot_group.is_overlay(); });
    const auto additional_plots_count =
     self.chart_state_.show_indicators()
      ? std::ranges::distance(plots | no_overlays_view)
      : 0;
    const auto backtest_running =
     execution_status &&
     execution_status->phase() == BacktestExecutionPhase::Running;
    const auto viewport_mode = self.chart_state_.viewport_mode();

    const auto axis_x_flags = ImPlotAxisFlags{ImPlotAxisFlags_None};
    auto axis_y_flags = ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_Opposite |
                        ImPlotAxisFlags_Foreground |
                        ImPlotAxisFlags_NoHighlight;
    const auto not_last_x_flags =
     ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoHighlight;

    auto crosshair_state = CrosshairState{};
    self.legend_hovered_ = false;
    const auto base_row_count = 2 + (self.chart_state_.show_volume() ? 1 : 0);
    const auto total_row_count =
     static_cast<std::size_t>(additional_plots_count) + base_row_count;
    auto& row_ratios = self.chart_state_.row_ratios(total_row_count);
    const auto fit_requested = self.chart_state_.consume_fit_request();
    const auto update_automatic_view = backtest_running || fit_requested;
    if(update_automatic_view) {
      axis_y_flags |= ImPlotAxisFlags_AutoFit;
    }

    constexpr auto chart_fit_padding = ImVec2{0.0f, 0.15f};
    ImPlot::PushStyleVar(ImPlotStyleVar_FitPadding, chart_fit_padding);
    if(ImPlot::BeginSubplots("##BacktestChart",
                             static_cast<int>(row_ratios.size()),
                             1,
                             ImVec2{-1, -1},
                             ImPlotSubplotFlags_LinkAllX |
                              ImPlotSubplotFlags_NoMenus,
                             row_ratios.data())) {
      constexpr auto plot_size = ImVec2{-1, 0};
      constexpr auto plot_flags =
       ImPlotFlags_NoLegend | ImPlotFlags_NoBoxSelect | ImPlotFlags_NoMenus;
      auto current_row = std::size_t{0};
      const auto setup_x_axis = [&](bool last_row) {
        ImPlot::SetupAxis(
         ImAxis_X1,
         nullptr,
         axis_x_flags | (last_row ? ImPlotAxisFlags_None : not_last_x_flags));
        ImPlot::SetupAxisFormat(
         ImAxis_X1, date_formatter, &backtest_timestamps);
        if(!update_automatic_view) {
          return;
        }

        const auto latest_bar = static_cast<double>(timeline_size - 1);
        if(viewport_mode == BacktestChartViewportMode::FitAll) {
          const auto right_padding_bars =
           std::clamp(static_cast<double>(timeline_size) * 0.05, 3.0, 20.0);
          ImPlot::SetupAxisLimits(
           ImAxis_X1, -0.5, latest_bar + right_padding_bars, ImPlotCond_Always);
        } else {
          constexpr auto right_padding_bars = 20.0;
          const auto visible_bar_count =
           static_cast<double>(self.chart_state_.follow_bar_count());
          const auto maximum = latest_bar + right_padding_bars;
          const auto minimum =
           std::max(-0.5, latest_bar - (visible_bar_count - 1.0));
          ImPlot::SetupAxisLimits(
           ImAxis_X1, minimum, maximum, ImPlotCond_Always);
        }
      };

      if(ImPlot::BeginPlot("##TopPlot", plot_size, plot_flags)) {
        setup_x_axis(++current_row == total_row_count);

        switch(self.chart_state_.top_plot()) {
        case BacktestTopPlot::Equity:
          ImPlot::SetupAxis(ImAxis_Y1, "% Equity", axis_y_flags);
          ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");
          self.plot_equity(portfolio_results.timeline(),
                           backtest_timestamps,
                           portfolio.initial_capital());
          break;
        case BacktestTopPlot::Drawdown:
          ImPlot::SetupAxis(ImAxis_Y1, "% Drawdown", axis_y_flags);
          ImPlot::SetupAxisFormat(ImAxis_Y1, "%.2f");
          self.plot_drawdown(portfolio_results.timeline(), backtest_timestamps);
          break;
        case BacktestTopPlot::ShadowReturn:
          ImPlot::SetupAxis(ImAxis_Y1, "Shadow return", axis_y_flags);
          ImPlot::SetupAxisFormat(ImAxis_Y1, "%.2f");
           self.plot_shadow_return(backtest_timelines, strategy_index);
          break;
        case BacktestTopPlot::FrequentistPerformance:
          ImPlot::SetupAxis(ImAxis_Y1, "Win / mean", axis_y_flags);
          ImPlot::SetupAxisFormat(ImAxis_Y1, "%.2f");
           self.plot_frequentist_performance(backtest_timelines, strategy_index);
          break;
        case BacktestTopPlot::CurrentStreaks:
          ImPlot::SetupAxis(ImAxis_Y1, "Current streak", axis_y_flags);
          ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");
           self.plot_strategy_streaks(backtest_timelines, strategy_index);
          break;
        case BacktestTopPlot::BayesianWin:
          ImPlot::SetupAxis(ImAxis_Y1, "Bayesian win", axis_y_flags);
          ImPlot::SetupAxisFormat(ImAxis_Y1, "%.2f");
           self.plot_bayesian_win(backtest_timelines, strategy_index);
          break;
        case BacktestTopPlot::BayesianPayoff:
          ImPlot::SetupAxis(ImAxis_Y1, "Bayesian payoff", axis_y_flags);
          ImPlot::SetupAxisFormat(ImAxis_Y1, "%.2f");
           self.plot_bayesian_payoff(backtest_timelines, strategy_index);
          break;
        }
        self.render_top_plot_legend(portfolio_results.timeline(),
                                    backtest_timelines,
                                    inspected_index,
                                     strategy_index);

        self.record_crosshair_plot(
         crosshair_state, timeline_size, self.chart_state_.pinned_bar());
        ImPlot::EndPlot();
      }

      if(ImPlot::BeginPlot("##main_plots", plot_size, plot_flags)) {
        setup_x_axis(++current_row == total_row_count);

        ImPlot::SetupAxis(ImAxis_Y1, "Price", axis_y_flags);
        ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");

        if(self.chart_state_.show_risk()) {
          self.plot_position(
            "Trade Positions", backtest_timelines, strategy_index);
        }
        self.plot_ohlc("OHLC", asset, timeline_size);
        auto overlay_value_groups = std::vector<PlotValueGroup>{};
        if(self.chart_state_.show_indicators()) {
          self.overlays_plots(
            context, *selected_strategy, inspected_index, overlay_value_groups);
        }
        if(self.chart_state_.show_trades()) {
          self.plot_signal(
            "Trade Signals", backtest_timelines, asset, strategy_index);
        }
        self.render_price_legend(asset, inspected_index, overlay_value_groups);

        self.record_crosshair_plot(
         crosshair_state, timeline_size, self.chart_state_.pinned_bar());

        ImPlot::EndPlot();
      }

      if(self.chart_state_.show_volume() &&
         ImPlot::BeginPlot("##VolumePlot", plot_size, plot_flags)) {
        setup_x_axis(++current_row == total_row_count);

        ImPlot::SetupAxis(
         ImAxis_Y1, "Volume", axis_y_flags | ImPlotAxisFlags_LockMin);
        ImPlot::SetupAxisFormat(ImAxis_Y1, volume_formatter);

        self.plot_volume("Volume", asset, timeline_size);
        self.render_volume_legend(asset, inspected_index);

        self.record_crosshair_plot(
         crosshair_state, timeline_size, self.chart_state_.pinned_bar());

        ImPlot::EndPlot();
      }

      if(self.chart_state_.show_indicators()) {
        auto plot_index = 0;
        for(const auto& plot_group : plots | no_overlays_view) {
          const auto plot_id = std::format("##IndicatorPlot{}", plot_index);

          if(ImPlot::BeginPlot(plot_id.c_str(), plot_size, plot_flags)) {
            setup_x_axis(++current_row == total_row_count);

            ImPlot::SetupAxis(
             ImAxis_Y1, plot_group.name().c_str(), axis_y_flags);

            auto plot_values = std::vector<PlotValue>{};
            auto item_index = std::size_t{0};
            for(const auto& plot_method : plot_group.items()) {
              const auto item_id =
               std::format("indicator_{}_{}", plot_index, item_index);
              const auto item_label =
               self.plot_item_label(plot_method, item_index);
              const auto context_for_plot =
               StrategyPlotContext{backtest_series_results,
                                   timeline_size,
                                   item_id,
                                   item_label,
                                   inspected_index,
                                   plot_values};
              plot_method(context_for_plot);
              ++item_index;
            }
            self.render_plot_value_legend(plot_group.name(), plot_values);

            self.record_crosshair_plot(
             crosshair_state, timeline_size, self.chart_state_.pinned_bar());
            ImPlot::EndPlot();
          }

          ++plot_index;
        }
      }

      const auto chart_hovered = ImPlot::IsSubplotsHovered();
      const auto mouse_wheel = ImGui::GetIO().MouseWheel;
      const auto fit_follow_span =
       chart_hovered && !self.legend_hovered_ &&
       viewport_mode == BacktestChartViewportMode::FollowLatest &&
       ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
      const auto zoom_follow_span =
       chart_hovered && !self.legend_hovered_ &&
       viewport_mode == BacktestChartViewportMode::FollowLatest &&
       mouse_wheel != 0.0f;
      ImPlot::EndSubplots();

      self.render_execution_badge(chart_position, chart_size, execution_status);
      self.render_chart_context_menu(chart_position, chart_size);

      if(fit_follow_span) {
        self.chart_state_.follow_bar_count(timeline_size);
      } else if(zoom_follow_span) {
        auto zoom_rate = static_cast<double>(ImPlot::GetInputMap().ZoomRate);
        if(mouse_wheel > 0.0f) {
          zoom_rate = -zoom_rate / (1.0 + 2.0 * zoom_rate);
        }
        const auto visible_bar_count =
         std::round(static_cast<double>(self.chart_state_.follow_bar_count()) *
                    (1.0 + zoom_rate));
        self.chart_state_.follow_bar_count(
         static_cast<std::size_t>(std::max(1.0, visible_bar_count)));
      }

      if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && chart_hovered &&
         self.chart_state_.pinned_bar()) {
        self.pending_unpin_ = true;
        self.pending_pin_bar_.reset();
      } else if(self.legend_hovered_) {
        self.pending_pin_bar_.reset();
        self.pending_unpin_ = false;
      } else if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && chart_hovered) {
        if(crosshair_state.hovered_bar) {
          self.pending_pin_bar_ = crosshair_state.hovered_bar;
        }
      }

      self.hovered_bar_ = crosshair_state.hovered_bar;
      if(ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        self.pending_pin_bar_.reset();
        self.pending_unpin_ = false;
      } else if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        if(self.pending_unpin_) {
          self.chart_state_.pin_bar(std::nullopt);
          self.pending_unpin_ = false;
        } else if(self.pending_pin_bar_) {
          self.chart_state_.pin_bar(self.pending_pin_bar_);
          self.pending_pin_bar_.reset();
        }
      }
      if(!self.legend_hovered_ || crosshair_state.pinned) {
        self.render_crosshair(crosshair_state, !self.legend_hovered_);
      }
    }
    ImPlot::PopStyleVar();
    ImGui::End();
  }

private:
  struct CrosshairState {
    std::vector<ImRect> plot_rects;
    std::optional<std::size_t> hovered_plot_index;
    std::optional<std::size_t> hovered_bar;
    std::optional<float> cursor_x;
    bool pinned{};
  };

  void record_crosshair_plot(this const BacktestChartWindow&,
                             CrosshairState& state,
                             std::size_t result_count,
                             std::optional<std::size_t> pinned_bar)
  {
    state.pinned = pinned_bar.has_value();
    const auto plot_position = ImPlot::GetPlotPos();
    const auto plot_size = ImPlot::GetPlotSize();
    state.plot_rects.emplace_back(
     plot_position,
     ImVec2{plot_position.x + plot_size.x, plot_position.y + plot_size.y});

    auto* plot = ImPlot::GetCurrentPlot();
    const auto any_x_axis_held =
     plot->Held ||
     ImPlot::AnyAxesHeld(&plot->Axes[ImAxis_X1], IMPLOT_NUM_X_AXES);
    const auto any_y_axis_held =
     plot->Held ||
     ImPlot::AnyAxesHeld(&plot->Axes[ImAxis_Y1], IMPLOT_NUM_Y_AXES);
    const auto plot_is_inspectable =
     plot->Hovered && !plot->Selecting && !plot->Items.Legend.Hovered;
    const auto crosshair_visible =
     plot_is_inspectable && !(any_x_axis_held || any_y_axis_held);

    if(plot_is_inspectable) {
      const auto mouse = ImPlot::GetPlotMousePos();
      if(result_count > 0) {
        const auto rounded_index =
         static_cast<std::ptrdiff_t>(std::llround(mouse.x));
        const auto clamped_index = std::clamp<std::ptrdiff_t>(
         rounded_index, 0, static_cast<std::ptrdiff_t>(result_count - 1));
        state.hovered_bar = static_cast<std::size_t>(clamped_index);
        const auto cursor_bar =
         pinned_bar.value_or(static_cast<std::size_t>(clamped_index));
        state.cursor_x =
         ImPlot::PlotToPixels(static_cast<double>(cursor_bar), 0.0).x;
      }
    }
    if(crosshair_visible) {
      state.hovered_plot_index = state.plot_rects.size() - 1;
    }
    if(pinned_bar && *pinned_bar < result_count && !state.cursor_x) {
      state.cursor_x =
       ImPlot::PlotToPixels(static_cast<double>(*pinned_bar), 0.0).x;
    }
  }

  void render_crosshair(this const BacktestChartWindow&,
                        const CrosshairState& state,
                        bool show_hover_crosshair)
  {
    if(!state.cursor_x) {
      return;
    }

    constexpr auto cursor_gap = 5.0f;
    const auto mouse_position = ImGui::GetIO().MousePos;
    const auto cursor_x = *state.cursor_x;
    const auto crosshair_color = ImPlot::GetStyleColorU32(ImPlotCol_Crosshairs);
    auto* draw_list = ImGui::GetWindowDrawList();
    const auto draw_dashed_line = [&](ImVec2 start, ImVec2 end) {
      constexpr auto dash_length = 4.0f;
      constexpr auto gap_length = 4.0f;
      const auto delta = ImVec2{end.x - start.x, end.y - start.y};
      const auto line_length = std::hypot(delta.x, delta.y);
      if(line_length <= 0.0f) {
        return;
      }

      const auto direction =
       ImVec2{delta.x / line_length, delta.y / line_length};
      for(auto offset = 0.0f; offset < line_length;
          offset += dash_length + gap_length) {
        const auto dash_end_offset =
         std::min(offset + dash_length, line_length);
        draw_list->AddLine(
         ImVec2{start.x + direction.x * offset, start.y + direction.y * offset},
         ImVec2{start.x + direction.x * dash_end_offset,
                start.y + direction.y * dash_end_offset},
         crosshair_color);
      }
    };
    const auto draw_vertical_line = [&](ImVec2 start, ImVec2 end) {
      if(state.pinned) {
        draw_list->AddLine(start, end, IM_COL32_WHITE);
      } else {
        draw_dashed_line(start, end);
      }
    };

    for(auto i = std::size_t{0}; i < state.plot_rects.size(); ++i) {
      const auto& plot_rect = state.plot_rects[i];
      if(cursor_x < plot_rect.Min.x || cursor_x > plot_rect.Max.x) {
        continue;
      }

      draw_list->PushClipRect(plot_rect.Min, plot_rect.Max, true);

      if(show_hover_crosshair && state.hovered_plot_index &&
         i == *state.hovered_plot_index) {
        draw_dashed_line(ImVec2{plot_rect.Min.x, mouse_position.y},
                         ImVec2{cursor_x - cursor_gap, mouse_position.y});
        draw_dashed_line(ImVec2{cursor_x + cursor_gap, mouse_position.y},
                         ImVec2{plot_rect.Max.x, mouse_position.y});
        draw_vertical_line(ImVec2{cursor_x, plot_rect.Min.y},
                           ImVec2{cursor_x, mouse_position.y - cursor_gap});
        draw_vertical_line(ImVec2{cursor_x, mouse_position.y + cursor_gap},
                           ImVec2{cursor_x, plot_rect.Max.y});
      } else {
        draw_vertical_line(ImVec2{cursor_x, plot_rect.Min.y},
                           ImVec2{cursor_x, plot_rect.Max.y});
      }

      draw_list->PopClipRect();
    }
  }

  BacktestChartState chart_state_{};
  std::optional<std::size_t> hovered_bar_{};
  std::optional<std::size_t> pending_pin_bar_{};
  bool pending_unpin_{};
  mutable bool legend_hovered_{false};

  ImVec4 bullish_color_;
  ImVec4 bearish_color_;

  ImVec4 risk_color_;
  ImVec4 reward_color_;

  ImVec4 trailing_stop_color_;

  static auto plot_source_label(const backtest::ErasedPlotSourceMethod<
                                 backtest::ErasedPlotMethodContext>& source,
                                std::string_view fallback,
                                std::size_t index) -> std::string
  {
    if(const auto* series =
        plot_source_method_cast<backtest::SeriesPlotSourceMethod>(source);
       series && !series->series_name().empty()) {
      return series->series_name();
    }
    if(plot_source_method_cast<backtest::ConstantPlotSourceMethod>(source)) {
      return std::format("Constant {}", index + 1);
    }
    return std::format("{} {}", fallback, index + 1);
  }

  static auto plot_item_label(
   const backtest::ErasedPlotMethod<backtest::ErasedPlotMethodContext>& method,
   std::size_t index) -> std::string
  {
    using PlotSource =
     backtest::ErasedPlotSourceMethod<backtest::ErasedPlotMethodContext>;
    if(plot_method_cast<backtest::HLinePlotMethod>(method)) {
      return std::format("Level {}", index + 1);
    }
    if(const auto* line =
        plot_method_cast<backtest::LinePlotMethod<PlotSource>>(method)) {
      return plot_source_label(line->source(), "Line", index);
    }
    if(const auto* histogram =
        plot_method_cast<backtest::HistogramPlotMethod<PlotSource>>(method)) {
      return plot_source_label(histogram->source(), "Histogram", index);
    }
    if(const auto* momentum =
        plot_method_cast<backtest::MomentumHistogramPlotMethod<PlotSource>>(
         method)) {
      return plot_source_label(momentum->source(), "Momentum", index);
    }
    return std::format("Plot {}", index + 1);
  }

  static auto format_plot_value(double value) -> std::string
  {
    return std::isfinite(value) ? std::format("{:.6g}", value)
                                : std::string{"â€”"};
  }

  void draw_plot_legend(this const BacktestChartWindow& self,
                        std::vector<LegendLine> lines)
  {
    const auto plot_position = ImPlot::GetPlotPos();
    const auto plot_size = ImPlot::GetPlotSize();
    const auto start_x = plot_position.x + 8.0f;
    const auto maximum_x = plot_position.x + plot_size.x - 8.0f;
    const auto line_height = ImGui::GetTextLineHeight() + 2.0f;
    auto cursor = ImVec2{start_x, plot_position.y + 7.0f};
    auto* draw_list = ImPlot::GetPlotDrawList();
    struct PositionedSegment {
      std::string text;
      ImU32 color;
      ImVec2 position;
      ImVec2 size;
    };
    auto positioned_segments = std::vector<PositionedSegment>{};
    auto content_maximum = cursor;

    for(const auto& line : lines) {
      cursor.x = start_x;
      for(const auto& [text, color] : line) {
        const auto size = ImGui::CalcTextSize(text.c_str());
        if(cursor.x > start_x && cursor.x + size.x > maximum_x) {
          cursor.x = start_x;
          cursor.y += line_height;
        }

        positioned_segments.push_back(
         PositionedSegment{text, color, cursor, size});
        content_maximum.x = std::max(content_maximum.x, cursor.x + size.x);
        content_maximum.y = std::max(content_maximum.y, cursor.y + size.y);
        cursor.x += size.x + 10.0f;
      }
      cursor.y += line_height;
    }

    constexpr auto background_padding = ImVec2{5.0f, 4.0f};
    const auto background_minimum =
     ImVec2{start_x - background_padding.x,
            plot_position.y + 7.0f - background_padding.y};
    const auto background_maximum =
     ImVec2{content_maximum.x + background_padding.x,
            content_maximum.y + background_padding.y};
    const auto background_hovered =
     ImGui::IsMouseHoveringRect(background_minimum, background_maximum);
    self.legend_hovered_ |= background_hovered;
    const auto background_color =
     background_hovered ? IM_COL32(10, 13, 18, 248) : IM_COL32(10, 13, 18, 125);

    ImPlot::PushPlotClipRect();
    draw_list->AddRectFilled(
     background_minimum, background_maximum, background_color, 4.0f);
    for(const auto& segment : positioned_segments) {
      draw_list->AddText(
       ImVec2{segment.position.x + 1.0f, segment.position.y + 1.0f},
       IM_COL32(0, 0, 0, 220),
       segment.text.c_str());
      draw_list->AddText(segment.position, segment.color, segment.text.c_str());
    }
    ImPlot::PopPlotClipRect();
  }

  void render_plot_value_legend(this const BacktestChartWindow& self,
                                const std::string& group_name,
                                const std::vector<PlotValue>& values)
  {
    auto line = LegendLine{
     {group_name.empty() ? "Indicator" : group_name, IM_COL32_WHITE}};
    for(const auto& value : values) {
      line.emplace_back(
       std::format("{} {}", value.label, format_plot_value(value.value)),
       static_cast<ImU32>(value.color));
    }
    self.draw_plot_legend({std::move(line)});
  }

  void
  render_price_legend(this const BacktestChartWindow& self,
                      const backtest::Asset& asset,
                      std::size_t index,
                      const std::vector<PlotValueGroup>& overlay_value_groups)
  {
    if(index >= asset.size()) {
      return;
    }

    const auto snapshot = asset.get_snapshot(index);
    const auto timestamp =
     format_datetime(static_cast<std::time_t>(snapshot.datetime()));
    const auto change = snapshot.close() - snapshot.open();
    const auto change_percent =
     snapshot.open() != 0.0 ? change / snapshot.open() * 100.0 : NAN;
    const auto candle_color = ImGui::GetColorU32(
     snapshot.close() >= snapshot.open() ? self.bullish_color_
                                         : self.bearish_color_);
    auto lines = std::vector<LegendLine>{
     {{timestamp, IM_COL32_WHITE},
      {std::format("O {:.6g}", snapshot.open()), candle_color},
      {std::format("H {:.6g}", snapshot.high()), candle_color},
      {std::format("L {:.6g}", snapshot.low()), candle_color},
      {std::format("C {:.6g}", snapshot.close()), candle_color},
      {std::format("{:+.6g}", change), candle_color},
      {std::isfinite(change_percent) ? std::format("{:+.2f}%", change_percent)
                                     : std::string{"\u2014"},
       candle_color}}};
    for(const auto& group : overlay_value_groups) {
      auto line = LegendLine{
       {group.label.empty() ? "Overlay" : group.label, IM_COL32_WHITE}};
      for(const auto& value : group.values) {
        line.emplace_back(
         std::format("{} {}", value.label, format_plot_value(value.value)),
         static_cast<ImU32>(value.color));
      }
      lines.push_back(std::move(line));
    }
    self.draw_plot_legend(std::move(lines));
  }

  void render_equity_legend(this const BacktestChartWindow& self,
                            const backtest::BacktestTimeline& timeline,
                            std::size_t index)
  {
    if(index >= timeline.size()) {
      return;
    }

    const auto equity = timeline.equity(index);
    const auto initial_capital = timeline.initial_capital(index);
    const auto percentage =
     initial_capital != 0.0 ? equity / initial_capital * 100.0 : NAN;
    const auto color = ImGui::GetColorU32(
     percentage >= 100.0 ? self.bullish_color_ : self.bearish_color_);
    self.draw_plot_legend(
     {{{"Equity", IM_COL32_WHITE},
       {format_currency(equity), color},
       {std::isfinite(percentage) ? std::format("{:.2f}%", percentage)
                                  : std::string{"â€”"},
        color}}});
  }

  static auto plot_item_color(const char* label) -> ImU32
  {
    const auto* item = ImPlot::GetItem(label);
    return item ? item->Color : IM_COL32_WHITE;
  }

  void
  render_top_plot_legend(this const BacktestChartWindow& self,
                         const backtest::PortfolioTimeline& portfolio_timeline,
                         const backtest::BacktestTimeline& backtest_timeline,
                         std::size_t index,
                      std::size_t strategy_index)
  {
    if(index >= backtest_timeline.size()) {
      return;
    }

    const auto value_segment =
     [](std::string_view label, double value, ImU32 color) -> LegendSegment {
      return {std::format("{} {}", label, format_plot_value(value)), color};
    };
    const auto& strategy_state = backtest_timeline.strategy_state(index, strategy_index);
     const auto& performance = strategy_state.model_performance;
    auto line = LegendLine{};

    switch(self.chart_state_.top_plot()) {
    case BacktestTopPlot::Equity:
      self.render_equity_legend(backtest_timeline, index);
      return;
    case BacktestTopPlot::Drawdown: {
      line.emplace_back("Drawdown", IM_COL32_WHITE);
      const auto timestamp = backtest_timeline.market_timestamp(index);
      const auto& rows = portfolio_timeline.rows();
      const auto row = std::ranges::lower_bound(
       rows, timestamp, {}, &backtest::PortfolioTimeline::Row::timestamp);
      const auto value = row != rows.end() && row->timestamp == timestamp
                          ? row->drawdown
                          : std::numeric_limits<double>::quiet_NaN();
      line.push_back(
       value_segment("Drawdown", value, plot_item_color("Drawdown")));
      break;
    }
    case BacktestTopPlot::ShadowReturn: {
      auto value = std::numeric_limits<double>::quiet_NaN();
        const auto& closed = strategy_state.model_closed_positions;
      if(!closed.empty()) {
        value = closed.back().return_ratio();
       } else if(const auto& open = strategy_state.model_open_position) {
        value = open->unrealized_return_ratio();
      }
      line.emplace_back("Shadow return", IM_COL32_WHITE);
      line.push_back(
       value_segment("Return", value, plot_item_color("Shadow return")));
      break;
    }
    case BacktestTopPlot::FrequentistPerformance:
      line.emplace_back("Frequentist performance", IM_COL32_WHITE);
      line.push_back(value_segment(
       "Win", performance.win_rate(), plot_item_color("Win rate")));
      line.push_back(value_segment("Break-even",
                                   performance.break_even_rate(),
                                   plot_item_color("Break-even rate")));
      line.push_back(value_segment(
       "Loss", performance.loss_rate(), plot_item_color("Loss rate")));
      line.push_back(value_segment("Mean return",
                                   performance.mean_return(),
                                   plot_item_color("Mean return")));
      break;
    case BacktestTopPlot::CurrentStreaks:
      line.emplace_back("Current streaks", IM_COL32_WHITE);
      line.push_back(
       value_segment("Winning",
                     static_cast<double>(performance.current_winning_streak()),
                     plot_item_color("Winning streak")));
      line.push_back(
       value_segment("Losing",
                     static_cast<double>(performance.current_losing_streak()),
                     plot_item_color("Losing streak")));
      break;
    case BacktestTopPlot::BayesianWin: {
      const auto& posterior = performance.win_probability_posterior();
      line.emplace_back("Bayesian win", IM_COL32_WHITE);
      line.push_back(value_segment(
       "Posterior", posterior.probability, plot_item_color("Posterior win")));
      line.push_back(value_segment(
       "95% lower", posterior.lower_95, plot_item_color("95% credible")));
      line.push_back(value_segment(
       "upper", posterior.upper_95, plot_item_color("95% credible")));
      break;
    }
    case BacktestTopPlot::BayesianPayoff: {
      const auto& winning = performance.winning_payoff_posterior();
      const auto& losing = performance.losing_payoff_posterior();
      line.emplace_back("Bayesian payoff", IM_COL32_WHITE);
      line.push_back(value_segment(
       "Winning", winning.mean, plot_item_color("Winning payoff")));
      line.push_back(value_segment(
       "95%", winning.lower_95, plot_item_color("Winning 95% credible")));
      line.push_back(value_segment(
       "to", winning.upper_95, plot_item_color("Winning 95% credible")));
      line.push_back(
       value_segment("Losing", losing.mean, plot_item_color("Losing payoff")));
      line.push_back(value_segment(
       "95%", losing.lower_95, plot_item_color("Losing 95% credible")));
      line.push_back(value_segment(
       "to", losing.upper_95, plot_item_color("Losing 95% credible")));
      break;
    }
    }

    self.draw_plot_legend({std::move(line)});
  }

  void render_volume_legend(this const BacktestChartWindow& self,
                            const backtest::Asset& asset,
                            std::size_t index)
  {
    if(index >= asset.size()) {
      return;
    }

    const auto snapshot = asset.get_snapshot(index);
    const auto color = ImGui::GetColorU32(snapshot.close() >= snapshot.open()
                                           ? self.bullish_color_
                                           : self.bearish_color_);
    self.draw_plot_legend(
     {{{"Volume", IM_COL32_WHITE},
       {format_compact_integer(snapshot.volume()), color}}});
  }

  void render_execution_badge(this const BacktestChartWindow&,
                              ImVec2 chart_position,
                              ImVec2 chart_size,
                              const BacktestExecutionStatus* execution_status)
  {
    if(!execution_status ||
       execution_status->phase() == BacktestExecutionPhase::Completed) {
      return;
    }

    const auto failed =
     execution_status->phase() == BacktestExecutionPhase::Failed;
    const auto text =
     failed
      ? std::string{PLUDUX_ICON_WARNING "  Failed"}
      : std::format("Running {:.0f}%", execution_status->progress() * 100.0f);
    const auto text_size = ImGui::CalcTextSize(text.c_str());
    constexpr auto padding = ImVec2{8.0f, 5.0f};
    const auto maximum = ImVec2{chart_position.x + chart_size.x - 10.0f,
                                chart_position.y + chart_size.y - 10.0f};
    const auto minimum = ImVec2{maximum.x - text_size.x - padding.x * 2.0f,
                                maximum.y - text_size.y - padding.y * 2.0f};
    auto* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(minimum,
                             maximum,
                             failed ? IM_COL32(120, 28, 40, 225)
                                    : IM_COL32(22, 62, 92, 220),
                             4.0f);
    draw_list->AddText(ImVec2{minimum.x + padding.x, minimum.y + padding.y},
                       IM_COL32_WHITE,
                       text.c_str());

    if(failed && ImGui::IsMouseHoveringRect(minimum, maximum)) {
      ImGui::BeginTooltip();
      ImGui::TextWrapped("%s", execution_status->error_message().c_str());
      ImGui::EndTooltip();
    }
  }

  void render_chart_context_menu(this BacktestChartWindow& self,
                                 ImVec2 chart_position,
                                 ImVec2 chart_size)
  {
    const auto chart_maximum =
     ImVec2{chart_position.x + chart_size.x, chart_position.y + chart_size.y};
    if(ImGui::IsMouseHoveringRect(chart_position, chart_maximum) &&
       ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
      ImGui::OpenPopup("##backtest_chart_context");
    }

    if(!ImGui::BeginPopup("##backtest_chart_context")) {
      return;
    }

    const auto viewport_mode = self.chart_state_.viewport_mode();
    const auto fit_all_selected =
     viewport_mode == BacktestChartViewportMode::FitAll;
    if(ImGui::MenuItem("Fit all bars", nullptr, fit_all_selected)) {
      self.chart_state_.viewport_mode(BacktestChartViewportMode::FitAll);
      self.chart_state_.request_fit();
    }
    const auto follow_latest_selected =
     viewport_mode == BacktestChartViewportMode::FollowLatest;
    const auto follow_latest_label = std::format(
     "Follow latest bars ({})", self.chart_state_.follow_bar_count());
    if(ImGui::MenuItem(
        follow_latest_label.c_str(), nullptr, follow_latest_selected)) {
      self.chart_state_.viewport_mode(BacktestChartViewportMode::FollowLatest);
      self.chart_state_.request_fit();
    }
    ImGui::BeginDisabled(!self.chart_state_.pinned_bar());
    if(ImGui::MenuItem("Clear pin")) {
      self.chart_state_.pin_bar(std::nullopt);
    }
    ImGui::EndDisabled();
    if(ImGui::MenuItem("Reset row heights")) {
      self.chart_state_.reset_row_ratios();
    }

    ImGui::Separator();
    if(ImGui::BeginMenu("Top subplot")) {
      const auto top_plot = self.chart_state_.top_plot();
      const auto menu_item = [&self, top_plot](const char* label,
                                               BacktestTopPlot plot) {
        if(ImGui::MenuItem(label, nullptr, top_plot == plot) &&
           top_plot != plot) {
          self.chart_state_.top_plot(plot);
        }
      };
      menu_item("Equity", BacktestTopPlot::Equity);
      menu_item("Drawdown", BacktestTopPlot::Drawdown);
      menu_item("Shadow return", BacktestTopPlot::ShadowReturn);
      menu_item("Frequentist performance",
                BacktestTopPlot::FrequentistPerformance);
      menu_item("Current streaks", BacktestTopPlot::CurrentStreaks);
      menu_item("Bayesian win", BacktestTopPlot::BayesianWin);
      menu_item("Bayesian payoff", BacktestTopPlot::BayesianPayoff);
      ImGui::EndMenu();
    }

    ImGui::Separator();
    auto layers_changed = false;
    layers_changed |=
     ImGui::MenuItem("Volume", nullptr, &self.chart_state_.show_volume());
    layers_changed |=
     ImGui::MenuItem("Trade events", nullptr, &self.chart_state_.show_trades());
    layers_changed |= ImGui::MenuItem(
     "Risk and targets", nullptr, &self.chart_state_.show_risk());
    layers_changed |= ImGui::MenuItem(
     "Strategy plots", nullptr, &self.chart_state_.show_indicators());
    if(layers_changed) {
      self.chart_state_.request_fit();
    }
    ImGui::EndPopup();
  }

  void render_status_panel(this const BacktestChartWindow&,
                           const char* icon,
                           const std::string& title,
                           const std::string& detail)
  {
    const auto available = ImGui::GetContentRegionAvail();
    const auto content_height = ImGui::GetTextLineHeightWithSpacing() * 4.0f;
    ImGui::Dummy(
     ImVec2{0.0f, std::max(0.0f, (available.y - content_height) * 0.42f)});

    const auto heading = std::format("{}  {}", icon, title);
    const auto heading_width = ImGui::CalcTextSize(heading.c_str()).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                         std::max(0.0f, (available.x - heading_width) / 2));
    ImGui::TextUnformatted(heading.c_str());

    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + available.x * 0.75f);
    const auto detail_width =
     ImGui::CalcTextSize(detail.c_str(), nullptr, false, available.x * 0.75f).x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                         std::max(0.0f, (available.x - detail_width) / 2));
    ImGui::TextDisabled("%s", detail.c_str());
    ImGui::PopTextWrapPos();
  }

  void render_incomplete_status(this const BacktestChartWindow& self,
                                const ApplicationState& app_state,
                                const backtest::Portfolio& portfolio)
  {
    auto missing = std::vector<std::string_view>{};
    if(!app_state.get_market_if_present(portfolio.market_handle())) {
      missing.emplace_back("market");
    }
    if(!app_state.get_broker_if_present(portfolio.broker_handle())) {
      missing.emplace_back("broker");
    }
    for(const auto handle : portfolio.system_handles()) {
      const auto* system = app_state.get_system_if_present(handle);
      if(!system || !app_state.is_system_ready(*system)) {
        missing.emplace_back("Strategy");
        break;
      }
    }

    auto missing_text = std::string{};
    for(auto index = std::size_t{0}; index < missing.size(); ++index) {
      if(index > 0) {
        missing_text += index + 1 == missing.size() ? " and " : ", ";
      }
      missing_text += missing[index];
    }

    self.render_status_panel(
     PLUDUX_ICON_WARNING,
     "Portfolio setup is incomplete",
     std::format("Choose the missing {} before results can be charted.",
                 missing_text));
  }

  static auto volume_formatter(double value, char* buff, int size, void*) -> int
  {
    return std::snprintf(
     buff, size, "%s", format_compact_integer(value).c_str());
  }

  static auto visible_index_range(std::size_t total_count)
   -> std::pair<std::size_t, std::size_t>
  {
    const auto limits = ImPlot::GetPlotLimits().X;
    const auto first = std::clamp<std::ptrdiff_t>(
     static_cast<std::ptrdiff_t>(std::floor(limits.Min)) - 1,
     0,
     static_cast<std::ptrdiff_t>(total_count));
    const auto last = std::clamp<std::ptrdiff_t>(
     static_cast<std::ptrdiff_t>(std::ceil(limits.Max)) + 2,
     first,
     static_cast<std::ptrdiff_t>(total_count));
    return {static_cast<std::size_t>(first), static_cast<std::size_t>(last)};
  }

  static auto
  date_formatter(double value, char* buff, int size, void* user_data) -> int
  {
    const auto& timestamps =
     *reinterpret_cast<const std::vector<double>*>(user_data);
    const auto index = static_cast<std::ptrdiff_t>(std::llround(value));
    if(index < 0 || index >= static_cast<std::ptrdiff_t>(timestamps.size())) {
      return std::snprintf(buff, size, "");
    }
    const auto formatted_datetime =
     format_datetime(static_cast<std::time_t>(timestamps[index]));
    return std::snprintf(buff, size, "%s", formatted_datetime.c_str());
  }

  void plot_ohlc(this const BacktestChartWindow& self,
                 const char* label_id,
                 const backtest::Asset& asset,
                 std::size_t total_count)
  {
    if(ImPlot::BeginItem(label_id) && total_count > 0) {
      ImPlot::GetCurrentItem()->Color = ImGui::GetColorU32(self.bullish_color_);

      auto* draw_list = ImPlot::GetPlotDrawList();
      constexpr double half_width = 0.3;
      const auto [first_visible, last_visible] =
       visible_index_range(total_count);
      for(auto i = first_visible; i < last_visible; ++i) {
        const auto snapshot = asset.get_snapshot(i);

        const auto open = snapshot.open();
        const auto high = snapshot.high();
        const auto low = snapshot.low();
        const auto close = snapshot.close();

        const auto open_left_pos = ImPlot::PlotToPixels(i - half_width, open);
        const auto open_right_pos = ImPlot::PlotToPixels(i + half_width, open);
        const auto close_left_pos = ImPlot::PlotToPixels(i - half_width, close);
        const auto close_right_pos =
         ImPlot::PlotToPixels(i + half_width, close);
        const auto low_pos = ImPlot::PlotToPixels(i, low);
        const auto high_pos = ImPlot::PlotToPixels(i, high);

        const auto color = ImGui::GetColorU32(
         open > close ? self.bearish_color_ : self.bullish_color_);

        draw_list->AddLine(
         low_pos,
         high_pos,
         color,
         ImMax(1.0f, ImAbs(open_left_pos.x - close_right_pos.x) / 10.0f));
        draw_list->AddRectFilled(open_left_pos, close_right_pos, color);

        const auto open_close_points = std::array{
         open_left_pos, close_left_pos, close_right_pos, open_right_pos};
        draw_list->AddPolyline(
         open_close_points.data(),
         open_close_points.size(),
         color,
         ImDrawFlags_Closed,
         ImMax(1.0f, ImAbs(open_left_pos.x - close_right_pos.x) / 10.0f));
      }

      if(ImPlot::FitThisFrame()) {
        for(auto i = std::size_t{0}; i < total_count; ++i) {
          const auto snapshot = asset.get_snapshot(i);

          ImPlot::FitPoint(ImPlotPoint(i, snapshot.low()));
          ImPlot::FitPoint(ImPlotPoint(i, snapshot.high()));
        }
      }

      ImPlot::EndItem();
    }
  }

  void plot_volume(this const BacktestChartWindow& self,
                   const char* label_id,
                   const backtest::Asset& asset,
                   std::size_t total_count)
  {
    if(ImPlot::BeginItem(label_id)) {
      ImPlot::GetCurrentItem()->Color = ImGui::GetColorU32(self.bullish_color_);

      if(ImPlot::FitThisFrame()) {
        for(auto i = std::size_t{0}; i < total_count; ++i) {
          const auto snapshot = asset.get_snapshot(i);

          ImPlot::FitPoint(ImPlotPoint(i, 0));
          ImPlot::FitPoint(ImPlotPoint(i, snapshot.volume()));
        }
      }

      auto* draw_list = ImPlot::GetPlotDrawList();

      constexpr auto half_width = 0.3;
      const auto [first_visible, last_visible] =
       visible_index_range(total_count);
      for(auto i = first_visible; i < last_visible; ++i) {
        const auto snapshot = asset.get_snapshot(i);

        const auto open = snapshot.open();
        const auto close = snapshot.close();
        const auto volume = snapshot.volume();

        ImU32 color = ImGui::GetColorU32(open > close ? self.bearish_color_
                                                      : self.bullish_color_);

        ImVec2 low_left_pos = ImPlot::PlotToPixels(i - half_width, 0);
        ImVec2 low_right_pos = ImPlot::PlotToPixels(i + half_width, 0);
        ImVec2 high_left_pos = ImPlot::PlotToPixels(i - half_width, volume);
        ImVec2 high_right_pos = ImPlot::PlotToPixels(i + half_width, volume);

        const auto line_points =
         std::array{low_left_pos, high_left_pos, high_right_pos, low_right_pos};
        draw_list->AddRectFilled(low_left_pos, high_right_pos, color);
        draw_list->AddPolyline(line_points.data(),
                               line_points.size(),
                               color,
                               ImDrawFlags_Closed,
                               1.0f);
      }

      ImPlot::EndItem();
    }
  }

  void plot_position(this const BacktestChartWindow& self,
                     const char* label_id,
                     const backtest::BacktestTimeline& backtest_timelines,
                           std::size_t strategy_index)
  {
    auto* draw_list = ImPlot::GetPlotDrawList();

    if(ImPlot::BeginItem(label_id)) {
      const auto timeline_size = backtest_timelines.size();
      const auto stop_line_color =
       ImGui::GetColorU32(self.trailing_stop_color_);
      auto take_profit_line_color = self.reward_color_;
      take_profit_line_color.w = 0.9f;
      const auto target_line_color = ImGui::GetColorU32(take_profit_line_color);
      auto stop_loss_line_points = std::vector<std::vector<ImVec2>>{};
      auto take_profit_line_points = std::vector<std::vector<ImVec2>>{};

      const auto flush_price_line = [&](std::vector<ImVec2>& line_points,
                                        ImU32 color) {
        if(line_points.size() > 1) {
          draw_list->AddPolyline(line_points.data(),
                                 line_points.size(),
                                 color,
                                 ImDrawFlags_None,
                                 1.4f);
        }

        line_points.clear();
      };

      const auto [first_visible, last_visible] =
       visible_index_range(timeline_size);
      for(auto i = first_visible; i < last_visible; ++i) {
        const auto& open_position = backtest_timelines.open_position(i);
        if(!open_position ||
           open_position->strategy_index() != strategy_index) {
          for(auto& points : stop_loss_line_points) {
            flush_price_line(points, stop_line_color);
          }
          for(auto& points : take_profit_line_points) {
            flush_price_line(points, target_line_color);
          }
          continue;
        }

        const auto avg_price = open_position->average_price();
        const auto risk_reference_price = open_position->risk_reference_price();
        const auto risk_boundary_price = open_position->risk_boundary_price();
        const auto& stop_loss_levels = open_position->stop_loss_levels();
        for(auto index = stop_loss_levels.size();
            index < stop_loss_line_points.size();
            ++index) {
          flush_price_line(stop_loss_line_points[index], stop_line_color);
        }
        stop_loss_line_points.resize(stop_loss_levels.size());
        const auto& take_profit_levels = open_position->take_profit_levels();
        for(auto index = take_profit_levels.size();
            index < take_profit_line_points.size();
            ++index) {
          flush_price_line(take_profit_line_points[index], target_line_color);
        }
        take_profit_line_points.resize(take_profit_levels.size());

        constexpr auto half_width = 0.5;
        const auto left_x = static_cast<double>(i) - half_width;
        const auto right_x =
         static_cast<double>(i) + (i == timeline_size - 1 ? 10.0 : half_width);

        const auto draw_price_band = [&](double boundary_price,
                                         double action_price,
                                         const ImVec4& color) {
          if(!std::isfinite(boundary_price) || !std::isfinite(action_price)) {
            return;
          }

          const auto risk_left_top_pos =
           ImPlot::PlotToPixels(left_x, std::max(boundary_price, action_price));
          const auto risk_right_bottom_pos = ImPlot::PlotToPixels(
           right_x, std::min(boundary_price, action_price));

          draw_list->AddRectFilled(
           risk_left_top_pos, risk_right_bottom_pos, ImGui::GetColorU32(color));
        };

        const auto append_price_line =
         [&](std::vector<ImVec2>& line_points, double price, ImU32 color) {
           if(!std::isfinite(price)) {
             flush_price_line(line_points, color);
             return;
           }

           const auto left_pos = ImPlot::PlotToPixels(left_x, price);
           const auto right_pos = ImPlot::PlotToPixels(right_x, price);

           if(!line_points.empty()) {
             const auto last_pos = line_points.back();
             constexpr auto connected_line_epsilon = 0.5f;
             const auto has_gap =
              ImAbs(last_pos.x - left_pos.x) > connected_line_epsilon;

             if(has_gap) {
               flush_price_line(line_points, color);
             } else if(last_pos.y != left_pos.y) {
               line_points.push_back(left_pos);
             }
           }

           if(line_points.empty()) {
             line_points.push_back(left_pos);
           }

           line_points.push_back(right_pos);
         };

        draw_price_band(
         risk_reference_price, risk_boundary_price, self.risk_color_);
        for(const auto& level : take_profit_levels) {
          if(level.active()) {
            draw_price_band(avg_price, level.price(), self.reward_color_);
          }
        }

        for(auto index = std::size_t{0}; index < stop_loss_levels.size();
            ++index) {
          append_price_line(stop_loss_line_points[index],
                            stop_loss_levels[index].active()
                             ? stop_loss_levels[index].effective_price()
                             : NAN,
                            stop_line_color);
        }
        for(auto index = std::size_t{0}; index < take_profit_levels.size();
            ++index) {
          append_price_line(take_profit_line_points[index],
                            take_profit_levels[index].active()
                             ? take_profit_levels[index].price()
                             : NAN,
                            target_line_color);
        }
      }

      for(auto& points : stop_loss_line_points) {
        flush_price_line(points, stop_line_color);
      }
      for(auto& points : take_profit_line_points) {
        flush_price_line(points, target_line_color);
      }

      ImPlot::EndItem();
    }
  }

  void plot_signal(this const BacktestChartWindow& self,
                   const char* label_id,
                   const backtest::BacktestTimeline& backtest_timelines,
                   const backtest::Asset& asset,
                    std::size_t strategy_index)
  {
    constexpr auto marker_offset = 50.0f;
    const auto marker_text_color =
     ImGui::GetColorU32(ImVec4{1.0f, 1.0f, 1.0f, 1.0f});
    auto* draw_list = ImPlot::GetPlotDrawList();

    if(ImPlot::BeginItem(label_id)) {
      const auto timeline_size = backtest_timelines.size();
      const auto plot_top = ImPlot::GetPlotPos().y;
      const auto plot_bottom = plot_top + ImPlot::GetPlotSize().y;
      const auto clamp_marker_y =
       [&](float y, float top_padding, float bottom_padding) {
         const auto minimum = plot_top + top_padding;
         const auto maximum = plot_bottom - bottom_padding;
         if(minimum > maximum) {
           return (minimum + maximum) * 0.5f;
         }

         return std::clamp(y, minimum, maximum);
       };

      const auto [first_visible, last_visible] =
       visible_index_range(timeline_size);
      for(auto i = first_visible; i < last_visible; ++i) {
        const auto snapshot = asset.get_snapshot(i);
        for(const auto& event : backtest_timelines.trade_events(i)) {
          if(event.strategy_index() != strategy_index) {
            continue;
          }

          if(event.is_rejected()) {
            const auto entry_low = snapshot.low();
            auto rejected_pos = ImPlot::PlotToPixels(i, entry_low);
            rejected_pos.y += marker_offset;
            const auto rejected_color = ImGui::GetColorU32(self.bearish_color_);
            constexpr auto rejected_marker_size = 5.0f;
            constexpr auto rejected_marker_thickness = 4.0f;
            constexpr auto rejected_hover_radius = 12.0f;
            rejected_pos.y = clamp_marker_y(
             rejected_pos.y, rejected_hover_radius, rejected_hover_radius);

            draw_list->AddLine(ImVec2{rejected_pos.x - rejected_marker_size,
                                      rejected_pos.y - rejected_marker_size},
                               ImVec2{rejected_pos.x + rejected_marker_size,
                                      rejected_pos.y + rejected_marker_size},
                               rejected_color,
                               rejected_marker_thickness);
            draw_list->AddLine(ImVec2{rejected_pos.x - rejected_marker_size,
                                      rejected_pos.y + rejected_marker_size},
                               ImVec2{rejected_pos.x + rejected_marker_size,
                                      rejected_pos.y - rejected_marker_size},
                               rejected_color,
                               rejected_marker_thickness);

            const auto hover_minimum =
             ImVec2{rejected_pos.x - rejected_hover_radius,
                    rejected_pos.y - rejected_hover_radius};
            const auto hover_maximum =
             ImVec2{rejected_pos.x + rejected_hover_radius,
                    rejected_pos.y + rejected_hover_radius};
            if(ImGui::IsMouseHoveringRect(hover_minimum, hover_maximum)) {
              const auto requested_quantity = std::abs(event.position_size());
              const auto order_value = requested_quantity * event.price();

              ImGui::BeginTooltip();
              ImGui::TextColored(self.bearish_color_, "Order rejected");
              ImGui::Separator();
              const auto strategy_label =
               event.strategy_index() == 0
                ? std::string{"Main"}
                : std::format("Failsafe {}", event.strategy_index());
              ImGui::Text("Strategy: %s", strategy_label.c_str());
              if(event.type() ==
                 backtest::TradeEvent::Type::rejected_maximum_open_trades) {
                ImGui::TextUnformatted("Reason: Maximum open trades reached");
              } else if(event.type() == backtest::TradeEvent::Type::
                                         rejected_maximum_combined_layers) {
                ImGui::TextUnformatted(
                 "Reason: Maximum combined layers reached");
              } else {
                ImGui::TextUnformatted("Reason: Insufficient cash");
              }
              ImGui::Text("Time: %s",
                          format_datetime(event.timestamp()).c_str());
              ImGui::Text("Requested quantity: %.6g", requested_quantity);
              ImGui::Text("Order price: %s",
                          format_currency(event.price()).c_str());
              ImGui::Text("Order value: %s",
                          format_currency(order_value).c_str());
              if(event.type() ==
                 backtest::TradeEvent::Type::rejected_insufficient_cash) {
                const auto available_cash = event.rejection_available_cash();
                const auto required_cash = event.rejection_required_cash();
                const auto estimated_fees = required_cash - order_value;
                const auto cash_shortfall = required_cash - available_cash;
                ImGui::Text("Estimated fees: %s",
                            format_currency(estimated_fees).c_str());
                ImGui::Separator();
                ImGui::Text("Cash required: %s",
                            format_currency(required_cash).c_str());
                ImGui::Text("Cash available: %s",
                            format_currency(available_cash).c_str());
                ImGui::TextColored(self.bearish_color_,
                                   "Cash shortfall: %s",
                                   format_currency(cash_shortfall).c_str());
              }
              ImGui::EndTooltip();
            }
          }

          if(event.is_entry() || event.is_scale_in()) {
            const auto entry_low = snapshot.low();

            auto entry_pos = ImPlot::PlotToPixels(i, entry_low);
            entry_pos.y += marker_offset;
            constexpr auto entry_marker_height = 10.0f;
            entry_pos.y = clamp_marker_y(
             entry_pos.y, entry_marker_height, ImGui::GetTextLineHeight());

            draw_list->AddTriangleFilled(
             ImVec2{entry_pos.x - 5, entry_pos.y},
             ImVec2{entry_pos.x + 5, entry_pos.y},
             ImVec2{entry_pos.x, entry_pos.y - entry_marker_height},
             ImGui::GetColorU32(self.bullish_color_));

            const auto trade_count_str = std::format("#{}", event.trade_id());
            const auto text_size = ImGui::CalcTextSize(trade_count_str.c_str());
            draw_list->AddText(
             ImVec2{entry_pos.x - text_size.x * 0.5f, entry_pos.y},
             marker_text_color,
             trade_count_str.c_str());
            constexpr auto entry_hover_radius = 12.0f;
            if(ImGui::IsMouseHoveringRect(
                ImVec2{entry_pos.x - entry_hover_radius,
                       entry_pos.y - entry_hover_radius},
                ImVec2{entry_pos.x + entry_hover_radius,
                       entry_pos.y + entry_hover_radius})) {
              const auto strategy_label =
               event.strategy_index() == 0
                ? std::string{"Main"}
                : std::format("Failsafe {}", event.strategy_index());
              ImGui::BeginTooltip();
              ImGui::TextUnformatted(event.is_scale_in() ? "Scale in"
                                                         : "Entry");
              ImGui::Separator();
              ImGui::Text("Strategy: %s", strategy_label.c_str());
              ImGui::Text("Time: %s",
                          format_datetime(event.timestamp()).c_str());
              ImGui::Text("Quantity: %.6g", std::abs(event.position_size()));
              ImGui::Text("Price: %s", format_currency(event.price()).c_str());
              ImGui::EndTooltip();
            }
          }

          if(event.is_exit() || event.is_scale_out()) {
            const auto exit_high = snapshot.high();

            auto exit_pos = ImPlot::PlotToPixels(i, exit_high);
            exit_pos.y -= marker_offset;
            constexpr auto exit_label_offset = 13.0f;
            constexpr auto full_exit_marker_height = 10.0f;
            exit_pos.y = clamp_marker_y(
             exit_pos.y, exit_label_offset, full_exit_marker_height);

            const auto exit_color = [&]() {
              if(event.type() == backtest::TradeEvent::Type::stop_loss) {
                return ImGui::GetColorU32(self.trailing_stop_color_);
              }

              if(event.type() == backtest::TradeEvent::Type::take_profit) {
                auto color = self.reward_color_;
                color.w = 0.9f;
                return ImGui::GetColorU32(color);
              }

              return ImGui::GetColorU32(self.bearish_color_);
            }();

            if(event.is_scale_out()) {
              constexpr auto scale_out_half_width = 4.0f;
              constexpr auto scale_out_height = 8.0f;
              draw_list->AddTriangle(
               ImVec2{exit_pos.x - scale_out_half_width, exit_pos.y},
               ImVec2{exit_pos.x + scale_out_half_width, exit_pos.y},
               ImVec2{exit_pos.x, exit_pos.y + scale_out_height},
               exit_color,
               2.0f);
            } else {
              draw_list->AddTriangleFilled(
               ImVec2{exit_pos.x - 5, exit_pos.y},
               ImVec2{exit_pos.x + 5, exit_pos.y},
               ImVec2{exit_pos.x, exit_pos.y + full_exit_marker_height},
               exit_color);
            }

            const auto trade_count_str = std::format("#{}", event.trade_id());
            const auto text_size = ImGui::CalcTextSize(trade_count_str.c_str());
            draw_list->AddText(
             ImVec2{exit_pos.x - text_size.x * 0.5f, exit_pos.y - 13},
             marker_text_color,
             trade_count_str.c_str());
          }
        }
      }

      ImPlot::EndItem();
    }
  }

  void plot_shadow_return(this const BacktestChartWindow&,
                          const backtest::BacktestTimeline& timeline,
                          std::size_t strategy_index)
  {
    auto xs = std::vector<double>(timeline.size());
    auto returns = std::vector<double>(
     timeline.size(), std::numeric_limits<double>::quiet_NaN());
    for(auto index = std::size_t{0}; index < timeline.size(); ++index) {
      xs[index] = static_cast<double>(index);
      const auto& strategy_state =
       timeline.strategy_state(index, strategy_index);
       const auto& closed = strategy_state.model_closed_positions;
      if(!closed.empty()) {
        returns[index] = closed.back().return_ratio();
      }
      if(std::isnan(returns[index])) {
         const auto& open = strategy_state.model_open_position;
        if(open) {
          returns[index] = open->unrealized_return_ratio();
        }
      }
    }
    ImPlot::PlotLine("Shadow return", xs.data(), returns.data(), xs.size());
  }

  void plot_frequentist_performance(this const BacktestChartWindow&,
                                    const backtest::BacktestTimeline& timeline,
                                     std::size_t strategy_index)
  {
    auto xs = std::vector<double>(timeline.size());
    auto win_rate = std::vector<double>(timeline.size());
    auto break_even_rate = std::vector<double>(timeline.size());
    auto loss_rate = std::vector<double>(timeline.size());
    auto mean_return = std::vector<double>(timeline.size());
    for(auto index = std::size_t{0}; index < timeline.size(); ++index) {
      xs[index] = static_cast<double>(index);
      const auto& performance =
        timeline.strategy_state(index, strategy_index).model_performance;
      win_rate[index] = performance.win_rate();
      break_even_rate[index] = performance.break_even_rate();
      loss_rate[index] = performance.loss_rate();
      mean_return[index] = performance.mean_return();
    }
    ImPlot::PlotLine("Win rate", xs.data(), win_rate.data(), xs.size());
    ImPlot::PlotLine(
     "Break-even rate", xs.data(), break_even_rate.data(), xs.size());
    ImPlot::PlotLine("Loss rate", xs.data(), loss_rate.data(), xs.size());
    ImPlot::PlotLine("Mean return", xs.data(), mean_return.data(), xs.size());
  }

  void plot_strategy_streaks(this const BacktestChartWindow&,
                             const backtest::BacktestTimeline& timeline,
                              std::size_t strategy_index)
  {
    auto xs = std::vector<double>(timeline.size());
    auto winning = std::vector<double>(timeline.size());
    auto losing = std::vector<double>(timeline.size());
    for(auto index = std::size_t{0}; index < timeline.size(); ++index) {
      xs[index] = static_cast<double>(index);
      const auto& performance =
        timeline.strategy_state(index, strategy_index).model_performance;
      winning[index] =
       static_cast<double>(performance.current_winning_streak());
      losing[index] = static_cast<double>(performance.current_losing_streak());
    }
    ImPlot::PlotLine("Winning streak", xs.data(), winning.data(), xs.size());
    ImPlot::PlotLine("Losing streak", xs.data(), losing.data(), xs.size());
  }

  void plot_bayesian_win(this const BacktestChartWindow&,
                         const backtest::BacktestTimeline& timeline,
                          std::size_t strategy_index)
  {
    auto xs = std::vector<double>(timeline.size());
    auto mean = std::vector<double>(timeline.size());
    auto lower = std::vector<double>(timeline.size());
    auto upper = std::vector<double>(timeline.size());
    for(auto index = std::size_t{0}; index < timeline.size(); ++index) {
      xs[index] = static_cast<double>(index);
      const auto& posterior =
       timeline.strategy_state(index, strategy_index)
         .model_performance.win_probability_posterior();
      mean[index] = posterior.probability;
      lower[index] = posterior.lower_95;
      upper[index] = posterior.upper_95;
    }
    ImPlot::PlotShaded(
     "95% credible", xs.data(), lower.data(), upper.data(), xs.size());
    ImPlot::PlotLine("Posterior win", xs.data(), mean.data(), xs.size());
  }

  void plot_bayesian_payoff(this const BacktestChartWindow&,
                            const backtest::BacktestTimeline& timeline,
                             std::size_t strategy_index)
  {
    auto xs = std::vector<double>(timeline.size());
    auto winning_mean = std::vector<double>(timeline.size());
    auto winning_lower = std::vector<double>(timeline.size());
    auto winning_upper = std::vector<double>(timeline.size());
    auto losing_mean = std::vector<double>(timeline.size());
    auto losing_lower = std::vector<double>(timeline.size());
    auto losing_upper = std::vector<double>(timeline.size());
    for(auto index = std::size_t{0}; index < timeline.size(); ++index) {
      xs[index] = static_cast<double>(index);
      const auto& performance =
        timeline.strategy_state(index, strategy_index).model_performance;
      const auto& winning = performance.winning_payoff_posterior();
      const auto& losing = performance.losing_payoff_posterior();
      winning_mean[index] = winning.mean;
      winning_lower[index] = winning.lower_95;
      winning_upper[index] = winning.upper_95;
      losing_mean[index] = losing.mean;
      losing_lower[index] = losing.lower_95;
      losing_upper[index] = losing.upper_95;
    }
    ImPlot::PlotShaded("Winning 95% credible",
                       xs.data(),
                       winning_lower.data(),
                       winning_upper.data(),
                       xs.size());
    ImPlot::PlotLine(
     "Winning payoff", xs.data(), winning_mean.data(), xs.size());
    ImPlot::PlotShaded("Losing 95% credible",
                       xs.data(),
                       losing_lower.data(),
                       losing_upper.data(),
                       xs.size());
    ImPlot::PlotLine("Losing payoff", xs.data(), losing_mean.data(), xs.size());
  }

  void plot_equity(this const BacktestChartWindow& self,
                   const backtest::PortfolioTimeline& timeline,
                   const std::vector<double>& backtest_timestamps,
                   double initial_capital)
  {
    auto xs = std::vector<double>{};
    auto ys = std::vector<double>{};
    auto portfolio_index = std::size_t{};
    for(auto backtest_index = std::size_t{};
        backtest_index < backtest_timestamps.size();
        ++backtest_index) {
      const auto timestamp =
       static_cast<std::time_t>(backtest_timestamps[backtest_index]);
      while(portfolio_index < timeline.size() &&
            timeline.row(portfolio_index).timestamp < timestamp) {
        ++portfolio_index;
      }
      if(portfolio_index >= timeline.size() ||
         timeline.row(portfolio_index).timestamp != timestamp) {
        continue;
      }
      const auto& row = timeline.row(portfolio_index);
      const auto equity_percentage = row.equity / initial_capital * 100.0;
      xs.push_back(static_cast<double>(backtest_index));
      ys.push_back(equity_percentage);
    }

    if(ImPlot::BeginItem("Equity")) {
      const auto plot_spec = ImPlotSpec{ImPlotProp_FillAlpha, 0.75f};
      ImPlot::PlotShaded(
       "Equity", xs.data(), ys.data(), xs.size(), 100, plot_spec);

      ImPlot::PlotLine("Equity", xs.data(), ys.data(), xs.size());

      ImPlot::EndItem();
    }
  }

  void plot_drawdown(this const BacktestChartWindow&,
                     const backtest::PortfolioTimeline& timeline,
                     const std::vector<double>& backtest_timestamps)
  {
    auto xs = std::vector<double>{};
    auto values = std::vector<double>{};
    auto portfolio_index = std::size_t{};
    for(auto backtest_index = std::size_t{};
        backtest_index < backtest_timestamps.size();
        ++backtest_index) {
      const auto timestamp =
       static_cast<std::time_t>(backtest_timestamps[backtest_index]);
      while(portfolio_index < timeline.size() &&
            timeline.row(portfolio_index).timestamp < timestamp) {
        ++portfolio_index;
      }
      if(portfolio_index >= timeline.size() ||
         timeline.row(portfolio_index).timestamp != timestamp) {
        continue;
      }
      xs.push_back(static_cast<double>(backtest_index));
      values.push_back(timeline.row(portfolio_index).drawdown);
    }
    ImPlot::PlotShaded(
     "Drawdown", xs.data(), values.data(), values.size(), 0.0);
    ImPlot::PlotLine("Drawdown", xs.data(), values.data(), values.size());
  }

  void overlays_plots(this const BacktestChartWindow& self,
                      WindowContext& context,
                        PortfolioStrategyKey selected_strategy,
                      std::size_t inspected_index,
                      std::vector<PlotValueGroup>& value_groups)
  {
    const auto& app_state = context.app_state();
    const auto portfolio_handle = app_state.selected_portfolio_handle();
    const auto run = selected_strategy.run;
    const auto& system = app_state.get_system(run.system_handle);
    const auto* strategy = app_state.get_strategy_if_present(
     system.strategy_handle(selected_strategy.strategy_index));
    if(!strategy) return;
    const auto& model = app_state.get_model(strategy->model_handle());
    const auto& results = app_state.get_portfolio_results(portfolio_handle);
    const auto& backtest_results = *results.backtest(run);
    const auto& backtest_timeline = backtest_results.timeline();
    const auto& series_results =
      backtest_results.series_results(selected_strategy.strategy_index);
    const auto& plots = model.plots();

    auto group_index = std::size_t{0};
    for(const auto& plot_group :
        plots | std::views::filter([](const auto& plot_group) {
          return plot_group.is_overlay();
        })) {
      auto values = std::vector<PlotValue>{};
      auto item_index = std::size_t{0};
      for(const auto& plot_method : plot_group.items()) {
        const auto item_id =
         std::format("overlay_{}_{}", group_index, item_index);
        const auto item_label = self.plot_item_label(plot_method, item_index);
        const auto context_for_plot =
         StrategyPlotContext{series_results,
                             backtest_timeline.size(),
                             item_id,
                             item_label,
                             inspected_index,
                             values};
        plot_method(context_for_plot);
        ++item_index;
      }
      value_groups.push_back(
       PlotValueGroup{plot_group.name(), std::move(values)});
      ++group_index;
    }
  }
};

} // namespace pludux::apps
