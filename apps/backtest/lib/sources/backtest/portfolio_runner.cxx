module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

export module pludux.backtest:portfolio_runner;

import :backtest_method_context;
import :backtest_runner;
import :portfolio;
import :portfolio_results;
import :store_handle;

export namespace pludux::backtest {

class PortfolioRunner {
public:
  class BacktestRun {
  public:
    BacktestRun(BacktestStoreHandle handle, BacktestRunner runner)
    : handle_{handle}
    , runner_{std::move(runner)}
    {
    }

    auto handle(this const BacktestRun& self) noexcept -> BacktestStoreHandle
    {
      return self.handle_;
    }

    auto runner(this BacktestRun& self) noexcept -> BacktestRunner&
    {
      return self.runner_;
    }

    auto runner(this const BacktestRun& self) noexcept -> const BacktestRunner&
    {
      return self.runner_;
    }

  private:
    BacktestStoreHandle handle_;
    BacktestRunner runner_;
  };

  PortfolioRunner(double initial_capital, std::vector<BacktestRun> backtests)
  : initial_capital_{initial_capital}
  , account_{initial_capital, 0.0, initial_capital, initial_capital}
  , backtests_{std::move(backtests)}
  {
    if(!std::isfinite(initial_capital) || initial_capital < 0.0) {
      throw std::invalid_argument{
       "Portfolio initial capital must be finite and non-negative"};
    }
    self_validate_assets();
    total_timestamps_ = collect_timestamps().size();
  }

  auto is_failed(this const PortfolioRunner& self) noexcept -> bool
  {
    return self.is_failed_;
  }

  void is_failed(this PortfolioRunner& self, bool value) noexcept
  {
    self.is_failed_ = value;
  }

  auto total_timestamps(this const PortfolioRunner& self) noexcept
   -> std::size_t
  {
    return self.total_timestamps_;
  }

  void initialize_results(this const PortfolioRunner& self,
                          PortfolioResults& results)
  {
    if(!results.backtests().empty()) {
      return;
    }
    results.backtests().reserve(self.backtests_.size());
    for(const auto& backtest : self.backtests_) {
      results.backtests().emplace_back(backtest.handle());
    }
    results.timeline().reserve(self.total_timestamps_);
  }

  void run(this PortfolioRunner& self, PortfolioResults& results)
  {
    if(self.is_failed_) {
      return;
    }
    self.initialize_results(results);

    auto next_timestamp = std::numeric_limits<std::time_t>::max();
    for(auto index = std::size_t{}; index < self.backtests_.size(); ++index) {
      const auto& asset = self.backtests_[index].runner().asset();
      const auto& backtest_result = results.backtests()[index];
      if(backtest_result.timeline().size() >= asset.size()) {
        continue;
      }
      const auto snapshot =
       asset.get_snapshot(backtest_result.timeline().size());
      next_timestamp =
       std::min(next_timestamp, static_cast<std::time_t>(snapshot.datetime()));
    }
    if(next_timestamp == std::numeric_limits<std::time_t>::max()) {
      return;
    }

    auto active = std::vector<std::size_t>{};
    for(auto index = std::size_t{}; index < self.backtests_.size(); ++index) {
      auto& backtest = self.backtests_[index];
      auto& result = results.backtests()[index];
      const auto& asset = backtest.runner().asset();
      if(result.timeline().size() >= asset.size()) {
        continue;
      }
      const auto snapshot = asset.get_snapshot(result.timeline().size());
      if(static_cast<std::time_t>(snapshot.datetime()) != next_timestamp) {
        continue;
      }
      self.sync_backtest(backtest.runner());
      if(backtest.runner().begin_bar(result.timeline())) {
        active.push_back(index);
      }
    }

    const auto run_exit_phase = [&](auto action) {
      for(const auto index : active) {
        auto& runner = self.backtests_[index].runner();
        self.sync_backtest(runner);
        const auto capital_before = self.account_.capital();
        action(runner, results.backtests()[index]);
        runner.settle_portfolio_account();
        self.account_.capital(
         self.account_.capital() +
         (runner.account_state().capital() - capital_before));
        self.refresh_unrealized();
      }
    };
    const auto run_entry_phase = [&](auto action) {
      for(const auto index : active) {
        auto& runner = self.backtests_[index].runner();
        self.sync_backtest(runner);
        action(runner, results.backtests()[index]);
        self.refresh_unrealized();
      }
    };

    run_exit_phase(
     [](BacktestRunner& runner, BacktestResults&) { runner.run_open_exits(); });
    run_entry_phase([](BacktestRunner& runner, BacktestResults& result) {
      runner.run_open_entries(result.series_results());
    });
    run_exit_phase(
     [](BacktestRunner& runner, BacktestResults&) { runner.run_intrabar(); });
    run_exit_phase([](BacktestRunner& runner, BacktestResults& result) {
      runner.run_close_exits(result.series_results());
    });
    run_entry_phase([](BacktestRunner& runner, BacktestResults& result) {
      runner.run_close_entries(result.series_results());
    });
    for(const auto index : active) {
      auto& runner = self.backtests_[index].runner();
      auto& result = results.backtests()[index];
      self.sync_backtest(runner);
      runner.finish_bar(result.series_results(), result.timeline());
      self.refresh_unrealized();
    }

    self.refresh_unrealized();
    self.account_.update_peak_to_current_equity();
    const auto drawdown = self.account_.drawdown();
    self.max_drawdown_ = std::max(self.max_drawdown_, drawdown);
    const auto reserved = self.reserved_notional();
    const auto net = self.net_exposure();
    auto fresh_count = std::size_t{};
    auto stale_count = std::size_t{};
    auto unavailable_count = std::size_t{};
    for(const auto& backtest : results.backtests()) {
      if(backtest.timeline().empty()) {
        ++unavailable_count;
      } else if(backtest.timeline().market_timestamp(
                 backtest.timeline().size() - 1) == next_timestamp) {
        ++fresh_count;
      } else {
        ++stale_count;
      }
    }
    results.timeline().append(PortfolioTimeline::Row{
     .timestamp = next_timestamp,
     .capital = self.account_.capital(),
     .available_capital = std::max(self.account_.capital() - reserved, 0.0),
     .equity = self.account_.equity(),
     .peak_equity = self.account_.peak_equity(),
     .drawdown = drawdown,
     .max_drawdown = self.max_drawdown_,
     .realized_pnl = self.account_.capital() - self.initial_capital_,
     .unrealized_pnl = self.account_.unrealized_pnl(),
     .reserved_notional = reserved,
     .gross_exposure = reserved,
     .net_exposure = net,
     .open_position_count = self.open_position_count(),
     .fresh_backtest_count = fresh_count,
     .stale_backtest_count = stale_count,
     .unavailable_backtest_count = unavailable_count});
  }

private:
  double initial_capital_{};
  BacktestAccountState account_;
  std::vector<BacktestRun> backtests_;
  double max_drawdown_{};
  std::size_t total_timestamps_{};
  bool is_failed_{};

  void self_validate_assets(this const PortfolioRunner& self)
  {
    for(const auto& backtest : self.backtests_) {
      const auto& asset = backtest.runner().asset();
      auto previous = -std::numeric_limits<double>::infinity();
      for(auto index = std::size_t{}; index < asset.size(); ++index) {
        const auto timestamp = asset.get_snapshot(index).datetime();
        if(!std::isfinite(timestamp) || timestamp <= previous) {
          throw std::invalid_argument{
           "Portfolio assets require finite, strictly increasing timestamps"};
        }
        previous = timestamp;
      }
    }
  }

  auto collect_timestamps(this const PortfolioRunner& self)
   -> std::vector<std::time_t>
  {
    auto timestamps = std::vector<std::time_t>{};
    for(const auto& backtest : self.backtests_) {
      const auto& asset = backtest.runner().asset();
      for(auto index = std::size_t{}; index < asset.size(); ++index) {
        timestamps.push_back(
         static_cast<std::time_t>(asset.get_snapshot(index).datetime()));
      }
    }
    std::ranges::sort(timestamps);
    const auto last = std::ranges::unique(timestamps).begin();
    timestamps.erase(last, timestamps.end());
    return timestamps;
  }

  void refresh_unrealized(this PortfolioRunner& self) noexcept
  {
    auto unrealized = 0.0;
    for(const auto& backtest : self.backtests_) {
      unrealized += backtest.runner().unrealized_pnl();
    }
    self.account_.unrealized_pnl(unrealized);
  }

  void sync_backtest(this PortfolioRunner& self,
                     BacktestRunner& runner) noexcept
  {
    self.refresh_unrealized();
    self.account_.update_peak_to_current_equity();
    runner.portfolio_account(self.account_, self.reserved_notional());
  }

  auto reserved_notional(this const PortfolioRunner& self) noexcept -> double
  {
    auto result = 0.0;
    for(const auto& backtest : self.backtests_) {
      result += backtest.runner().reserved_notional();
    }
    return result;
  }

  auto net_exposure(this const PortfolioRunner& self) noexcept -> double
  {
    auto result = 0.0;
    for(const auto& backtest : self.backtests_) {
      result += backtest.runner().net_exposure();
    }
    return result;
  }

  auto open_position_count(this const PortfolioRunner& self) noexcept
   -> std::size_t
  {
    return static_cast<std::size_t>(
     std::ranges::count_if(self.backtests_, [](const BacktestRun& value) {
       return value.runner().has_open_position();
     }));
  }
};

} // namespace pludux::backtest
