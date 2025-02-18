#ifndef PLUDUX_APPS_BACKTEST_WINDOWS_PLOT_DATA_WINDOW_HPP
#define PLUDUX_APPS_BACKTEST_WINDOWS_PLOT_DATA_WINDOW_HPP

#include <vector>

#include <imgui.h>

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

  static int VolumeFormatter(double value, char* buff, int size, void*);

  static void TickerTooltip(AssetQuote asset_quote, bool span_subplots = false);

  void PlotOHLC(const char* label_id, AssetQuote asset_quote);

  void DrawClosedTrades(const char* label_id,
                          const std::vector<backtest::TradeRecord>& trades);

  void DrawOpenTrade(const char* label_id, const backtest::TradeRecord& trade);

  void PlotVolume(const char* label_id, AssetQuote asset_quote);
};

} // namespace pludux::apps

#endif
