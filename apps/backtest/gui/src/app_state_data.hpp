#ifndef PLUDUX_APPS_BACKTEST_APP_STATE_DATA_HPP
#define PLUDUX_APPS_BACKTEST_APP_STATE_DATA_HPP

#include <optional>
#include <queue>
#include <string>
#include <vector>

#include <pludux/asset_history.hpp>
#include <pludux/backtest.hpp>
#include <pludux/backtest/asset.hpp>
#include <pludux/screener.hpp>

namespace pludux::apps {

struct AppStateData {
  std::string strategy_name{};

  std::queue<std::string> alert_messages{};

  std::vector<Backtest> backtests{};

  std::optional<backtest::Strategy> strategy{};

  std::vector<backtest::Asset> assets{};

  std::ptrdiff_t selected_asset_index{-1};

  QuoteAccess quote_access{};
};

} // namespace pludux::apps

#endif
