module;

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <format>
#include <iomanip>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

export module pludux.apps.backtest:windows.plot_data_window;

import pludux.backtest;
import :window_context;

namespace pludux::apps {

class PlotContext {
public:
  PlotContext(const SeriesEvaluationResults& series_results,
              std::size_t results_size,
              bool overlay)
  : series_results_{series_results}
  , results_size_{results_size}
  , overlay_{overlay}
  {
  }

  void render_plot_line(this const PlotContext& self,
                        const std::vector<double>& data,
                        std::uint32_t color)
  {
    const auto plot_spec =
     ImPlotSpec{ImPlotProp_LineColor,
                ImGui::ColorConvertU32ToFloat4(static_cast<ImU32>(color))};

    ImPlot::PlotLine("", data.data(), data.size(), 1.0, 0.0, plot_spec);
  }

  void render_plot_histogram(this const PlotContext& self,
                             const std::vector<double>& data,
                             std::uint32_t color)
  {
    const auto plot_spec =
     ImPlotSpec{ImPlotProp_FillColor,
                ImGui::ColorConvertU32ToFloat4(static_cast<ImU32>(color))};
    ImPlot::PlotBars("", data.data(), data.size(), 0.8, 0.0, plot_spec);
  }

  auto series_results(this const PlotContext& self, const std::string& name)
   -> std::optional<std::reference_wrapper<const std::vector<double>>>
  {
    if(const auto results = self.series_results_.results(name)) {
      return results;
    }

    return std::nullopt;
  }

  auto results_size(this const PlotContext& self) -> std::size_t
  {
    return self.results_size_;
  }

private:
  const SeriesEvaluationResults& series_results_;
  std::size_t results_size_;

  bool overlay_;
};

export class PlotDataWindow {
public:
  PlotDataWindow()
  : last_selected_backtest_opt_{}
  , last_timeline_size_opt_{}
  , bullish_color_{0.5, 1, 0, 1}
  , bearish_color_{1, 0, 0.5, 1}
  , risk_color_{1, 0, 0, 0.25}
  , reward_color_{0, 1, 0, 0.25}
  , trailing_stop_color_{1, 0., 0., 0.9}
  , row_ratios_{1, 4, 1}
  {
  }

  void render(this PlotDataWindow& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto backtest_handle = app_state.selected_backtest_handle();
    const auto backtest_ptr =
     app_state.get_backtest_if_present(backtest_handle);

    ImGui::Begin("Plots", nullptr);

    if(!backtest_ptr || !app_state.is_backtest_ready(*backtest_ptr)) {
      ImGui::End();
      return;
    }

    const auto& backtest = *backtest_ptr;
    const auto asset_handle = backtest.asset_handle();
    const auto& asset = app_state.get_asset(asset_handle);
    const auto& backtest_timelines =
     app_state.get_backtest_timelines(backtest_handle);
    const auto& backtest_series_results =
     app_state.get_series_results(backtest_handle);
    const auto timeline_size = backtest_timelines.size();

    const auto startegy_handle = backtest.strategy_handle();
    const auto& strategy = app_state.get_strategy(startegy_handle);

    const auto& plots = strategy.plots();
    const auto no_overlays_view = std::views::filter(
     [](const auto& plot_group) { return !plot_group.is_overlay(); });
    const auto additional_plots_count =
     std::ranges::distance(plots | no_overlays_view);

    auto axis_x_flags = ImPlotAxisFlags{ImPlotAxisFlags_None};
    auto axis_y_flags = ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_Opposite |
                        ImPlotAxisFlags_Foreground |
                        ImPlotAxisFlags_NoHighlight;
    const auto not_last_x_flags =
     ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoHighlight;

    constexpr auto minimum_row_ratios_size = int{3};
    const auto total_row_count =
     additional_plots_count + minimum_row_ratios_size;
    const auto must_resize_row_ratios =
     self.row_ratios_.size() != total_row_count;
    if(must_resize_row_ratios) {
      self.row_ratios_.resize(total_row_count, 1);
    }

    const auto timeline_size_changed =
     !self.last_timeline_size_opt_ ||
     self.last_timeline_size_opt_.value() != timeline_size;
    const auto reset_chart_view =
     timeline_size_changed ||
     self.is_selected_backtest_changed(backtest_handle) ||
     must_resize_row_ratios;

    if(reset_chart_view) {
      axis_x_flags |= ImPlotAxisFlags_AutoFit;
      axis_y_flags |= ImPlotAxisFlags_AutoFit;
    }

    self.last_selected_backtest_opt_ = backtest_handle;
    self.last_timeline_size_opt_ = timeline_size;

    if(ImPlot::BeginSubplots("##MainPlots",
                             static_cast<int>(self.row_ratios_.size()),
                             1,
                             ImVec2{-1, -1},
                             ImPlotSubplotFlags_LinkAllX,
                             self.row_ratios_.data())) {
      constexpr auto plot_size = ImVec2{-1, 0};
      constexpr auto plot_flags =
       ImPlotFlags_NoLegend | ImPlotFlags_Crosshairs | ImPlotFlags_NoBoxSelect;

      if(ImPlot::BeginPlot("##EquityPlot", plot_size, plot_flags)) {
        ImPlot::SetupAxis(ImAxis_X1,
                          nullptr,
                          axis_x_flags | ImPlotAxisFlags_NoTickLabels |
                           ImPlotAxisFlags_NoHighlight);
        ImPlot::SetupAxisFormat(ImAxis_X1, date_formatter, &context);

        ImPlot::SetupAxis(ImAxis_Y1, "% Equity", axis_y_flags);
        ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");

        self.plot_equity(backtest_timelines);

        ImPlot::EndPlot();
      }

      if(ImPlot::BeginPlot("##main_plots", plot_size, plot_flags)) {
        ImPlot::SetupAxis(ImAxis_X1,
                          nullptr,
                          axis_x_flags | ImPlotAxisFlags_NoTickLabels |
                           ImPlotAxisFlags_NoHighlight);
        ImPlot::SetupAxisFormat(ImAxis_X1, date_formatter, &context);

        ImPlot::SetupAxis(ImAxis_Y1, "Price", axis_y_flags);
        ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");

        self.plot_position("Trade Positions", backtest_timelines);
        self.plot_ohlc("OHLC", asset, timeline_size);
        self.overlays_plots(context);
        self.plot_signal("Trade Signals", backtest_timelines, asset);

        ImPlot::EndPlot();
      }

      if(ImPlot::BeginPlot("##VolumePlot", plot_size, plot_flags)) {
        auto is_last_plot = additional_plots_count == 0;

        if(is_last_plot) {
          ImPlot::SetupAxis(ImAxis_X1, nullptr, axis_x_flags);
        } else {
          ImPlot::SetupAxis(
           ImAxis_X1, nullptr, axis_x_flags | not_last_x_flags);
        }
        ImPlot::SetupAxisFormat(ImAxis_X1, date_formatter, &context);

        ImPlot::SetupAxis(
         ImAxis_Y1, "Volume", axis_y_flags | ImPlotAxisFlags_LockMin);
        ImPlot::SetupAxisFormat(ImAxis_Y1, volume_formatter);

        self.plot_volume("Volume", asset, timeline_size);

        ImPlot::EndPlot();
      }

      // TODO: use std::view::enumerate when it's available
      auto i = 0;
      for(const auto& plot_group : plots | no_overlays_view) {
        const auto plot_id = std::format("##Plot{}", i);

        const auto context_for_plots = PlotContext{
         backtest_series_results, timeline_size, plot_group.is_overlay()};

        if(ImPlot::BeginPlot(plot_id.c_str(), plot_size, plot_flags)) {
          const auto is_last_plot = i == additional_plots_count - 1;

          if(is_last_plot) {
            ImPlot::SetupAxis(ImAxis_X1, nullptr, axis_x_flags);
          } else {
            ImPlot::SetupAxis(
             ImAxis_X1, nullptr, axis_x_flags | not_last_x_flags);
          }
          ImPlot::SetupAxisFormat(ImAxis_X1, date_formatter, &context);

          ImPlot::SetupAxis(ImAxis_Y1, plot_group.name().c_str(), axis_y_flags);

          {
            const auto& plot_items = plot_group.items();
            for(const auto& plot_method : plot_items) {
              plot_method(context_for_plots);
            }
          }

          ImPlot::EndPlot();
        }

        ++i;
      }

      ImPlot::EndSubplots();
    }
    ImGui::End();
  }

private:
  std::optional<backtest::BacktestStoreHandle> last_selected_backtest_opt_;
  std::optional<std::size_t> last_timeline_size_opt_;

  ImVec4 bullish_color_;
  ImVec4 bearish_color_;

  ImVec4 risk_color_;
  ImVec4 reward_color_;

  ImVec4 trailing_stop_color_;

  std::vector<float> row_ratios_;

  static auto volume_formatter(double value, char* buff, int size, void*) -> int
  {
    return std::snprintf(
     buff, size, "%s", format_compact_integer(value).c_str());
  }

  static auto
  date_formatter(double value, char* buff, int size, void* user_data) -> int
  {
    auto& context = *reinterpret_cast<WindowContext*>(user_data);
    const auto& app_state = context.app_state();
    const auto backtest_handle = app_state.selected_backtest_handle();
    const auto& backtest = app_state.get_backtest(backtest_handle);
    const auto& timeline = app_state.get_backtest_timelines(backtest_handle);

    const auto idx = static_cast<std::ptrdiff_t>(value);
    if(idx < 0 || idx >= timeline.size()) {
      return std::snprintf(buff, size, "");
    }

    const auto& asset_handle = backtest.asset_handle();
    const auto& asset = app_state.get_asset(asset_handle);
    const auto& snapshot = asset.get_snapshot(idx);

    const auto datetime = snapshot.datetime();
    const auto timestamp = static_cast<std::time_t>(datetime);

    const auto formated_datetime = format_datetime(timestamp);
    return std::snprintf(buff, size, "%s", formated_datetime.c_str());
  }

  auto is_selected_backtest_changed(
   this const PlotDataWindow& self,
   const backtest::BacktestStoreHandle& selected_backtest_handle) -> bool
  {
    return !self.last_selected_backtest_opt_ ||
           self.last_selected_backtest_opt_.value() != selected_backtest_handle;
  }

  void plot_ohlc(this const PlotDataWindow& self,
                 const char* label_id,
                 const backtest::Asset& asset,
                 std::size_t total_count)
  {
    if(ImPlot::BeginItem(label_id) && total_count > 0) {
      ImPlot::GetCurrentItem()->Color = ImGui::GetColorU32(self.bullish_color_);

      auto* draw_list = ImPlot::GetPlotDrawList();
      constexpr double half_width = 0.3;
      for(int i = 0, ii = static_cast<int>(total_count); i < ii; ++i) {
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

      {
        auto timeline_i = static_cast<int>(total_count) - 1;
        const auto is_hovered =
         ImPlot::IsSubplotsHovered() || ImPlot::IsPlotHovered();
        if(is_hovered) {
          ImPlotPoint mouse = ImPlot::GetPlotMousePos();
          mouse.x = std::round(mouse.x);
          const auto hovered_idx = static_cast<int>(mouse.x);
          const auto timeline_size_i = static_cast<int>(total_count);
          if(hovered_idx > -1 && hovered_idx < timeline_size_i) {
            timeline_i = hovered_idx;
          }
        }

        const auto snapshot = asset.get_snapshot(timeline_i);

        const auto open = snapshot.open();
        const auto high = snapshot.high();
        const auto low = snapshot.low();
        const auto close = snapshot.close();

        const auto change = close - open;
        const auto change_percent = open != 0.0 ? (change / open) * 100.0 : 0.0;
        const auto is_bullish = close >= open;

        const auto o_label_text = std::string{"O"};
        const auto o_value_text = std::format("{:.2f}", open);
        const auto h_label_text = std::string{"  H"};
        const auto h_value_text = std::format("{:.2f}", high);
        const auto l_label_text = std::string{"  L"};
        const auto l_value_text = std::format("{:.2f}", low);
        const auto c_label_text = std::string{"  C"};
        const auto close_text = std::format("{:.2f}", close);
        const auto change_text =
         std::format("  {:+.2f} ({:+.2f}%)", change, change_percent);

        const auto marker_text_color =
         ImGui::GetColorU32(ImVec4{1.0f, 1.0f, 1.0f, 1.0f});
        const auto close_text_color = ImGui::GetColorU32(
         is_bullish ? self.bullish_color_ : self.bearish_color_);
        const auto plot_pos = ImPlot::GetPlotPos();
        auto text_cursor = ImVec2{plot_pos.x + 8.0f, plot_pos.y + 8.0f};

        const auto draw_text_segment = [&](const std::string& text,
                                           ImU32 color) {
          draw_list->AddText(text_cursor, color, text.c_str());
          text_cursor.x += ImGui::CalcTextSize(text.c_str()).x;
        };

        ImPlot::PushPlotClipRect();
        draw_text_segment(o_label_text, marker_text_color);
        draw_text_segment(o_value_text, close_text_color);
        draw_text_segment(h_label_text, marker_text_color);
        draw_text_segment(h_value_text, close_text_color);
        draw_text_segment(l_label_text, marker_text_color);
        draw_text_segment(l_value_text, close_text_color);
        draw_text_segment(c_label_text, marker_text_color);
        draw_text_segment(close_text, close_text_color);
        draw_text_segment(change_text, close_text_color);
        ImPlot::PopPlotClipRect();
      }

      ImPlot::EndItem();
    }
  }

  void plot_volume(this const PlotDataWindow& self,
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

      if(total_count > 0) {
        auto timeline_i = static_cast<int>(total_count) - 1;
        const auto is_hovered =
         ImPlot::IsSubplotsHovered() || ImPlot::IsPlotHovered();
        if(is_hovered) {
          ImPlotPoint mouse = ImPlot::GetPlotMousePos();
          mouse.x = std::round(mouse.x);
          const auto hovered_idx = static_cast<int>(mouse.x);
          const auto timeline_size_i = static_cast<int>(total_count);
          if(hovered_idx > -1 && hovered_idx < timeline_size_i) {
            timeline_i = hovered_idx;
          }
        }

        const auto snapshot = asset.get_snapshot(timeline_i);
        const auto open = snapshot.open();
        const auto close = snapshot.close();
        const auto volume = snapshot.volume();

        const auto is_bullish = close >= open;

        const auto v_label_text = std::string{"Vol "};
        const auto v_value_text = format_compact_integer(volume);

        const auto marker_text_color =
         ImGui::GetColorU32(ImVec4{1.0f, 1.0f, 1.0f, 1.0f});
        const auto close_text_color = ImGui::GetColorU32(
         is_bullish ? self.bullish_color_ : self.bearish_color_);
        const auto plot_pos = ImPlot::GetPlotPos();
        auto text_cursor = ImVec2{plot_pos.x + 8.0f, plot_pos.y + 8.0f};

        const auto draw_text_segment = [&](const std::string& text,
                                           ImU32 color) {
          draw_list->AddText(text_cursor, color, text.c_str());
          text_cursor.x += ImGui::CalcTextSize(text.c_str()).x;
        };

        ImPlot::PushPlotClipRect();
        draw_text_segment(v_label_text, marker_text_color);
        draw_text_segment(v_value_text, close_text_color);
        ImPlot::PopPlotClipRect();
      }

      constexpr auto half_width = 0.3;
      for(int i = 0, ii = static_cast<int>(total_count); i < ii; ++i) {
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

  void plot_position(this const PlotDataWindow& self,
                     const char* label_id,
                     const backtest::BacktestTimeline& backtest_timelines)
  {
    auto* draw_list = ImPlot::GetPlotDrawList();

    constexpr float half_width = 0.5f;
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

      for(auto i = std::size_t{0}; i < timeline_size; ++i) {
        const auto& open_position = backtest_timelines.open_position(i);
        if(!open_position) {
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

  void plot_signal(this const PlotDataWindow& self,
                   const char* label_id,
                   const backtest::BacktestTimeline& backtest_timelines,
                   const backtest::Asset& asset)
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

      for(auto i = std::size_t{0}; i < timeline_size; ++i) {
        const auto snapshot = asset.get_snapshot(i);
        for(const auto& event : backtest_timelines.trade_events(i)) {
          if(event.is_rejected()) {
            const auto entry_low = snapshot.low();
            auto rejected_pos = ImPlot::PlotToPixels(i, entry_low);
            rejected_pos.y += marker_offset;
            const auto rejected_color = ImGui::GetColorU32(self.bearish_color_);
            constexpr auto rejected_marker_size = 6.0f;
            rejected_pos.y = clamp_marker_y(
             rejected_pos.y, rejected_marker_size, rejected_marker_size);

            draw_list->AddLine(ImVec2{rejected_pos.x - rejected_marker_size,
                                      rejected_pos.y - rejected_marker_size},
                               ImVec2{rejected_pos.x + rejected_marker_size,
                                      rejected_pos.y + rejected_marker_size},
                               rejected_color,
                               2.0f);
            draw_list->AddLine(ImVec2{rejected_pos.x - rejected_marker_size,
                                      rejected_pos.y + rejected_marker_size},
                               ImVec2{rejected_pos.x + rejected_marker_size,
                                      rejected_pos.y - rejected_marker_size},
                               rejected_color,
                               2.0f);
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

  void plot_equity(this const PlotDataWindow& self,
                   const backtest::BacktestTimeline& backtest_timelines)
  {
    const auto timeline_size = backtest_timelines.size();
    const auto timeline_size_i = static_cast<int>(timeline_size);
    auto xs = std::vector<double>{};
    auto ys = std::vector<double>{};
    for(auto timeline_i = 0; timeline_i < timeline_size; ++timeline_i) {
      const auto equity = backtest_timelines.equity(timeline_i);
      const auto equity_percentage =
       equity / backtest_timelines.initial_capital(timeline_i) * 100.0;
      const auto plot_idx = timeline_i;
      xs.push_back(plot_idx);
      ys.push_back(equity_percentage);
    }

    auto equity_i = timeline_size_i > 0 ? timeline_size_i - 1 : -1;

    if(ImPlot::IsSubplotsHovered() || ImPlot::IsPlotHovered()) {
      constexpr auto half_width = 0.5;
      auto* draw_list = ImPlot::GetPlotDrawList();
      ImPlotPoint mouse = ImPlot::GetPlotMousePos();
      mouse.x = std::round(mouse.x);
      float tool_l = ImPlot::PlotToPixels(mouse.x - half_width, mouse.y).x;
      float tool_r = ImPlot::PlotToPixels(mouse.x + half_width, mouse.y).x;
      float tool_t = ImPlot::GetPlotPos().y;
      float tool_b = tool_t + ImPlot::GetPlotSize().y;
      ImPlot::PushPlotClipRect();
      draw_list->AddRectFilled(ImVec2(tool_l, tool_t),
                               ImVec2(tool_r, tool_b),
                               IM_COL32(128, 128, 128, 64));
      ImPlot::PopPlotClipRect();

      const auto plot_idx = static_cast<int>(mouse.x);
      if(plot_idx > -1 && plot_idx < timeline_size_i) {
        equity_i = plot_idx;
      }

      if(ImPlot::IsPlotHovered() && plot_idx > -1 &&
         plot_idx < timeline_size_i) {
        const auto timeline_i = plot_idx;
        ImGui::BeginTooltip();
        ImGui::Text("%s%%", format_currency(ys[timeline_i]).c_str());
        ImGui::EndTooltip();
      }
    }

    if(equity_i > -1) {
      auto* draw_list = ImPlot::GetPlotDrawList();
      const auto marker_text_color =
       ImGui::GetColorU32(ImVec4{1.0f, 1.0f, 1.0f, 1.0f});
      const auto equity_color = ImGui::GetColorU32(
       ys[equity_i] >= 100.0 ? self.bullish_color_ : self.bearish_color_);
      const auto plot_pos = ImPlot::GetPlotPos();
      auto text_cursor = ImVec2{plot_pos.x + 8.0f, plot_pos.y + 8.0f};
      const auto eq_label_text = std::string{"Eq "};
      const auto eq_value_text =
       std::string{format_currency(ys[equity_i])} + "%";

      const auto draw_text_segment = [&](const std::string& text, ImU32 color) {
        draw_list->AddText(text_cursor, color, text.c_str());
        text_cursor.x += ImGui::CalcTextSize(text.c_str()).x;
      };

      ImPlot::PushPlotClipRect();
      draw_text_segment(eq_label_text, marker_text_color);
      draw_text_segment(eq_value_text, equity_color);
      ImPlot::PopPlotClipRect();
    }

    if(ImPlot::BeginItem("Equity")) {
      const auto plot_spec = ImPlotSpec{ImPlotProp_FillAlpha, 0.75f};
      ImPlot::PlotShaded(
       "Equity", xs.data(), ys.data(), xs.size(), 100, plot_spec);

      ImPlot::PlotLine("Equity", xs.data(), ys.data(), xs.size());

      ImPlot::EndItem();
    }
  }

  void overlays_plots(this const PlotDataWindow& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto backtest_handle = app_state.selected_backtest_handle();
    const auto& backtest = app_state.selected_backtest();
    const auto& strategy_handle = backtest.strategy_handle();
    const auto& strategy = app_state.get_strategy(strategy_handle);
    const auto& backtest_timelines =
     app_state.get_backtest_timelines(backtest_handle);
    const auto& series_results = app_state.get_series_results(backtest_handle);
    const auto& plots = strategy.plots();

    const auto context_for_plots =
     PlotContext{series_results, backtest_timelines.size(), true};

    for(const auto& plot_group :
        plots | std::views::filter([](const auto& plot_group) {
          return plot_group.is_overlay();
        })) {
      const auto& plot_items = plot_group.items();
      for(const auto& plot_method : plot_items) {
        plot_method(context_for_plots);
      }
    }
  }
};

} // namespace pludux::apps
