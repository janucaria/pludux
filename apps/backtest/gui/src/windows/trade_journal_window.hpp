#ifndef PLUDUX_APPS_BACKTEST_WINDOWS_TRADE_JOURNAL_WINDOW_HPP
#define PLUDUX_APPS_BACKTEST_WINDOWS_TRADE_JOURNAL_WINDOW_HPP

#include <ctime>
#include <string>

#include <imgui.h>

#include "../app_state.hpp"

namespace pludux::apps {

class TradeJournalWindow {
public:
  TradeJournalWindow();

  void render(AppState& app_state);

private:
  static void draw_trade_row(int trade_count,
                             const backtest::TradeRecord& trade);

  static auto format_trade_status(backtest::TradeRecord::Status status) noexcept
   -> std::string;
};

} // namespace pludux::apps

#endif
