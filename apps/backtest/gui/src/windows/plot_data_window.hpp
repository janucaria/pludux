#ifndef PLUDUX_APPS_BACKTEST_WINDOWS_PLOT_DATA_WINDOW_HPP
#define PLUDUX_APPS_BACKTEST_WINDOWS_PLOT_DATA_WINDOW_HPP

#include <vector>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include <pludux/backtest.hpp>

#include "../app_state.hpp"

namespace pludux::apps {

class PlotDataWindow {
public:
  PlotDataWindow();

  void render(AppState& app_state);

private:
  std::ptrdiff_t last_selected_backtest_index_;

  ImVec4 bullish_color_;
  ImVec4 bearish_color_;

  ImVec4 risk_color_;
  ImVec4 reward_color_;

  ImVec4 trailing_stop_color_;

  ImPlotRange plot_range_{0, 1};

  static int VolumeFormatter(double value, char* buff, int size, void*);

  static void TickerTooltip(const AssetHistory& asset_history,
                            bool span_subplots = false);

  void PlotOHLC(const char* label_id, const AssetHistory& asset_history);

  void PlotVolume(const char* label_id, const AssetHistory& asset_history);

  void
  DrawTrades(const char* label_id,
             const std::vector<backtest::BacktestSummary>& backtest_summaries,
             const AssetHistory& asset_history);

  void
  plot_equity(const std::vector<backtest::BacktestSummary>& backtest_summaries);
};

} // namespace pludux::apps

#endif
