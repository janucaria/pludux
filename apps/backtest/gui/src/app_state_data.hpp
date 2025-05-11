#ifndef PLUDUX_APPS_BACKTEST_APP_STATE_DATA_HPP
#define PLUDUX_APPS_BACKTEST_APP_STATE_DATA_HPP

#include <optional>
#include <queue>
#include <string>
#include <vector>
#include <list>

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

  std::ptrdiff_t selected_backtest_index{-1};

  std::list<backtest::Asset> assets{};
};

} // namespace pludux::apps

#endif
