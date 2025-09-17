module;

#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include <pludux/asset_history.hpp>
#include <pludux/backtest.hpp>
#include <pludux/screener.hpp>

export module pludux.apps.backtest.app_state_data;

export import pludux.backtest;

export namespace pludux::apps {

struct AppStateData {
  std::queue<std::string> alert_messages{};

  std::vector<Backtest> backtests{};

  std::ptrdiff_t selected_backtest_index{-1};

  std::vector<std::shared_ptr<backtest::Strategy>> strategies{};

  std::vector<std::shared_ptr<backtest::Asset>> assets{};

  std::vector<std::shared_ptr<backtest::Profile>> profiles{};
};

} // namespace pludux::apps
