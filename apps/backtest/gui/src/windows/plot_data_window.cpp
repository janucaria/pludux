#include <array>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include <pludux/asset_quote.hpp>

#include "../app_state.hpp"
#include "./plot_data_window.hpp"

namespace pludux::apps {

PlotDataWindow::PlotDataWindow()
: bullish_color_{0.5, 1, 0, 1}
, bearish_color_{1, 0, 0.5, 1}
, risk_color_{1, 0, 0, 0.25}
, reward_color_{0, 1, 0, 0.25}
{
}

void PlotDataWindow::render(AppState& app_state)
{
  const auto& state = app_state.state();

  ImGui::Begin("Charts", nullptr);

  if(!state.backtest.has_value() || !state.asset_data.has_value()) {
    ImGui::End();
    return;
  }

  const auto& asset_data = state.asset_data.value();
  const auto& backtest = state.backtest.value();
  const auto& backtest_history = backtest.history();

  // get the avaliable space
  const auto available_space = ImGui::GetContentRegionAvail();

  ImGui::BeginChild(
   "Top pane", ImVec2{-1, available_space.y * 0.7f}, ImGuiChildFlags_ResizeY);

  auto axis_x_flags =
   ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoHighlight;
  auto axis_y_flags = ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_Opposite |
                      ImPlotAxisFlags_Foreground | ImPlotAxisFlags_NoLabel |
                      ImPlotAxisFlags_NoHighlight;

  if(backtest.should_run(asset_data)) {
    axis_x_flags |= ImPlotAxisFlags_AutoFit;
    axis_y_flags |= ImPlotAxisFlags_AutoFit;
  }

  if(ImPlot::BeginPlot("##OHLCPlot",
                       {-1, -1},
                       ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText |
                        ImPlotFlags_NoBoxSelect)) {
    ImPlot::SetupAxisLinks(ImAxis_X1, &plot_range_.Min, &plot_range_.Max);
    ImPlot::SetupAxis(ImAxis_X1, nullptr, axis_x_flags);
    ImPlot::SetupAxisLimits(ImAxis_X1, 0, asset_data.size() + 100);

    ImPlot::SetupAxis(ImAxis_Y1, nullptr, axis_y_flags);
    ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");

    DrawTrades("Trades", backtest_history);
    TickerTooltip(backtest_history, false);
    PlotOHLC("OHLC", backtest_history);

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
          ImPlot::SetupAxisLinks(ImAxis_X1, &plot_range_.Min, &plot_range_.Max);

          ImPlot::SetupAxis(
           ImAxis_Y1, nullptr, axis_y_flags | ImPlotAxisFlags_LockMin);
          ImPlot::SetupAxisFormat(ImAxis_Y1, VolumeFormatter);

          TickerTooltip(backtest_history, false);
          PlotVolume("Volume", backtest_history);

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

void PlotDataWindow::TickerTooltip(const backtest::History& backtest_history,
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
    if(ImPlot::IsPlotHovered() && idx > -1 && idx < backtest_history.size()) {
      ImGui::BeginTooltip();
      char buff[32];
      ImPlot::FormatDate(
       ImPlotTime::FromDouble(backtest_history.timestamp(idx)),
       buff,
       32,
       ImPlotDateFmt_DayMoYr,
       ImPlot::GetStyle().UseISO8601);
      ImGui::Text("Date:");
      ImGui::SameLine(60);
      ImGui::Text("%s", buff);
      ImGui::Text("Open:");
      ImGui::SameLine(60);
      ImGui::Text("%.2f", backtest_history.open(idx));
      ImGui::Text("Close:");
      ImGui::SameLine(60);
      ImGui::Text("%.2f", backtest_history.close(idx));
      ImGui::Text("High:");
      ImGui::SameLine(60);
      ImGui::Text("%.2f", backtest_history.high(idx));
      ImGui::Text("Low:");
      ImGui::SameLine(60);
      ImGui::Text("%.2f", backtest_history.low(idx));
      ImGui::Text("Volume:");
      ImGui::SameLine(60);
      ImGui::Text("%s",
                  std::format(std::locale("en_US.UTF-8"),
                              "{:L}",
                              (int)(backtest_history.volume(idx)))
                   .c_str());
      ImGui::EndTooltip();
    }
  }
}

int PlotDataWindow::VolumeFormatter(double value, char* buff, int size, void*)
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

void PlotDataWindow::PlotOHLC(const char* label_id,
                              const backtest::History& backtest_history)
{
  auto* draw_list = ImPlot::GetPlotDrawList();
  constexpr double half_width = 0.3;

  if(ImPlot::BeginItem(label_id)) {
    ImPlot::GetCurrentItem()->Color = ImGui::GetColorU32(bullish_color_);

    if(ImPlot::FitThisFrame()) {
      for(int i = 0; i < backtest_history.size(); ++i) {
        ImPlot::FitPoint(ImPlotPoint(i, backtest_history.low(i)));
        ImPlot::FitPoint(ImPlotPoint(i, backtest_history.high(i)));
      }
    }

    for(int i = 0; i < backtest_history.size(); ++i) {
      const auto open = backtest_history.open(i);
      const auto high = backtest_history.high(i);
      const auto low = backtest_history.low(i);
      const auto close = backtest_history.close(i);

      const auto open_left_pos = ImPlot::PlotToPixels(i - half_width, open);
      const auto open_right_pos = ImPlot::PlotToPixels(i + half_width, open);
      const auto close_left_pos = ImPlot::PlotToPixels(i - half_width, close);
      const auto close_right_pos = ImPlot::PlotToPixels(i + half_width, close);
      const auto low_pos = ImPlot::PlotToPixels(i, low);
      const auto high_pos = ImPlot::PlotToPixels(i, high);

      const auto color =
       ImGui::GetColorU32(open > close ? bearish_color_ : bullish_color_);

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

void PlotDataWindow::PlotVolume(const char* label_id,
                                const backtest::History& backtest_history)
{
  // get ImGui window DrawList
  ImDrawList* draw_list = ImPlot::GetPlotDrawList();
  // calc real value width
  const double half_width = 0.3;
  // begin plot item
  if(ImPlot::BeginItem(label_id)) {
    // override legend icon color
    ImPlot::GetCurrentItem()->Color = ImGui::GetColorU32(bullish_color_);
    // fit data if requested
    if(ImPlot::FitThisFrame()) {
      for(int i = 0; i < backtest_history.size(); ++i) {
        ImPlot::FitPoint(ImPlotPoint(i, 0));
        ImPlot::FitPoint(ImPlotPoint(i, backtest_history.volume(i)));
      }
    }
    // render data
    for(int i = 0; i < backtest_history.size(); ++i) {
      ImU32 color = ImGui::GetColorU32(
       backtest_history.open(i) > backtest_history.close(i) ? bearish_color_
                                                            : bullish_color_);
      ImVec2 low_left_pos = ImPlot::PlotToPixels(i - half_width, 0);
      ImVec2 low_right_pos = ImPlot::PlotToPixels(i + half_width, 0);
      ImVec2 high_left_pos =
       ImPlot::PlotToPixels(i - half_width, backtest_history.volume(i));
      ImVec2 high_right_pos =
       ImPlot::PlotToPixels(i + half_width, backtest_history.volume(i));

      const auto line_points =
       std::array{low_left_pos, high_left_pos, high_right_pos, low_right_pos};
      draw_list->AddRectFilled(low_left_pos, high_right_pos, color);
      draw_list->AddPolyline(
       line_points.data(), line_points.size(), color, ImDrawFlags_Closed, 1.0f);
    }

    // end plot item
    ImPlot::EndItem();
  }
}

void PlotDataWindow::DrawTrades(const char* label_id,
                                const backtest::History& backtest_history)
{
  auto* draw_list = ImPlot::GetPlotDrawList();
  constexpr double half_width = 0.5;
  if(ImPlot::BeginItem(label_id)) {
    for(int i = 0, ii = backtest_history.size(); i < ii; ++i) {
      const auto snapshot = backtest_history[i];
      const auto& trade = snapshot.trade_record();
      const auto is_last_trade = i == ii - 1;

      if(!trade.has_value()) {
        continue;
      }

      const auto entry_price = trade->entry_price();
      const auto exit_price = trade->exit_price();
      const auto entry_idx = trade->entry_index();
      const auto exit_idx = trade->exit_index();
      const auto stop_loss_price = trade->stop_loss_price();
      const auto take_profit_price = trade->take_profit_price();

      const auto left_half_width =
       trade->is_open() && i == entry_idx ? 0.0 : half_width;
      const auto right_half_width =
       trade->is_closed() && i == exit_idx ? 0.0 : half_width;

      {
        const auto risk_left_top_pos =
         ImPlot::PlotToPixels(i - left_half_width, entry_price);
        const auto risk_right_bottom_pos =
         ImPlot::PlotToPixels(i + right_half_width, stop_loss_price);

        draw_list->AddRectFilled(risk_left_top_pos,
                                 risk_right_bottom_pos,
                                 ImGui::GetColorU32(risk_color_));
      }
      {
        const auto reward_left_top_pos =
         ImPlot::PlotToPixels(i - left_half_width, take_profit_price);
        const auto reward_right_bottom_pos =
         ImPlot::PlotToPixels(i + right_half_width, entry_price);

        draw_list->AddRectFilled(reward_left_top_pos,
                                 reward_right_bottom_pos,
                                 ImGui::GetColorU32(reward_color_));
      }

      if(is_last_trade) {
        if(trade->is_open()) {
          constexpr auto left_shift_width = -half_width;
          constexpr auto right_shift_width = 10.;
          const auto risk_left_top_pos =
           ImPlot::PlotToPixels(i - left_shift_width, entry_price);
          const auto risk_right_bottom_pos =
           ImPlot::PlotToPixels(i + right_shift_width, stop_loss_price);

          const auto reward_left_top_pos =
           ImPlot::PlotToPixels(i - left_shift_width, take_profit_price);
          const auto reward_right_bottom_pos =
           ImPlot::PlotToPixels(i + right_shift_width, entry_price);

          draw_list->AddRectFilled(risk_left_top_pos,
                                   risk_right_bottom_pos,
                                   ImGui::GetColorU32(risk_color_));
          draw_list->AddRectFilled(reward_left_top_pos,
                                   reward_right_bottom_pos,
                                   ImGui::GetColorU32(reward_color_));
        }
      } else if(trade->is_closed()) {
        const auto entry_idx_stop_loss_price =
         ImPlot::PlotToPixels(entry_idx, stop_loss_price);
        const auto entry_idx_entry_price =
         ImPlot::PlotToPixels(entry_idx, entry_price);
        const auto entry_idx_take_profit_price =
         ImPlot::PlotToPixels(entry_idx, take_profit_price);
        const auto exit_idx_take_profit_price =
         ImPlot::PlotToPixels(exit_idx, take_profit_price);
        const auto exit_idx_entry_price =
         ImPlot::PlotToPixels(exit_idx, entry_price);
        const auto exit_idx_stop_loss_price =
         ImPlot::PlotToPixels(exit_idx, stop_loss_price);

        const auto risk_points = std::array{exit_idx_entry_price,
                                            exit_idx_stop_loss_price,
                                            entry_idx_stop_loss_price,
                                            entry_idx_entry_price};
        draw_list->AddPolyline(risk_points.data(),
                               risk_points.size(),
                               ImGui::GetColorU32(risk_color_),
                               ImDrawFlags_None,
                               1.0f);

        const auto reward_points = std::array{entry_idx_entry_price,
                                              entry_idx_take_profit_price,
                                              exit_idx_take_profit_price,
                                              exit_idx_entry_price};
        draw_list->AddPolyline(reward_points.data(),
                               reward_points.size(),
                               ImGui::GetColorU32(reward_color_),
                               ImDrawFlags_None,
                               1.0f);
      }
    }
    ImPlot::EndItem();
  }
}

} // namespace pludux::apps
