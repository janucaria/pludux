module;

#include <utility>
#include <vector>

export module pludux.backtest:store_arena;

import pludux;

import :backtest;
import :asset;
import :strategy;
import :market;
import :broker;
import :profile;
import :backtest_summary;

export namespace pludux::backtest {

class StoreArena {
public:
  StoreArena() = default;

  StoreArena(std::vector<Backtest> backtests,
             std::vector<Asset> assets,
             std::vector<Strategy> strategies,
             std::vector<Market> markets,
             std::vector<Broker> brokers,
             std::vector<Profile> profiles,
             std::vector<std::vector<BacktestSummary>> backtest_summaries,
             std::vector<SeriesResultsCollector> series_results)
  : backtests_{std::move(backtests)}
  , assets_{std::move(assets)}
  , strategies_{std::move(strategies)}
  , markets_{std::move(markets)}
  , brokers_{std::move(brokers)}
  , profiles_{std::move(profiles)}
  , backtest_summaries_{std::move(backtest_summaries)}
  , series_results_{std::move(series_results)}
  {
  }

  auto backtests(this const StoreArena& self) noexcept
   -> const std::vector<Backtest>&
  {
    return self.backtests_;
  }

  auto backtests(this StoreArena& self) noexcept -> std::vector<Backtest>&
  {
    return self.backtests_;
  }

  auto assets(this const StoreArena& self) noexcept -> const std::vector<Asset>&
  {
    return self.assets_;
  }

  auto assets(this StoreArena& self) noexcept -> std::vector<Asset>&
  {
    return self.assets_;
  }

  auto strategies(this const StoreArena& self) noexcept
   -> const std::vector<Strategy>&
  {
    return self.strategies_;
  }

  auto strategies(this StoreArena& self) noexcept -> std::vector<Strategy>&
  {
    return self.strategies_;
  }

  auto markets(this const StoreArena& self) noexcept
   -> const std::vector<Market>&
  {
    return self.markets_;
  }

  auto markets(this StoreArena& self) noexcept -> std::vector<Market>&
  {
    return self.markets_;
  }

  auto brokers(this const StoreArena& self) noexcept
   -> const std::vector<Broker>&
  {
    return self.brokers_;
  }

  auto brokers(this StoreArena& self) noexcept -> std::vector<Broker>&
  {
    return self.brokers_;
  }

  auto profiles(this const StoreArena& self) noexcept
   -> const std::vector<Profile>&
  {
    return self.profiles_;
  }

  auto profiles(this StoreArena& self) noexcept -> std::vector<Profile>&
  {
    return self.profiles_;
  }

  auto backtest_summaries(this const StoreArena& self) noexcept
   -> const std::vector<std::vector<BacktestSummary>>&
  {
    return self.backtest_summaries_;
  }

  auto backtest_summaries(this StoreArena& self) noexcept
   -> std::vector<std::vector<BacktestSummary>>&
  {
    return self.backtest_summaries_;
  }

  auto series_results(this const StoreArena& self) noexcept
   -> const std::vector<SeriesResultsCollector>&
  {
    return self.series_results_;
  }

  auto series_results(this StoreArena& self) noexcept
   -> std::vector<SeriesResultsCollector>&
  {
    return self.series_results_;
  }

private:
  std::vector<Backtest> backtests_;
  std::vector<Asset> assets_;
  std::vector<Strategy> strategies_;
  std::vector<Market> markets_;
  std::vector<Broker> brokers_;
  std::vector<Profile> profiles_;
  std::vector<std::vector<BacktestSummary>> backtest_summaries_;
  std::vector<SeriesResultsCollector> series_results_;
};

} // namespace pludux::backtest
