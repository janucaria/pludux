module;

#include <algorithm>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

export module pludux.apps.backtest.portfolio_backtest_selections;

import pludux.backtest;

export namespace pludux::apps {

struct PortfolioBacktestSelection {
  backtest::PortfolioStoreHandle portfolio_handle;
  backtest::BacktestRunKey run;

  auto operator==(const PortfolioBacktestSelection& other) const -> bool
  {
    return portfolio_handle == other.portfolio_handle && run == other.run;
  }
};

class PortfolioBacktestSelections {
public:
  PortfolioBacktestSelections() = default;

  explicit PortfolioBacktestSelections(
   std::vector<PortfolioBacktestSelection> selections)
  {
    for(const auto& selection : selections) {
      remember(selection.portfolio_handle, selection.run);
    }
  }

  auto selections(this const PortfolioBacktestSelections& self) noexcept
   -> const std::vector<PortfolioBacktestSelection>&
  {
    return self.selections_;
  }

  auto lookup(this const PortfolioBacktestSelections& self,
              backtest::PortfolioStoreHandle portfolio_handle) noexcept
   -> std::optional<backtest::BacktestRunKey>
  {
    const auto found =
     std::ranges::find(self.selections_,
                       portfolio_handle,
                       &PortfolioBacktestSelection::portfolio_handle);
    if(found == self.selections_.end()) {
      return std::nullopt;
    }
    return found->run;
  }

  void remember(this PortfolioBacktestSelections& self,
                backtest::PortfolioStoreHandle portfolio_handle,
                backtest::BacktestRunKey run)
  {
    const auto found =
     std::ranges::find(self.selections_,
                       portfolio_handle,
                       &PortfolioBacktestSelection::portfolio_handle);
    if(found == self.selections_.end()) {
      self.selections_.push_back({portfolio_handle, run});
      return;
    }
    found->run = run;
  }

  void remove_portfolio(this PortfolioBacktestSelections& self,
                        backtest::PortfolioStoreHandle portfolio_handle)
  {
    std::erase_if(self.selections_, [&](const auto& selection) {
      return selection.portfolio_handle == portfolio_handle;
    });
  }

  void remove_backtest(this PortfolioBacktestSelections& self,
                       backtest::BacktestStoreHandle backtest_handle)
  {
    std::erase_if(self.selections_, [&](const auto& selection) {
      return selection.run.backtest_handle == backtest_handle;
    });
  }

  void remove_asset(this PortfolioBacktestSelections& self,
                    backtest::AssetStoreHandle asset_handle)
  {
    std::erase_if(self.selections_, [&](const auto& selection) {
      return selection.run.asset_handle == asset_handle;
    });
  }

  template<typename IsValid>
  auto normalize(this PortfolioBacktestSelections& self,
                 backtest::PortfolioStoreHandle portfolio_handle,
                 std::span<const backtest::BacktestRunKey> ordered_runs,
                 IsValid&& is_valid) -> std::optional<backtest::BacktestRunKey>
  {
    const auto remembered = self.lookup(portfolio_handle);
    if(remembered &&
       std::ranges::find(ordered_runs, *remembered) != ordered_runs.end() &&
       is_valid(*remembered)) {
      return remembered;
    }

    for(const auto run : ordered_runs) {
      if(is_valid(run)) {
        self.remember(portfolio_handle, run);
        return run;
      }
    }

    self.remove_portfolio(portfolio_handle);
    return std::nullopt;
  }

  auto operator==(this const PortfolioBacktestSelections& self,
                  const PortfolioBacktestSelections& other) -> bool
  {
    return self.selections_.size() == other.selections_.size() &&
           std::ranges::equal(self.selections_, other.selections_);
  }

private:
  std::vector<PortfolioBacktestSelection> selections_;
};

} // namespace pludux::apps
