module;

#include <algorithm>
#include <array>
#include <format>
#include <tuple>
#include <vector>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

export module pludux.apps.backtest:windows.plot_data_window;

import :app_state;

export namespace pludux::apps {

class PlotDataWindow {
public:
  PlotDataWindow()
  : last_selected_backtest_index_{-1}
  , bullish_color_{0.5, 1, 0, 1}
  , bearish_color_{1, 0, 0.5, 1}
  , risk_color_{1, 0, 0, 0.25}
  , reward_color_{0, 1, 0, 0.25}
  , trailing_stop_color_{1, 0., 0., 0.9}
  , plot_range_{0, 1}
  {
  }

  void render(this PlotDataWindow& self, AppState& app_state)
  {
    const auto& state = app_state.state();
    const auto& backtests = state.backtests;

    ImGui::Begin("Charts", nullptr);

    if(backtests.empty()) {
      ImGui::End();
      return;
    }

    const auto& asset = backtests[state.selected_backtest_index].asset();
    const auto& asset_history = asset.history();

    auto backtest_summaries = std::vector<backtest::BacktestSummary>{};
    auto is_backtest_should_run = false;
    if(!state.backtests.empty()) {
      const auto& backtest = state.backtests[state.selected_backtest_index];
      is_backtest_should_run = backtest.should_run();
      backtest_summaries = backtest.summaries();
    }

    // get the avaliable space
    const auto available_space = ImGui::GetContentRegionAvail();

    ImGui::BeginChild(
     "Top pane", ImVec2{-1, available_space.y * 0.7f}, ImGuiChildFlags_ResizeY);

    auto axis_x_flags = ImPlotAxisFlags_None | ImPlotAxisFlags_Invert;
    auto axis_y_flags = ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_Opposite |
                        ImPlotAxisFlags_Foreground | ImPlotAxisFlags_NoLabel |
                        ImPlotAxisFlags_NoHighlight;

    const auto reset_chart_view =
     self.last_selected_backtest_index_ != state.selected_backtest_index;
    if(is_backtest_should_run || reset_chart_view) {
      axis_x_flags |= ImPlotAxisFlags_AutoFit;
      axis_y_flags |= ImPlotAxisFlags_AutoFit;

      self.last_selected_backtest_index_ = state.selected_backtest_index;
    }

    if(ImPlot::BeginPlot("##OHLCPlot",
                         {-1, -1},
                         ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText |
                          ImPlotFlags_NoBoxSelect)) {
      ImPlot::SetupAxisLinks(
       ImAxis_X1, &self.plot_range_.Min, &self.plot_range_.Max);
      ImPlot::SetupAxis(ImAxis_X1,
                        nullptr,
                        axis_x_flags | ImPlotAxisFlags_NoTickLabels |
                         ImPlotAxisFlags_NoHighlight);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, asset_history.size() + 100);

      ImPlot::SetupAxis(ImAxis_Y1, nullptr, axis_y_flags);
      ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");

      self.DrawTrades("Trades", backtest_summaries, asset_history);
      self.TickerTooltip(asset_history, false);
      self.PlotOHLC("OHLC", asset_history);

      ImPlot::EndPlot();
    }
    ImGui::EndChild();

    ImGui::BeginChild("Bottom pane", ImVec2{-1, -1});
    {
      const auto tab_bar_flags = ImGuiTabBarFlags_None;

      if(ImGui::BeginTabBar("Indicators", tab_bar_flags)) {
        if(ImGui::BeginTabItem("Volume")) {
          if(ImPlot::BeginPlot("##VolumePlot",
                               {-1, -1},
                               ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText |
                                ImPlotFlags_NoBoxSelect)) {
            ImPlot::SetupAxisLinks(
             ImAxis_X1, &self.plot_range_.Min, &self.plot_range_.Max);

            ImPlot::SetupAxis(ImAxis_X1, nullptr, axis_x_flags);
            ImPlot::SetupAxis(
             ImAxis_Y1, nullptr, axis_y_flags | ImPlotAxisFlags_LockMin);
            ImPlot::SetupAxisFormat(ImAxis_Y1, VolumeFormatter);

            self.TickerTooltip(asset_history, false);
            self.PlotVolume("Volume", asset_history);

            ImPlot::EndPlot();
          }

          ImGui::EndTabItem();
        }

        if(ImGui::BeginTabItem("Equity (%)")) {
          if(ImPlot::BeginPlot("##EquityPlot",
                               {-1, -1},
                               ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText |
                                ImPlotFlags_NoBoxSelect)) {
            ImPlot::SetupAxisLinks(
             ImAxis_X1, &self.plot_range_.Min, &self.plot_range_.Max);

            ImPlot::SetupAxis(ImAxis_X1, nullptr, axis_x_flags);

            ImPlot::SetupAxis(ImAxis_Y1, nullptr, axis_y_flags);
            ImPlot::SetupAxisFormat(ImAxis_Y1, VolumeFormatter);

            self.plot_equity(backtest_summaries);

            ImPlot::EndPlot();
          }

          ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
      }
    }
    ImGui::EndChild();
    ImGui::End();
  }

private:
  std::ptrdiff_t last_selected_backtest_index_;

  ImVec4 bullish_color_;
  ImVec4 bearish_color_;

  ImVec4 risk_color_;
  ImVec4 reward_color_;

  ImVec4 trailing_stop_color_;

  ImPlotRange plot_range_;

  static void TickerTooltip(const AssetHistory& asset_history,
                            bool span_subplots)
  {
    ImDrawList* draw_list = ImPlot::GetPlotDrawList();
    const double half_width = 0.5;
    const bool hovered =
     span_subplots ? ImPlot::IsSubplotsHovered() : ImPlot::IsPlotHovered();
    if(hovered) {
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

      const auto idx = static_cast<int>(mouse.x);
      if(ImPlot::IsPlotHovered() && idx > -1 && idx < asset_history.size()) {
        const auto snapshot = AssetSnapshot(idx, asset_history);

        if(ImGui::BeginTooltip()) {
          ImGui::Text("Date:");
          ImGui::SameLine(60);
          ImGui::Text("%s", format_datetime(snapshot.datetime()).c_str());
          ImGui::Text("Open:");
          ImGui::SameLine(60);
          ImGui::Text("%.2f", snapshot.open());
          ImGui::Text("Close:");
          ImGui::SameLine(60);
          ImGui::Text("%.2f", snapshot.close());
          ImGui::Text("High:");
          ImGui::SameLine(60);
          ImGui::Text("%.2f", snapshot.high());
          ImGui::Text("Low:");
          ImGui::SameLine(60);
          ImGui::Text("%.2f", snapshot.low());
          ImGui::Text("Volume:");
          ImGui::SameLine(60);
          ImGui::Text("%s", format_currency(snapshot.volume()).c_str());
          ImGui::EndTooltip();
        }
      }
    }
  }

  static auto VolumeFormatter(double value, char* buff, int size, void*) -> int
  {
    double v[] = {1000000000000, 1000000000, 1000000, 1000, 1};
    const char* p[] = {"T", "B", "M", "k", ""};
    if(value == 0) {
      return snprintf(buff, size, "0");
    }
    for(int i = 0; i < 5; ++i) {
      if(fabs(value) >= v[i]) {
        return snprintf(buff, size, "%g%s", value / v[i], p[i]);
      }
    }
    return snprintf(buff, size, "%g%s", value / v[4], p[4]);
  }

  void PlotOHLC(this const PlotDataWindow& self,
                const char* label_id,
                const AssetHistory& asset_history)
  {
    auto* draw_list = ImPlot::GetPlotDrawList();
    constexpr double half_width = 0.3;

    if(ImPlot::BeginItem(label_id)) {
      ImPlot::GetCurrentItem()->Color = ImGui::GetColorU32(self.bullish_color_);

      if(ImPlot::FitThisFrame()) {
        for(int i = 0; i < asset_history.size(); ++i) {
          const auto snapshot = AssetSnapshot(i, asset_history);

          ImPlot::FitPoint(ImPlotPoint(i, snapshot.low()));
          ImPlot::FitPoint(ImPlotPoint(i, snapshot.high()));
        }
      }

      for(int i = 0, ii = asset_history.size(); i < ii; ++i) {
        const auto snapshot = AssetSnapshot(i, asset_history);

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
      ImPlot::EndItem();
    }
  }

  void PlotVolume(this const PlotDataWindow& self,
                  const char* label_id,
                  const AssetHistory& asset_history)
  {
    // get ImGui window DrawList
    ImDrawList* draw_list = ImPlot::GetPlotDrawList();
    // calc real value width
    const double half_width = 0.3;
    // begin plot item
    if(ImPlot::BeginItem(label_id)) {
      // override legend icon color
      ImPlot::GetCurrentItem()->Color = ImGui::GetColorU32(self.bullish_color_);
      // fit data if requested
      if(ImPlot::FitThisFrame()) {
        for(int i = 0; i < asset_history.size(); ++i) {
          const auto snapshot = AssetSnapshot(i, asset_history);

          ImPlot::FitPoint(ImPlotPoint(i, 0));
          ImPlot::FitPoint(ImPlotPoint(i, snapshot.volume()));
        }
      }
      // render data
      for(int i = 0, ii = asset_history.size(); i < ii; ++i) {
        const auto snapshot = AssetSnapshot(i, asset_history);

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

      // end plot item
      ImPlot::EndItem();
    }
  }

  void
  DrawTrades(this const PlotDataWindow& self,
             const char* label_id,
             const std::vector<backtest::BacktestSummary>& backtest_summaries,
             const AssetHistory& asset_history)
  {
    constexpr auto marker_offset = 50.0f;
    const auto marker_text_color =
     ImGui::GetColorU32(ImVec4{1.0f, 1.0f, 1.0f, 1.0f});
    auto* draw_list = ImPlot::GetPlotDrawList();

    constexpr float half_width = 0.5f;
    if(ImPlot::BeginItem(label_id)) {
      auto trailing_stop_lines = std::vector<ImVec2>{};
      trailing_stop_lines.reserve(backtest_summaries.size());

      for(auto i = std::size_t{0}, ii = backtest_summaries.size(); i < ii;
          ++i) {
        const auto& summary = backtest_summaries[i];
        const auto& session = summary.trade_session();

        for(const auto& record : session.trade_record_range()) {
          const auto is_long_position = record.is_long_position();
          const auto exit_idx = record.exit_index();
          const auto exit_price = record.exit_price();
          const auto take_profit_price = record.take_profit_price();
          const auto stop_loss_price = record.stop_loss_price();
          const auto trailing_stop_price = record.trailing_stop_price();
          const auto entry_idx = record.entry_index();
          const auto avg_price = record.average_price();

          const auto top_color =
           is_long_position ? self.reward_color_ : self.risk_color_;
          const auto bottom_color =
           is_long_position ? self.risk_color_ : self.reward_color_;

          const auto [top_price, middle_price, bottom_price] = [&]() {
            if(is_long_position) {
              const auto top_price = !std::isnan(take_profit_price)
                                      ? take_profit_price
                                      : std::max(avg_price, exit_price);
              const auto middle_price = std::max(avg_price, stop_loss_price);
              const auto bottom_price = !std::isnan(stop_loss_price)
                                         ? stop_loss_price
                                         : std::min(middle_price, exit_price);

              return std::tuple{top_price, middle_price, bottom_price};
            }

            const auto top_price = !std::isnan(stop_loss_price)
                                    ? stop_loss_price
                                    : std::max(avg_price, exit_price);
            const auto middle_price = std::max(avg_price, take_profit_price);
            const auto bottom_price = !std::isnan(take_profit_price)
                                       ? take_profit_price
                                       : std::min(middle_price, exit_price);

            return std::tuple{top_price, middle_price, bottom_price};
          }();

          const auto left_half_width = exit_idx == entry_idx ? 0.0 : half_width;
          const auto right_half_width =
           !record.is_closed() ? (exit_idx == 0 ? 10.0 : half_width) : 0.0;

          {
            const auto risk_left_top_pos =
             ImPlot::PlotToPixels(exit_idx + left_half_width, middle_price);
            const auto risk_right_bottom_pos =
             ImPlot::PlotToPixels(exit_idx - right_half_width, bottom_price);

            draw_list->AddRectFilled(risk_left_top_pos,
                                     risk_right_bottom_pos,
                                     ImGui::GetColorU32(bottom_color));
          }
          {
            const auto reward_left_top_pos =
             ImPlot::PlotToPixels(exit_idx + left_half_width, top_price);
            const auto reward_right_bottom_pos =
             ImPlot::PlotToPixels(exit_idx - right_half_width, middle_price);

            draw_list->AddRectFilled(reward_left_top_pos,
                                     reward_right_bottom_pos,
                                     ImGui::GetColorU32(top_color));
          }

          if(record.entry_index() == record.exit_index()) {
            const auto record_entry_idx = record.entry_index();

            const auto entry_low =
             AssetSnapshot{record_entry_idx, asset_history}.low();

            auto entry_pos = ImPlot::PlotToPixels(record_entry_idx, entry_low);
            entry_pos.y += marker_offset;

            draw_list->AddTriangleFilled(
             ImVec2{entry_pos.x - 5, entry_pos.y},
             ImVec2{entry_pos.x + 5, entry_pos.y},
             ImVec2{entry_pos.x, entry_pos.y - 10},
             ImGui::GetColorU32(self.bullish_color_));

            const auto trade_count_str =
             std::format("#{}", summary.trade_count() + 1);
            const auto text_size = ImGui::CalcTextSize(trade_count_str.c_str());
            draw_list->AddText(
             ImVec2{entry_pos.x - text_size.x * 0.5f, entry_pos.y},
             marker_text_color,
             trade_count_str.c_str());
          }

          {
            if(record.is_closed()) {
              const auto exit_high =
               AssetSnapshot{exit_idx, asset_history}.high();

              auto exit_pos = ImPlot::PlotToPixels(exit_idx, exit_high);
              exit_pos.y -= marker_offset;

              draw_list->AddTriangleFilled(
               ImVec2{exit_pos.x - 5, exit_pos.y},
               ImVec2{exit_pos.x + 5, exit_pos.y},
               ImVec2{exit_pos.x, exit_pos.y + 10},
               ImGui::GetColorU32(self.bearish_color_));

              const auto trade_count_str =
               std::format("#{}", summary.trade_count());
              const auto text_size =
               ImGui::CalcTextSize(trade_count_str.c_str());
              draw_list->AddText(
               ImVec2{exit_pos.x - text_size.x * 0.5f, exit_pos.y - 13},
               marker_text_color,
               trade_count_str.c_str());
            }
          }

          {
            const auto left_pos =
             ImPlot::PlotToPixels(exit_idx + half_width, trailing_stop_price);
            const auto center_pos =
             ImPlot::PlotToPixels(exit_idx, trailing_stop_price);
            const auto right_pos =
             ImPlot::PlotToPixels(exit_idx - half_width, trailing_stop_price);

            trailing_stop_lines.push_back(left_pos);
            trailing_stop_lines.push_back(center_pos);
            trailing_stop_lines.push_back(right_pos);

            if(record.is_closed()) {
              const auto nan_pos = ImVec2{NAN, NAN};
              trailing_stop_lines.push_back(nan_pos);
            }
          }
        }
      }

      {
        // remove duplicates of consecutive points
        auto it = std::unique(trailing_stop_lines.begin(),
                              trailing_stop_lines.end(),
                              [](const ImVec2& a, const ImVec2& b) {
                                return a.x == b.x && a.y == b.y;
                              });

        trailing_stop_lines.erase(it, trailing_stop_lines.end());
        if(!trailing_stop_lines.empty()) {
          draw_list->AddPolyline(trailing_stop_lines.data(),
                                 trailing_stop_lines.size(),
                                 ImGui::GetColorU32(self.trailing_stop_color_),
                                 ImDrawFlags_None,
                                 1.2f);
        }
      }

      ImPlot::EndItem();
    }
  }

  void
  plot_equity(this const PlotDataWindow& self,
              const std::vector<backtest::BacktestSummary>& backtest_summaries)
  {
    const auto summaries_size = backtest_summaries.size();
    auto xs = std::vector<double>{};
    auto ys = std::vector<double>{};
    for(auto summary_i = 0; summary_i < summaries_size; ++summary_i) {
      const auto& summary = backtest_summaries[summary_i];
      const auto equity = summary.equity();
      const auto equity_percentage = equity / summary.initial_capital() * 100.0;
      const auto plot_idx = summaries_size - summary_i - 1;
      xs.push_back(plot_idx);
      ys.push_back(equity_percentage);
    }

    if(ImPlot::IsPlotHovered()) {
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
      if(ImPlot::IsPlotHovered() && plot_idx > -1 &&
         plot_idx < summaries_size) {
        const auto summary_i = summaries_size - plot_idx - 1;
        ImGui::BeginTooltip();
        ImGui::Text("%s%%", format_currency(ys[summary_i]).c_str());
        ImGui::EndTooltip();
      }
    }

    if(ImPlot::BeginItem("Equity")) {
      ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.5f);
      ImPlot::PlotShaded("Equity", xs.data(), ys.data(), xs.size(), 100);
      ImPlot::PopStyleVar();

      ImPlot::PlotLine("Equity", xs.data(), ys.data(), xs.size());

      ImPlot::EndItem();
    }
  }
};

} // namespace pludux::apps
