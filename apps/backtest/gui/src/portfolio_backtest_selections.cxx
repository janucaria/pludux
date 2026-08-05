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
  backtest::BacktestStoreHandle backtest_handle;

  auto operator==(const PortfolioBacktestSelection&) const -> bool = default;
};

class PortfolioBacktestSelections {
public:
  PortfolioBacktestSelections() = default;

  explicit PortfolioBacktestSelections(
   std::vector<PortfolioBacktestSelection> selections)
  {
    for(const auto& selection : selections) {
      remember(selection.portfolio_handle, selection.backtest_handle);
    }
  }

  auto selections(this const PortfolioBacktestSelections& self) noexcept
   -> const std::vector<PortfolioBacktestSelection>&
  {
    return self.selections_;
  }

  auto lookup(this const PortfolioBacktestSelections& self,
              backtest::PortfolioStoreHandle portfolio_handle) noexcept
   -> std::optional<backtest::BacktestStoreHandle>
  {
    const auto found =
     std::ranges::find(self.selections_,
                       portfolio_handle,
                       &PortfolioBacktestSelection::portfolio_handle);
    if(found == self.selections_.end()) {
      return std::nullopt;
    }
    return found->backtest_handle;
  }

  void remember(this PortfolioBacktestSelections& self,
                backtest::PortfolioStoreHandle portfolio_handle,
                backtest::BacktestStoreHandle backtest_handle)
  {
    const auto found =
     std::ranges::find(self.selections_,
                       portfolio_handle,
                       &PortfolioBacktestSelection::portfolio_handle);
    if(found == self.selections_.end()) {
      self.selections_.push_back({portfolio_handle, backtest_handle});
      return;
    }
    found->backtest_handle = backtest_handle;
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
      return selection.backtest_handle == backtest_handle;
    });
  }

  template<typename IsValid>
  auto
  normalize(this PortfolioBacktestSelections& self,
            backtest::PortfolioStoreHandle portfolio_handle,
            std::span<const backtest::BacktestStoreHandle> ordered_backtests,
            IsValid&& is_valid) -> std::optional<backtest::BacktestStoreHandle>
  {
    const auto remembered = self.lookup(portfolio_handle);
    if(remembered &&
       std::ranges::find(ordered_backtests, *remembered) !=
        ordered_backtests.end() &&
       is_valid(*remembered)) {
      return remembered;
    }

    for(const auto backtest_handle : ordered_backtests) {
      if(is_valid(backtest_handle)) {
        self.remember(portfolio_handle, backtest_handle);
        return backtest_handle;
      }
    }

    self.remove_portfolio(portfolio_handle);
    return std::nullopt;
  }

  auto operator==(const PortfolioBacktestSelections&) const -> bool = default;

private:
  std::vector<PortfolioBacktestSelection> selections_;
};

} // namespace pludux::apps
