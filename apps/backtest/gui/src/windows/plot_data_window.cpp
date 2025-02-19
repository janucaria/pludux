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
  static ImPlotRange plot_range{0, 1};

  const auto& state = app_state.state();

  ImGui::Begin("Charts", nullptr);

  if(!state.asset_data.has_value()) {
    ImGui::End();
    return;
  }

  const auto& asset_data = state.asset_data.value();
  const auto& backtest = state.backtest;
  const auto& closed_trades = backtest
                               ? backtest->backtesting_summary().closed_trades()
                               : std::vector<backtest::TradeRecord>{};
  const auto& open_trade =
   backtest ? backtest->backtesting_summary().open_trade() : std::nullopt;

  // get the avaliable space
  const auto available_space = ImGui::GetContentRegionAvail();

  ImGui::BeginChild(
   "Top pane", ImVec2{-1, available_space.y * 0.7f}, ImGuiChildFlags_ResizeY);
  if(ImPlot::BeginPlot("##OHLCPlot",
                       {-1, -1},
                       ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText |
                        ImPlotFlags_NoBoxSelect)) {
    ImPlot::SetupAxisLinks(ImAxis_X1, &plot_range.Min, &plot_range.Max);

    const auto axis_x_flags = ImPlotAxisFlags_Invert |
                              ImPlotAxisFlags_NoTickLabels |
                              ImPlotAxisFlags_NoHighlight;

    ImPlot::SetupAxis(ImAxis_X1, nullptr, axis_x_flags);
    ImPlot::SetupAxisLimits(ImAxis_X1, -100, asset_data.size());

    auto axis_y_flags = ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_Opposite |
                        ImPlotAxisFlags_Foreground | ImPlotAxisFlags_NoLabel |
                        ImPlotAxisFlags_NoHighlight;
    if(state.resource_changed) {
      axis_y_flags |= ImPlotAxisFlags_AutoFit;

      app_state.push_action(
       [](AppStateData& state) { state.resource_changed = false; });
    }

    ImPlot::SetupAxis(ImAxis_Y1, nullptr, axis_y_flags);
    ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");

    ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);

    DrawClosedTrades("ClosedTrades", closed_trades);

    if(open_trade.has_value()) {
      DrawOpenTrade("OpenTrade", open_trade.value());
    }

    if(backtest) {
      const auto asset_quote = AssetQuote{asset_data, backtest->quote_access()};

      PlotOHLC("OHLC", asset_quote);
      TickerTooltip(asset_quote, false);
    }

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
          ImPlot::SetupAxisLinks(ImAxis_X1, &plot_range.Min, &plot_range.Max);
          ImPlot::SetupAxes(nullptr,
                            nullptr,
                            ImPlotAxisFlags_Invert,
                            ImPlotAxisFlags_LockMin | ImPlotAxisFlags_AutoFit |
                             ImPlotAxisFlags_RangeFit |
                             ImPlotAxisFlags_Opposite);
          ImPlot::SetupAxisFormat(ImAxis_Y1, VolumeFormatter);

          if(backtest) {
            const auto asset_quote =
             AssetQuote{asset_data, backtest->quote_access()};
            TickerTooltip(asset_quote, false);
            PlotVolume("Volume", asset_quote);
          }
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

void PlotDataWindow::TickerTooltip(AssetQuote asset_quote, bool span_subplots)
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
    if(ImPlot::IsPlotHovered() && idx > -1 &&
       idx < asset_quote.get_asset_size()) {
      ImGui::BeginTooltip();
      char buff[32];
      ImPlot::FormatDate(ImPlotTime::FromDouble(asset_quote.time()[idx]),
                         buff,
                         32,
                         ImPlotDateFmt_DayMoYr,
                         ImPlot::GetStyle().UseISO8601);
      ImGui::Text("Date:");
      ImGui::SameLine(60);
      ImGui::Text("%s", buff);
      ImGui::Text("Open:");
      ImGui::SameLine(60);
      ImGui::Text("%.2f", asset_quote.open()[idx]);
      ImGui::Text("Close:");
      ImGui::SameLine(60);
      ImGui::Text("%.2f", asset_quote.close()[idx]);
      ImGui::Text("High:");
      ImGui::SameLine(60);
      ImGui::Text("%.2f", asset_quote.high()[idx]);
      ImGui::Text("Low:");
      ImGui::SameLine(60);
      ImGui::Text("%.2f", asset_quote.low()[idx]);
      ImGui::Text("Volume:");
      ImGui::SameLine(60);
      ImGui::Text("%s",
                  std::format(std::locale("en_US.UTF-8"),
                              "{:L}",
                              (int)(asset_quote.volume()[idx]))
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

void PlotDataWindow::PlotOHLC(const char* label_id, AssetQuote asset_quote)
{
  // get ImGui window DrawList
  ImDrawList* draw_list = ImPlot::GetPlotDrawList();
  // calc real value width
  const double half_width = 0.25;
  // begin plot item
  if(ImPlot::BeginItem(label_id)) {
    // override legend icon color
    ImPlot::GetCurrentItem()->Color = ImGui::GetColorU32(bullish_color_);
    // fit data if requested
    if(ImPlot::FitThisFrame()) {
      for(int i = 0; i < asset_quote.get_asset_size(); ++i) {
        ImPlot::FitPoint(ImPlotPoint(i, asset_quote.low()[i]));
        ImPlot::FitPoint(ImPlotPoint(i, asset_quote.high()[i]));
      }
    }
    // render data
    for(int i = 0; i < asset_quote.get_asset_size(); ++i) {
      ImU32 color = ImGui::GetColorU32(
       asset_quote.open()[i] > asset_quote.close()[i] ? bearish_color_
                                                      : bullish_color_);
      ImVec2 open_left_pos =
       ImPlot::PlotToPixels(i - half_width, asset_quote.open()[i]);
      ImVec2 open_right_pos =
       ImPlot::PlotToPixels(i + half_width, asset_quote.open()[i]);
      ImVec2 close_left_pos =
       ImPlot::PlotToPixels(i - half_width, asset_quote.close()[i]);
      ImVec2 close_right_pos =
       ImPlot::PlotToPixels(i + half_width, asset_quote.close()[i]);
      ImVec2 open_close_points[4] = {
       open_left_pos, close_left_pos, close_right_pos, open_right_pos};
      ImVec2 low_pos = ImPlot::PlotToPixels(i, asset_quote.low()[i]);
      ImVec2 high_pos = ImPlot::PlotToPixels(i, asset_quote.high()[i]);
      draw_list->AddLine(
       low_pos,
       high_pos,
       color,
       ImMax(1.0f, ImAbs(open_left_pos.x - close_right_pos.x) / 10.0f));
      draw_list->AddRectFilled(open_left_pos, close_right_pos, color);
      draw_list->AddPolyline(
       open_close_points,
       4,
       color,
       ImDrawFlags_Closed,
       ImMax(1.0f, ImAbs(open_left_pos.x - close_right_pos.x) / 10.0f));
    }

    // end plot item
    ImPlot::EndItem();
  }
}

void PlotDataWindow::DrawClosedTrades(
 const char* label_id, const std::vector<backtest::TradeRecord>& trades)
{
  auto* draw_list = ImPlot::GetPlotDrawList();

  if(ImPlot::BeginItem(label_id)) {
    for(const auto& trade : trades) {
      const auto entry_idx = trade.entry_index();
      const auto entry_price = trade.entry_price();
      const auto exit_idx = trade.exit_index();
      const auto exit_price = trade.exit_price();
      const auto stop_loss_price = trade.stop_loss_price();
      const auto take_profit_price = trade.take_profit_price();

      const auto risk_left_top_pos =
       ImPlot::PlotToPixels(entry_idx, entry_price);
      const auto risk_right_bottom_pos =
       ImPlot::PlotToPixels(exit_idx, stop_loss_price);
      const auto risk_left_bottom_pos =
       ImPlot::PlotToPixels(entry_idx, stop_loss_price);
      const auto risk_right_top_pos =
       ImPlot::PlotToPixels(exit_idx, entry_price);

      ImVec2 risk_points[] = {risk_right_top_pos,
                              risk_right_bottom_pos,
                              risk_left_bottom_pos,
                              risk_left_top_pos};

      const auto reward_left_bottom_pos =
       ImPlot::PlotToPixels(entry_idx, entry_price);
      const auto reward_left_top_pos =
       ImPlot::PlotToPixels(entry_idx, take_profit_price);
      const auto reward_right_top_pos =
       ImPlot::PlotToPixels(exit_idx, take_profit_price);
      const auto reward_right_bottom_pos =
       ImPlot::PlotToPixels(exit_idx, entry_price);

      ImVec2 reward_points[] = {reward_left_bottom_pos,
                                reward_left_top_pos,
                                reward_right_top_pos,
                                reward_right_bottom_pos};

      const auto line_risk_color =
       ImVec4{risk_color_.x, risk_color_.y, risk_color_.z, 0.5f};
      const auto line_reward_color =
       ImVec4{reward_color_.x, reward_color_.y, reward_color_.z, 0.5f};

      draw_list->AddPolyline(risk_points,
                             4,
                             ImGui::GetColorU32(line_risk_color),
                             ImDrawFlags_None,
                             1.0f);
      draw_list->AddPolyline(reward_points,
                             4,
                             ImGui::GetColorU32(line_reward_color),
                             ImDrawFlags_None,
                             1.0f);

      draw_list->AddRectFilled(risk_left_top_pos,
                               risk_right_bottom_pos,
                               ImGui::GetColorU32(risk_color_));
      draw_list->AddRectFilled(reward_left_top_pos,
                               reward_right_bottom_pos,
                               ImGui::GetColorU32(reward_color_));
    }
    ImPlot::EndItem();
  }
}

void PlotDataWindow::DrawOpenTrade(const char* label_id,
                                   const backtest::TradeRecord& trade)
{
  constexpr auto id_right_shift = 10;

  auto* draw_list = ImPlot::GetPlotDrawList();

  if(ImPlot::BeginItem(label_id)) {
    const auto entry_idx = static_cast<std::ptrdiff_t>(trade.entry_index());
    const auto entry_price = trade.entry_price();
    const auto exit_idx = static_cast<std::ptrdiff_t>(trade.exit_index());
    const auto exit_price = trade.exit_price();
    const auto stop_loss_price = trade.stop_loss_price();
    const auto take_profit_price = trade.take_profit_price();

    const auto risk_left_top_pos = ImPlot::PlotToPixels(entry_idx, entry_price);
    const auto risk_right_bottom_pos =
     ImPlot::PlotToPixels(exit_idx - id_right_shift, stop_loss_price);

    const auto reward_left_top_pos =
     ImPlot::PlotToPixels(entry_idx, take_profit_price);
    const auto reward_right_bottom_pos =
     ImPlot::PlotToPixels(exit_idx - id_right_shift, entry_price);

    draw_list->AddRectFilled(
     risk_left_top_pos, risk_right_bottom_pos, ImGui::GetColorU32(risk_color_));
    draw_list->AddRectFilled(reward_left_top_pos,
                             reward_right_bottom_pos,
                             ImGui::GetColorU32(reward_color_));

    ImPlot::EndItem();
  }
}

void PlotDataWindow::PlotVolume(const char* label_id, AssetQuote asset_quote)
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
      for(int i = 0; i < asset_quote.get_asset_size(); ++i) {
        ImPlot::FitPoint(ImPlotPoint(i, 0));
        ImPlot::FitPoint(ImPlotPoint(i, asset_quote.volume()[i]));
      }
    }
    // render data
    for(int i = 0; i < asset_quote.get_asset_size(); ++i) {
      ImU32 color = ImGui::GetColorU32(
       asset_quote.open()[i] > asset_quote.close()[i] ? bearish_color_
                                                      : bullish_color_);
      ImVec2 low_left_pos = ImPlot::PlotToPixels(i - half_width, 0);
      ImVec2 low_right_pos = ImPlot::PlotToPixels(i + half_width, 0);
      ImVec2 high_left_pos =
       ImPlot::PlotToPixels(i - half_width, asset_quote.volume()[i]);
      ImVec2 high_right_pos =
       ImPlot::PlotToPixels(i + half_width, asset_quote.volume()[i]);

      ImVec2 line_points[4] = {
       low_left_pos, high_left_pos, high_right_pos, low_right_pos};
      draw_list->AddRectFilled(low_left_pos, high_right_pos, color);
      draw_list->AddPolyline(line_points, 4, color, ImDrawFlags_Closed, 1.0f);
    }

    // end plot item
    ImPlot::EndItem();
  }
}

} // namespace pludux::apps
