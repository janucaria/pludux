#ifndef PLUDUX_APPS_BACKTEST_WINDOWS_PLOT_DATA_WINDOW_HPP
#define PLUDUX_APPS_BACKTEST_WINDOWS_PLOT_DATA_WINDOW_HPP

#include <vector>

#include <imgui.h>
#include <implot.h>
#include <implot_internal.h>

#include <pludux/asset_quote.hpp>

#include "../app_state.hpp"

namespace pludux::apps {

class PlotDataWindow {
public:
  PlotDataWindow();

  void render(AppState& app_state);

private:
  ImVec4 bullish_color_;
  ImVec4 bearish_color_;

  ImVec4 risk_color_;
  ImVec4 reward_color_;

  ImPlotRange plot_range_{0, 1};

  static int VolumeFormatter(double value, char* buff, int size, void*);

  static void TickerTooltip(const backtest::History& backtest_history,
                            bool span_subplots = false);

  void PlotOHLC(const char* label_id,
                const backtest::History& backtest_history);

  void PlotVolume(const char* label_id,
                  const backtest::History& backtest_history);

  void DrawTrades(const char* label_id,
                  const backtest::History& backtest_history);
};

} // namespace pludux::apps

#endif
