module;

#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

export module pludux.apps.backtest:app_state_data;

import pludux.backtest;

export namespace pludux::apps {

struct AppStateData {
  std::queue<std::string> alert_messages{};

  std::vector<Backtest> backtests{};

  std::ptrdiff_t selected_backtest_index{-1};

  std::vector<std::shared_ptr<backtest::Strategy>> strategies{};

  std::vector<std::shared_ptr<backtest::Asset>> assets{};

  std::vector<std::shared_ptr<backtest::Profile>> profiles{};

  std::vector<std::shared_ptr<backtest::Broker>> brokers{};
};

} // namespace pludux::apps
