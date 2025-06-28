#ifndef PLUDUX_APPS_BACKTEST_APP_STATE_DATA_HPP
#define PLUDUX_APPS_BACKTEST_APP_STATE_DATA_HPP

#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <pludux/asset_history.hpp>
#include <pludux/backtest.hpp>
#include <pludux/backtest/asset.hpp>
#include <pludux/screener.hpp>

namespace pludux::apps {

struct StateAssetHash {
  using is_transparent = void;

  auto operator()(std::string_view asset_name) const -> std::size_t
  {
    return std::hash<std::string_view>{}(asset_name);
  }

  auto operator()(const backtest::Asset& asset) const -> std::size_t
  {
    return this->operator()(asset.name());
  }

  auto operator()(const std::shared_ptr<backtest::Asset>& asset_ptr) const
   -> std::size_t
  {
    return this->operator()(*asset_ptr);
  }
};

struct StateAssetKeyEqual {
  using is_transparent = void;

  auto operator()(const backtest::Asset& lhs, const backtest::Asset& rhs) const
   -> bool
  {
    return lhs.name() == rhs.name();
  }

  auto operator()(std::string_view lhs, const backtest::Asset& rhs) const
   -> bool
  {
    return lhs == rhs.name();
  }

  auto operator()(const backtest::Asset& lhs, std::string_view rhs) const
   -> bool
  {
    return lhs.name() == rhs;
  }

  auto operator()(const std::shared_ptr<backtest::Asset>& lhs,
                  const std::shared_ptr<backtest::Asset>& rhs) const -> bool
  {
    return this->operator()(*lhs, *rhs);
  }

  auto operator()(std::string_view lhs,
                  const std::shared_ptr<backtest::Asset>& rhs) const -> bool
  {
    return this->operator()(lhs, *rhs);
  }

  auto operator()(const std::shared_ptr<backtest::Asset>& lhs,
                  std::string_view rhs) const -> bool
  {
    return this->operator()(*lhs, rhs);
  }

  auto operator()(const std::shared_ptr<backtest::Asset>& lhs,
                  const backtest::Asset& rhs) const -> bool
  {
    return this->operator()(*lhs, rhs);
  }

  auto operator()(const backtest::Asset& lhs,
                  const std::shared_ptr<backtest::Asset>& rhs) const -> bool
  {
    return this->operator()(lhs, *rhs);
  }
};

struct AppStateData {
  std::queue<std::string> alert_messages{};

  std::vector<Backtest> backtests{};

  std::ptrdiff_t selected_backtest_index{-1};

  std::vector<std::shared_ptr<backtest::Strategy>> strategies{};

  std::unordered_set<std::shared_ptr<backtest::Asset>,
                     StateAssetHash,
                     StateAssetKeyEqual>
   assets{};

  QuoteAccess quote_access{};
};

} // namespace pludux::apps

#endif
