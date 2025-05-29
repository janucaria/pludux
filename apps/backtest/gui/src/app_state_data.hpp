#ifndef PLUDUX_APPS_BACKTEST_APP_STATE_DATA_HPP
#define PLUDUX_APPS_BACKTEST_APP_STATE_DATA_HPP

#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include <pludux/asset_history.hpp>
#include <pludux/backtest.hpp>
#include <pludux/backtest/asset.hpp>
#include <pludux/screener.hpp>

namespace pludux::apps {

struct AppStateData {
  std::queue<std::string> alert_messages{};

  std::vector<Backtest> backtests{};

  std::vector<std::unique_ptr<backtest::Strategy>> strategies{};

  std::ptrdiff_t selected_strategy_index{-1};

  std::vector<std::unique_ptr<backtest::Asset>> assets{};

  std::ptrdiff_t selected_asset_index{-1};

  QuoteAccess quote_access{};
};

} // namespace pludux::apps

#endif
