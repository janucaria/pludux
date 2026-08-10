module;

#include <algorithm>
#include <cstddef>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

export module pludux.apps.backtest.portfolio_backtest_setup_selections;

import pludux.backtest;

export namespace pludux::apps {

struct PortfolioBacktestSetupKey {
  backtest::BacktestRunKey run;
  std::size_t setup_index{};

  auto operator==(const PortfolioBacktestSetupKey&) const noexcept
   -> bool = default;
};

struct PortfolioBacktestSetupSelection {
  backtest::PortfolioStoreHandle portfolio_handle;
  PortfolioBacktestSetupKey setup;

  auto operator==(const PortfolioBacktestSetupSelection&) const noexcept
   -> bool = default;
};

class PortfolioBacktestSetupSelections {
public:
  PortfolioBacktestSetupSelections() = default;

  explicit PortfolioBacktestSetupSelections(
   std::vector<PortfolioBacktestSetupSelection> selections)
  {
    for(const auto& selection : selections) {
      remember(selection.portfolio_handle, selection.setup);
    }
  }

  auto selections(this const PortfolioBacktestSetupSelections& self) noexcept
   -> const std::vector<PortfolioBacktestSetupSelection>&
  {
    return self.selections_;
  }

  auto lookup(this const PortfolioBacktestSetupSelections& self,
              backtest::PortfolioStoreHandle portfolio_handle) noexcept
   -> std::optional<PortfolioBacktestSetupKey>
  {
    const auto found =
     std::ranges::find(self.selections_,
                       portfolio_handle,
                       &PortfolioBacktestSetupSelection::portfolio_handle);
    if(found == self.selections_.end()) {
      return std::nullopt;
    }
    return found->setup;
  }

  void remember(this PortfolioBacktestSetupSelections& self,
                backtest::PortfolioStoreHandle portfolio_handle,
                PortfolioBacktestSetupKey setup)
  {
    const auto found =
     std::ranges::find(self.selections_,
                       portfolio_handle,
                       &PortfolioBacktestSetupSelection::portfolio_handle);
    if(found == self.selections_.end()) {
      self.selections_.push_back({portfolio_handle, setup});
      return;
    }
    found->setup = setup;
  }

  void remove_portfolio(this PortfolioBacktestSetupSelections& self,
                        backtest::PortfolioStoreHandle portfolio_handle)
  {
    std::erase_if(self.selections_, [&](const auto& selection) {
      return selection.portfolio_handle == portfolio_handle;
    });
  }

  void remove_backtest(this PortfolioBacktestSetupSelections& self,
                       backtest::BacktestStoreHandle backtest_handle)
  {
    std::erase_if(self.selections_, [&](const auto& selection) {
      return selection.setup.run.backtest_handle == backtest_handle;
    });
  }

  void remove_asset(this PortfolioBacktestSetupSelections& self,
                    backtest::AssetStoreHandle asset_handle)
  {
    std::erase_if(self.selections_, [&](const auto& selection) {
      return selection.setup.run.asset_handle == asset_handle;
    });
  }

  template<typename IsValid>
  auto normalize(this PortfolioBacktestSetupSelections& self,
                 backtest::PortfolioStoreHandle portfolio_handle,
                 std::span<const PortfolioBacktestSetupKey> ordered_setups,
                 IsValid&& is_valid) -> std::optional<PortfolioBacktestSetupKey>
  {
    const auto remembered = self.lookup(portfolio_handle);
    if(remembered &&
       std::ranges::find(ordered_setups, *remembered) != ordered_setups.end() &&
       is_valid(*remembered)) {
      return remembered;
    }

    for(const auto setup : ordered_setups) {
      if(is_valid(setup)) {
        self.remember(portfolio_handle, setup);
        return setup;
      }
    }

    self.remove_portfolio(portfolio_handle);
    return std::nullopt;
  }

  auto operator==(this const PortfolioBacktestSetupSelections& self,
                  const PortfolioBacktestSetupSelections& other) -> bool
  {
    return self.selections_.size() == other.selections_.size() &&
           std::ranges::equal(self.selections_, other.selections_);
  }

private:
  std::vector<PortfolioBacktestSetupSelection> selections_;
};

} // namespace pludux::apps
