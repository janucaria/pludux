module;

#include <cstddef>
#include <utility>
#include <vector>

export module pludux.backtest:portfolio_results;

import pludux;

import :backtest_timeline;
import :portfolio_timeline;
import :store_handle;

export namespace pludux::backtest {

struct BacktestRunKey {
  SystemStoreHandle system_handle;
  AssetStoreHandle asset_handle;

  auto operator==(const BacktestRunKey&) const noexcept -> bool = default;
};

class BacktestResults {
public:
  BacktestResults() = default;

  BacktestResults(SystemStoreHandle system_handle,
                  AssetStoreHandle asset_handle,
                  BacktestTimeline timeline = {},
                   std::size_t strategy_count = 1)
  : system_handle_{system_handle}
  , asset_handle_{asset_handle}
  , timeline_{std::move(timeline)}
   , strategy_series_results_(strategy_count)
  {
  }

  auto operator==(this const BacktestResults& self,
                  const BacktestResults& other) noexcept -> bool
  {
    return self.system_handle_.slot_index() == other.system_handle_.slot_index() &&
            self.system_handle_.generation() == other.system_handle_.generation() &&
           self.asset_handle_ == other.asset_handle_ &&
           self.timeline_.size() == other.timeline_.size() &&
            self.strategy_series_results_.size() ==
             other.strategy_series_results_.size();
  }

  auto system_handle(this const BacktestResults& self) noexcept
     -> SystemStoreHandle
  {
    return self.system_handle_;
  }

  auto asset_handle(this const BacktestResults& self) noexcept
   -> AssetStoreHandle
  {
    return self.asset_handle_;
  }

  auto key(this const BacktestResults& self) noexcept -> BacktestRunKey
  {
    return {self.system_handle_, self.asset_handle_};
  }

  auto timeline(this const BacktestResults& self) noexcept
   -> const BacktestTimeline&
  {
    return self.timeline_;
  }

  auto timeline(this BacktestResults& self) noexcept -> BacktestTimeline&
  {
    return self.timeline_;
  }

  auto series_results(this const BacktestResults& self,
                      std::size_t strategy_index)
   -> const SeriesEvaluationResults&
  {
    return self.strategy_series_results_.at(strategy_index);
  }

  auto series_results(this BacktestResults& self, std::size_t strategy_index)
   -> SeriesEvaluationResults&
  {
    return self.strategy_series_results_.at(strategy_index);
  }

  auto strategy_series_results(this BacktestResults& self) noexcept
   -> std::vector<SeriesEvaluationResults>&
  {
    return self.strategy_series_results_;
  }

  auto strategy_series_results(this const BacktestResults& self) noexcept
   -> const std::vector<SeriesEvaluationResults>&
  {
    return self.strategy_series_results_;
  }

private:
  SystemStoreHandle system_handle_;
  AssetStoreHandle asset_handle_;
  BacktestTimeline timeline_;
  std::vector<SeriesEvaluationResults> strategy_series_results_;
};

class PortfolioResults {
public:
  PortfolioResults() = default;

  PortfolioResults(PortfolioTimeline timeline,
                   std::vector<BacktestResults> backtests)
  : timeline_{std::move(timeline)}
  , backtests_{std::move(backtests)}
  {
  }

  auto operator==(const PortfolioResults&) const noexcept -> bool = default;

  auto timeline(this const PortfolioResults& self) noexcept
   -> const PortfolioTimeline&
  {
    return self.timeline_;
  }

  auto timeline(this PortfolioResults& self) noexcept -> PortfolioTimeline&
  {
    return self.timeline_;
  }

  auto backtests(this const PortfolioResults& self) noexcept
   -> const std::vector<BacktestResults>&
  {
    return self.backtests_;
  }

  auto backtests(this PortfolioResults& self) noexcept
   -> std::vector<BacktestResults>&
  {
    return self.backtests_;
  }

  auto backtest(this PortfolioResults& self, BacktestRunKey key) noexcept
   -> BacktestResults*
  {
    for(auto& result : self.backtests_) {
      if(result.key() == key) {
        return &result;
      }
    }
    return nullptr;
  }

  auto backtest(this const PortfolioResults& self, BacktestRunKey key) noexcept
   -> const BacktestResults*
  {
    for(const auto& result : self.backtests_) {
      if(result.key() == key) {
        return &result;
      }
    }
    return nullptr;
  }

private:
  PortfolioTimeline timeline_;
  std::vector<BacktestResults> backtests_;
};

} // namespace pludux::backtest
