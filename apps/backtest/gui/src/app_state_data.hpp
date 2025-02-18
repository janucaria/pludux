#ifndef PLUDUX_APPS_BACKTEST_APP_STATE_DATA_HPP
#define PLUDUX_APPS_BACKTEST_APP_STATE_DATA_HPP

#include <optional>
#include <queue>
#include <string>
#include <vector>

#include <pludux/asset_history.hpp>
#include <pludux/backtest.hpp>
#include <pludux/screener.hpp>

namespace pludux::apps {

struct AppStateData {
  std::string strategy_name{};

  std::string asset_name{};

  bool resource_changed{false};

  std::queue<std::string> alert_messages{};

  std::optional<Backtest> backtest{};

  std::optional<AssetHistory> asset_data{};
};

} // namespace pludux::apps

#endif
