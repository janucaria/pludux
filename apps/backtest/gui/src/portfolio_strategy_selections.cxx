module;

#include <algorithm>
#include <cstddef>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

export module pludux.apps.backtest.portfolio_strategy_selections;

import pludux.backtest;

export namespace pludux::apps {

struct PortfolioStrategyKey {
  backtest::BacktestRunKey run;
  std::size_t strategy_index{};

  auto operator==(const PortfolioStrategyKey&) const noexcept
   -> bool = default;
};

struct PortfolioStrategySelection {
  backtest::PortfolioStoreHandle portfolio_handle;
  PortfolioStrategyKey strategy;

  auto operator==(const PortfolioStrategySelection&) const noexcept
   -> bool = default;
};

class PortfolioStrategySelections {
public:
  PortfolioStrategySelections() = default;

  explicit PortfolioStrategySelections(
   std::vector<PortfolioStrategySelection> selections)
  {
    for(const auto& selection : selections) {
      remember(selection.portfolio_handle, selection.strategy);
    }
  }

  auto selections(this const PortfolioStrategySelections& self) noexcept
   -> const std::vector<PortfolioStrategySelection>&
  {
    return self.selections_;
  }

  auto lookup(this const PortfolioStrategySelections& self,
              backtest::PortfolioStoreHandle portfolio_handle) noexcept
    -> std::optional<PortfolioStrategyKey>
  {
    const auto found =
     std::ranges::find(self.selections_,
                       portfolio_handle,
                         &PortfolioStrategySelection::portfolio_handle);
    if(found == self.selections_.end()) {
      return std::nullopt;
    }
    return found->strategy;
  }

  void remember(this PortfolioStrategySelections& self,
                backtest::PortfolioStoreHandle portfolio_handle,
                PortfolioStrategyKey strategy)
  {
    const auto found =
     std::ranges::find(self.selections_,
                       portfolio_handle,
                         &PortfolioStrategySelection::portfolio_handle);
    if(found == self.selections_.end()) {
      self.selections_.push_back({portfolio_handle, strategy});
      return;
    }
    found->strategy = strategy;
  }

  void remove_portfolio(this PortfolioStrategySelections& self,
                        backtest::PortfolioStoreHandle portfolio_handle)
  {
    std::erase_if(self.selections_, [&](const auto& selection) {
      return selection.portfolio_handle == portfolio_handle;
    });
  }

  void remove_system(this PortfolioStrategySelections& self,
                     backtest::SystemStoreHandle system_handle)
  {
    std::erase_if(self.selections_, [&](const auto& selection) {
        return selection.strategy.run.system_handle == system_handle;
    });
  }

  void remove_asset(this PortfolioStrategySelections& self,
                    backtest::AssetStoreHandle asset_handle)
  {
    std::erase_if(self.selections_, [&](const auto& selection) {
       return selection.strategy.run.asset_handle == asset_handle;
    });
  }

  template<typename IsValid>
  auto normalize(this PortfolioStrategySelections& self,
                 backtest::PortfolioStoreHandle portfolio_handle,
                  std::span<const PortfolioStrategyKey> ordered_strategies,
                  IsValid&& is_valid) -> std::optional<PortfolioStrategyKey>
  {
    const auto remembered = self.lookup(portfolio_handle);
    if(remembered &&
        std::ranges::find(ordered_strategies, *remembered) !=
         ordered_strategies.end() &&
       is_valid(*remembered)) {
      return remembered;
    }

    for(const auto strategy : ordered_strategies) {
      if(is_valid(strategy)) {
        self.remember(portfolio_handle, strategy);
        return strategy;
      }
    }

    self.remove_portfolio(portfolio_handle);
    return std::nullopt;
  }

  auto operator==(this const PortfolioStrategySelections& self,
                  const PortfolioStrategySelections& other) -> bool
  {
    return self.selections_.size() == other.selections_.size() &&
           std::ranges::equal(self.selections_, other.selections_);
  }

private:
  std::vector<PortfolioStrategySelection> selections_;
};

} // namespace pludux::apps
