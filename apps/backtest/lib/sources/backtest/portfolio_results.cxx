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
  BacktestStoreHandle backtest_handle;
  AssetStoreHandle asset_handle;

  auto operator==(const BacktestRunKey&) const noexcept -> bool = default;
};

class BacktestResults {
public:
  BacktestResults() = default;

  BacktestResults(BacktestStoreHandle backtest_handle,
                  AssetStoreHandle asset_handle,
                  BacktestTimeline timeline = {},
                  SeriesEvaluationResults series_results = {})
  : backtest_handle_{backtest_handle}
  , asset_handle_{asset_handle}
  , timeline_{std::move(timeline)}
  , series_results_{std::move(series_results)}
  {
  }

  auto operator==(this const BacktestResults& self,
                  const BacktestResults& other) noexcept -> bool
  {
    return self.backtest_handle_.slot_index() ==
            other.backtest_handle_.slot_index() &&
           self.backtest_handle_.generation() ==
            other.backtest_handle_.generation() &&
           self.asset_handle_ == other.asset_handle_ &&
           self.timeline_.size() == other.timeline_.size() &&
           self.series_results_.results().size() ==
            other.series_results_.results().size();
  }

  auto backtest_handle(this const BacktestResults& self) noexcept
   -> BacktestStoreHandle
  {
    return self.backtest_handle_;
  }

  auto asset_handle(this const BacktestResults& self) noexcept
   -> AssetStoreHandle
  {
    return self.asset_handle_;
  }

  auto key(this const BacktestResults& self) noexcept -> BacktestRunKey
  {
    return {self.backtest_handle_, self.asset_handle_};
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

  auto series_results(this const BacktestResults& self) noexcept
   -> const SeriesEvaluationResults&
  {
    return self.series_results_;
  }

  auto series_results(this BacktestResults& self) noexcept
   -> SeriesEvaluationResults&
  {
    return self.series_results_;
  }

private:
  BacktestStoreHandle backtest_handle_;
  AssetStoreHandle asset_handle_;
  BacktestTimeline timeline_;
  SeriesEvaluationResults series_results_;
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
