#include <algorithm>
#include <array>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include <pludux/asset_quote.hpp>

#include "../app_state.hpp"
#include "./plot_data_window.hpp"

namespace pludux::apps {

PlotDataWindow::PlotDataWindow()
: last_selected_backtest_index_{-1}
, bullish_color_{0.5, 1, 0, 1}
, bearish_color_{1, 0, 0.5, 1}
, risk_color_{1, 0, 0, 0.25}
, reward_color_{0, 1, 0, 0.25}
, trailing_stop_color_{1, 0., 0., 0.9}
{
}

void PlotDataWindow::render(AppState& app_state)
{
  const auto& state = app_state.state();

  ImGui::Begin("Charts", nullptr);

  if(state.selected_asset_index < 0) {
    ImGui::End();
    return;
  }

  const auto& quote_access = state.quote_access;
  const auto& asset = *state.assets[state.selected_asset_index];
  const auto& asset_history = asset.history();

  auto trade_records = std::vector<backtest::TradeRecord>{};
  auto is_backtest_should_run = false;
  if(!state.backtests.empty()) {
    const auto& backtest = state.backtests[state.selected_asset_index];
    is_backtest_should_run = backtest.should_run();
    trade_records = backtest.trade_records();
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
   last_selected_backtest_index_ != state.selected_asset_index;
  if(is_backtest_should_run || reset_chart_view) {
    axis_x_flags |= ImPlotAxisFlags_AutoFit;
    axis_y_flags |= ImPlotAxisFlags_AutoFit;

    last_selected_backtest_index_ = state.selected_asset_index;
  }

  if(ImPlot::BeginPlot("##OHLCPlot",
                       {-1, -1},
                       ImPlotFlags_NoLegend | ImPlotFlags_NoMouseText |
                        ImPlotFlags_NoBoxSelect)) {
    ImPlot::SetupAxisLinks(ImAxis_X1, &plot_range_.Min, &plot_range_.Max);
    ImPlot::SetupAxis(ImAxis_X1,
                      nullptr,
                      axis_x_flags | ImPlotAxisFlags_NoTickLabels |
                       ImPlotAxisFlags_NoHighlight);
    ImPlot::SetupAxisLimits(ImAxis_X1, 0, asset_history.size() + 100);

    ImPlot::SetupAxis(ImAxis_Y1, nullptr, axis_y_flags);
    ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");

    DrawTrades("Trades", trade_records, asset_history, quote_access);
    TickerTooltip(asset_history, quote_access, false);
    PlotOHLC("OHLC", asset_history, quote_access);

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

          ImPlot::SetupAxis(ImAxis_X1, nullptr, axis_x_flags);
          ImPlot::SetupAxis(
           ImAxis_Y1, nullptr, axis_y_flags | ImPlotAxisFlags_LockMin);
          ImPlot::SetupAxisFormat(ImAxis_Y1, VolumeFormatter);

          TickerTooltip(asset_history, quote_access, false);
          PlotVolume("Volume", asset_history, quote_access);

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

void PlotDataWindow::TickerTooltip(const AssetHistory& asset_history,
                                   const QuoteAccess& quote_access,
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
      const auto& snapshot = asset_history[idx];
      const auto& quote = AssetQuote{snapshot, quote_access};

      ImGui::BeginTooltip();
      char buff[32];
      ImPlot::FormatDate(ImPlotTime::FromDouble(quote.time()),
                         buff,
                         32,
                         ImPlotDateFmt_DayMoYr,
                         ImPlot::GetStyle().UseISO8601);
      ImGui::Text("Date:");
      ImGui::SameLine(60);
      ImGui::Text("%s", buff);
      ImGui::Text("Open:");
      ImGui::SameLine(60);
      ImGui::Text("%.2f", quote.open());
      ImGui::Text("Close:");
      ImGui::SameLine(60);
      ImGui::Text("%.2f", quote.close());
      ImGui::Text("High:");
      ImGui::SameLine(60);
      ImGui::Text("%.2f", quote.high());
      ImGui::Text("Low:");
      ImGui::SameLine(60);
      ImGui::Text("%.2f", quote.low());
      ImGui::Text("Volume:");
      ImGui::SameLine(60);
      ImGui::Text(
       "%s",
       std::format(std::locale("en_US.UTF-8"), "{:L}", (int)(quote.volume()))
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
                              const AssetHistory& asset_history,
                              const QuoteAccess& quote_access)
{
  auto* draw_list = ImPlot::GetPlotDrawList();
  constexpr double half_width = 0.3;

  if(ImPlot::BeginItem(label_id)) {
    ImPlot::GetCurrentItem()->Color = ImGui::GetColorU32(bullish_color_);

    if(ImPlot::FitThisFrame()) {
      for(int i = 0; i < asset_history.size(); ++i) {
        const auto& snapshot = asset_history[i];
        const auto& quote = AssetQuote{snapshot, quote_access};

        ImPlot::FitPoint(ImPlotPoint(i, quote.low()));
        ImPlot::FitPoint(ImPlotPoint(i, quote.high()));
      }
    }

    for(int i = 0, ii = asset_history.size(); i < ii; ++i) {
      const auto& snapshot = asset_history[i];
      const auto& quote = AssetQuote{snapshot, quote_access};

      const auto open = quote.open();
      const auto high = quote.high();
      const auto low = quote.low();
      const auto close = quote.close();

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
                                const AssetHistory& asset_history,
                                const QuoteAccess& quote_access)
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
      for(int i = 0; i < asset_history.size(); ++i) {
        const auto& snapshot = asset_history[i];
        const auto& quote = AssetQuote{snapshot, quote_access};

        ImPlot::FitPoint(ImPlotPoint(i, 0));
        ImPlot::FitPoint(ImPlotPoint(i, quote.volume()));
      }
    }
    // render data
    for(int i = 0, ii = asset_history.size(); i < ii; ++i) {
      const auto& snapshot = asset_history[i];
      const auto& quote = AssetQuote{snapshot, quote_access};

      const auto open = quote.open();
      const auto close = quote.close();
      const auto volume = quote.volume();

      ImU32 color =
       ImGui::GetColorU32(open > close ? bearish_color_ : bullish_color_);

      ImVec2 low_left_pos = ImPlot::PlotToPixels(i - half_width, 0);
      ImVec2 low_right_pos = ImPlot::PlotToPixels(i + half_width, 0);
      ImVec2 high_left_pos = ImPlot::PlotToPixels(i - half_width, volume);
      ImVec2 high_right_pos = ImPlot::PlotToPixels(i + half_width, volume);

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

void PlotDataWindow::DrawTrades(
 const char* label_id,
 const std::vector<backtest::TradeRecord>& trade_records,
 const AssetHistory& asset_history,
 const QuoteAccess& quote_access)
{
  auto* draw_list = ImPlot::GetPlotDrawList();

  constexpr double half_width = 0.5;
  if(ImPlot::BeginItem(label_id)) {
    auto trailing_stop_lines = std::vector<ImVec2>{};
    trailing_stop_lines.reserve(trade_records.size());

    for(int i = 0, ii = trade_records.size(); i < ii; ++i) {
      const auto& trade = trade_records[i];

      const auto take_profit_price = trade.take_profit_price();
      const auto stop_loss_price = trade.stop_loss_price();
      const auto trailing_stop_price = trade.trailing_stop_price();
      const auto entry_idx = trade.entry_index();
      const auto asset_idx = trade.at_index();
      const auto entry_price = trade.entry_price();
      const auto current_price = trade.exit_price();

      const auto top_price = !std::isnan(take_profit_price)
                              ? take_profit_price
                              : std::max(entry_price, current_price);
      const auto middle_price = std::max(entry_price, stop_loss_price);
      const auto bottom_price = !std::isnan(stop_loss_price)
                                 ? stop_loss_price
                                 : std::min(middle_price, current_price);

      const auto left_half_width = asset_idx == entry_idx ? 0.0 : half_width;
      const auto right_half_width =
       trade.is_open() ? (asset_idx == 0 ? 10.0 : half_width) : 0.0;

      {
        const auto risk_left_top_pos =
         ImPlot::PlotToPixels(asset_idx + left_half_width, middle_price);
        const auto risk_right_bottom_pos =
         ImPlot::PlotToPixels(asset_idx - right_half_width, bottom_price);

        draw_list->AddRectFilled(risk_left_top_pos,
                                 risk_right_bottom_pos,
                                 ImGui::GetColorU32(risk_color_));
      }
      {
        const auto reward_left_top_pos =
         ImPlot::PlotToPixels(asset_idx + left_half_width, top_price);
        const auto reward_right_bottom_pos =
         ImPlot::PlotToPixels(asset_idx - right_half_width, middle_price);

        draw_list->AddRectFilled(reward_left_top_pos,
                                 reward_right_bottom_pos,
                                 ImGui::GetColorU32(reward_color_));
      }

      {
        if(!trade.is_flat()) {
          const auto left_pos =
           ImPlot::PlotToPixels(asset_idx + half_width, trailing_stop_price);
          const auto center_pos =
           ImPlot::PlotToPixels(asset_idx, trailing_stop_price);
          const auto right_pos =
           ImPlot::PlotToPixels(asset_idx - half_width, trailing_stop_price);

          trailing_stop_lines.push_back(left_pos);
          trailing_stop_lines.push_back(center_pos);
          trailing_stop_lines.push_back(right_pos);

          if(trade.is_closed()) {
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
                               ImGui::GetColorU32(trailing_stop_color_),
                               ImDrawFlags_None,
                               1.2f);
      }
    }

    ImPlot::EndItem();
  }
}

} // namespace pludux::apps
