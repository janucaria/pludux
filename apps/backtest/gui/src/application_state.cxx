module;

#include <algorithm>
#include <iterator>
#include <memory>
#include <optional>
#include <queue>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

export module pludux.apps.backtest.application_state;

import pludux.backtest;
import pludux.apps.backtest.portfolio_backtest_selections;

import pludux.apps.backtest.document_state;
import pludux.apps.backtest.view_state;

export namespace pludux::apps {

class ApplicationState {
public:
  ApplicationState() = default;

  ApplicationState(backtest::Store store,
                   DocumentState document_state,
                   ViewState view_state)
  : store_{std::move(store)}
  , document_state_{std::move(document_state)}
  , view_state_{std::move(view_state)}
  {
    normalize_portfolio_backtest_selections();
  }

  auto store(this const ApplicationState& self) noexcept
   -> const backtest::Store&
  {
    return self.store_;
  }

  auto store(this ApplicationState& self) noexcept -> backtest::Store&
  {
    return self.store_;
  }

  auto document_state(this const ApplicationState& self) noexcept
   -> const DocumentState&
  {
    return self.document_state_;
  }

  auto view_state(this const ApplicationState& self) noexcept
   -> const ViewState&
  {
    return self.view_state_;
  }

  auto imgui_ini_settings(this const ApplicationState& self) noexcept
   -> const std::string&
  {
    return self.view_state_.imgui_ini_settings();
  }

  void imgui_ini_settings(this ApplicationState& self,
                          std::string settings) noexcept
  {
    self.view_state_.imgui_ini_settings(std::move(settings));
  }

  void select_backtest(this ApplicationState& self,
                       backtest::BacktestStoreHandle backtest_handle)
  {
    self.view_state_.selected_backtest_handle(backtest_handle);
  }

  void select_portfolio(this ApplicationState& self,
                        backtest::PortfolioStoreHandle handle)
  {
    self.view_state_.selected_portfolio_handle(handle);
    self.normalize_portfolio_backtest_selection(handle);
  }

  auto
  select_portfolio_backtest(this ApplicationState& self,
                            backtest::PortfolioStoreHandle portfolio_handle,
                            backtest::BacktestStoreHandle backtest_handle)
   -> bool
  {
    const auto* portfolio =
     self.store_.get_portfolio_if_present(portfolio_handle);
    if(!portfolio || !self.store_.get_backtest_if_present(backtest_handle) ||
       std::ranges::find(portfolio->backtest_handles(), backtest_handle) ==
        portfolio->backtest_handles().end()) {
      return false;
    }

    self.view_state_.selected_portfolio_handle(portfolio_handle);
    self.view_state_.portfolio_backtest_selections().remember(portfolio_handle,
                                                              backtest_handle);
    return true;
  }

  auto
  selected_portfolio_backtest_handle(this const ApplicationState& self) noexcept
   -> std::optional<backtest::BacktestStoreHandle>
  {
    return self.view_state_.portfolio_backtest_selections().lookup(
     self.view_state_.selected_portfolio_handle());
  }

  auto selected_portfolio_handle(this const ApplicationState& self) noexcept
   -> backtest::PortfolioStoreHandle
  {
    return self.view_state_.selected_portfolio_handle();
  }

  auto selected_portfolio_if_present(this ApplicationState& self) noexcept
   -> backtest::Portfolio*
  {
    return self.store_.get_portfolio_if_present(
     self.view_state_.selected_portfolio_handle());
  }

  auto selected_portfolio_if_present(this const ApplicationState& self) noexcept
   -> const backtest::Portfolio*
  {
    return self.store_.get_portfolio_if_present(
     self.view_state_.selected_portfolio_handle());
  }

  auto get_portfolio_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::PortfolioStoreHandle>&
  {
    return self.document_state_.portfolio_handles();
  }

  auto get_portfolio_handles(this ApplicationState& self) noexcept
   -> std::vector<backtest::PortfolioStoreHandle>&
  {
    return self.document_state_.portfolio_handles();
  }

  void reorder_list_portfolio(this ApplicationState& self,
                              std::size_t from_index,
                              std::size_t to_index) noexcept
  {
    self.document_state_.reorder_portfolio_handle(from_index, to_index);
  }

  auto add_portfolio(this ApplicationState& self, backtest::Portfolio portfolio)
   -> std::optional<backtest::PortfolioStoreHandle>
  {
    const auto handle = self.store_.add_portfolio(std::move(portfolio));
    if(handle) {
      self.document_state_.add_portfolio_handle(*handle);
      self.store_.add_portfolio_results(*handle, {});
    }
    return handle;
  }

  auto get_portfolio(this const ApplicationState& self,
                     backtest::PortfolioStoreHandle handle) noexcept
   -> const backtest::Portfolio&
  {
    return self.store_.get_portfolio(handle);
  }

  auto get_portfolio(this ApplicationState& self,
                     backtest::PortfolioStoreHandle handle) noexcept
   -> backtest::Portfolio&
  {
    return self.store_.get_portfolio(handle);
  }

  auto get_portfolio_if_present(this const ApplicationState& self,
                                backtest::PortfolioStoreHandle handle) noexcept
   -> const backtest::Portfolio*
  {
    return self.store_.get_portfolio_if_present(handle);
  }

  auto get_portfolio_if_present(this ApplicationState& self,
                                backtest::PortfolioStoreHandle handle) noexcept
   -> backtest::Portfolio*
  {
    return self.store_.get_portfolio_if_present(handle);
  }

  auto update_portfolio(this ApplicationState& self,
                        backtest::PortfolioStoreHandle handle,
                        backtest::Portfolio portfolio) -> bool
  {
    if(!self.store_.update_portfolio(handle, std::move(portfolio))) {
      return false;
    }
    self.normalize_portfolio_backtest_selection(handle);
    self.reset_portfolio(handle);
    return true;
  }

  auto rerun_portfolio(this ApplicationState& self,
                       backtest::PortfolioStoreHandle handle) -> bool
  {
    if(!self.store_.get_portfolio_if_present(handle)) {
      return false;
    }
    self.reset_portfolio(handle);
    return true;
  }

  auto remove_portfolio(this ApplicationState& self,
                        backtest::PortfolioStoreHandle handle) -> bool
  {
    if(!self.store_.remove_portfolio(handle)) {
      return false;
    }
    self.document_state_.remove_portfolio_handle(handle);
    self.view_state_.portfolio_backtest_selections().remove_portfolio(handle);
    if(self.view_state_.selected_portfolio_handle() == handle) {
      self.view_state_.selected_portfolio_handle({});
    }
    self.store_.remove_portfolio_results(handle);
    return true;
  }

  auto selected_backtest_handle(this const ApplicationState& self) noexcept
   -> backtest::BacktestStoreHandle
  {
    return self.view_state_.selected_backtest_handle();
  }

  auto selected_backtest(this const ApplicationState& self) noexcept
   -> const backtest::Backtest&
  {
    const auto selected_backtest_handle =
     self.view_state_.selected_backtest_handle();
    return self.store_.get_backtest(selected_backtest_handle);
  }

  auto selected_backtest(this ApplicationState& self) noexcept
   -> backtest::Backtest&
  {
    const auto selected_backtest_handle =
     self.view_state_.selected_backtest_handle();
    return self.store_.get_backtest(selected_backtest_handle);
  }

  auto selected_backtest_if_present(this ApplicationState& self) noexcept
   -> backtest::Backtest*
  {
    const auto selected_backtest_handle =
     self.view_state_.selected_backtest_handle();
    return self.store_.get_backtest_if_present(selected_backtest_handle);
  }

  auto selected_backtest_if_present(this const ApplicationState& self) noexcept
   -> const backtest::Backtest*
  {
    const auto selected_backtest_handle =
     self.view_state_.selected_backtest_handle();
    return self.store_.get_backtest_if_present(selected_backtest_handle);
  }

  auto get_backtest_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::BacktestStoreHandle>&
  {
    return self.document_state_.backtest_handles();
  }

  auto get_backtest_handles(this ApplicationState& self) noexcept
   -> std::vector<backtest::BacktestStoreHandle>&
  {
    return self.document_state_.backtest_handles();
  }

  void reorder_list_backtest(this ApplicationState& self,
                             std::size_t from_index,
                             std::size_t to_index)
  {
    self.document_state_.reorder_backtest_handle(from_index, to_index);
  }

  auto add_backtest(this ApplicationState& self, backtest::Backtest backtest)
   -> std::optional<backtest::BacktestStoreHandle>
  {
    const auto handle_opt = self.store_.add_backtest(std::move(backtest));
    if(handle_opt) {
      self.document_state_.add_backtest_handle(*handle_opt);
    }
    return handle_opt;
  }

  auto get_backtest(this const ApplicationState& self,
                    backtest::BacktestStoreHandle handle) noexcept
   -> const backtest::Backtest&
  {
    return self.store_.get_backtest(handle);
  }

  auto get_backtest(this ApplicationState& self,
                    backtest::BacktestStoreHandle handle) noexcept
   -> backtest::Backtest&
  {
    return self.store_.get_backtest(handle);
  }

  auto get_backtest_if_present(this const ApplicationState& self,
                               backtest::BacktestStoreHandle handle) noexcept
   -> const backtest::Backtest*
  {
    return self.store_.get_backtest_if_present(handle);
  }

  auto get_backtest_if_present(this ApplicationState& self,
                               backtest::BacktestStoreHandle handle) noexcept
   -> backtest::Backtest*
  {
    return self.store_.get_backtest_if_present(handle);
  }

  auto update_backtest(this ApplicationState& self,
                       backtest::BacktestStoreHandle handle,
                       backtest::Backtest backtest) -> bool
  {
    if(self.store_.update_backtest(handle, std::move(backtest))) {
      self.reset_backtest(handle);
      return true;
    }

    return false;
  }

  auto remove_backtest(this ApplicationState& self,
                       backtest::BacktestStoreHandle handle) -> bool
  {
    if(self.store_.remove_backtest(handle)) {
      self.document_state_.remove_backtest_handle(handle);
      self.view_state_.portfolio_backtest_selections().remove_backtest(handle);

      if(self.view_state_.selected_backtest_handle() == handle) {
        self.view_state_.selected_backtest_handle({});
      }

      self.normalize_portfolio_backtest_selections();

      return true;
    }
    return false;
  }

  auto get_asset_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::AssetStoreHandle>&
  {
    return self.document_state_.asset_handles();
  }

  void reorder_list_asset(this ApplicationState& self,
                          std::size_t from_index,
                          std::size_t to_index)
  {
    self.document_state_.reorder_asset_handle(from_index, to_index);
  }

  void add_asset(this ApplicationState& self, backtest::Asset asset)
  {
    const auto handle_opt = self.store_.add_asset(std::move(asset));
    if(handle_opt) {
      self.document_state_.add_asset_handle(*handle_opt);
    }
  }

  auto get_asset(this const ApplicationState& self,
                 backtest::AssetStoreHandle handle) noexcept
   -> const backtest::Asset&
  {
    return self.store_.get_asset(handle);
  }

  auto get_asset_if_present(this const ApplicationState& self,
                            backtest::AssetStoreHandle handle) noexcept
   -> const backtest::Asset*
  {
    return self.store_.get_asset_if_present(handle);
  }

  auto update_asset(this ApplicationState& self,
                    backtest::AssetStoreHandle handle,
                    backtest::Asset edit_asset) -> bool
  {
    const auto& asset = self.get_asset_if_present(handle);
    if(!asset) {
      return false;
    }

    const auto reset_backtests = !asset->equivalent_rules(edit_asset);

    if(self.store_.update_asset(handle, std::move(edit_asset))) {
      if(reset_backtests) {
        const auto& backtest_handles = self.document_state_.backtest_handles();
        for(const auto& backtest_handle : backtest_handles) {
          auto backtest_ptr =
           self.store_.get_backtest_if_present(backtest_handle);
          if(backtest_ptr && backtest_ptr->asset_handle() == handle) {
            self.reset_backtest(backtest_handle);
          }
        }
      }

      return true;
    }

    return false;
  }

  auto remove_asset(this ApplicationState& self,
                    backtest::AssetStoreHandle handle) -> bool
  {
    if(self.store_.remove_asset(handle)) {
      self.document_state_.remove_asset_handle(handle);

      const auto& backtest_handles = self.document_state_.backtest_handles();
      for(const auto& backtest_handle : backtest_handles) {
        const auto backtest_ptr =
         self.store_.get_backtest_if_present(backtest_handle);
        if(backtest_ptr && backtest_ptr->asset_handle() == handle) {
          self.reset_backtest(backtest_handle);
        }
      }

      return true;
    }

    return false;
  }

  auto get_strategy_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::StrategyStoreHandle>&
  {
    return self.document_state_.strategy_handles();
  }

  void reorder_list_strategy(this ApplicationState& self,
                             std::size_t from_index,
                             std::size_t to_index)
  {
    self.document_state_.reorder_strategy_handle(from_index, to_index);
  }

  void add_strategy(this ApplicationState& self, backtest::Strategy strategy)
  {
    const auto handle_opt = self.store_.add_strategy(std::move(strategy));
    if(handle_opt) {
      self.document_state_.add_strategy_handle(*handle_opt);
    }
  }

  auto get_strategy(this const ApplicationState& self,
                    backtest::StrategyStoreHandle handle) noexcept
   -> const backtest::Strategy&
  {
    return self.store_.get_strategy(handle);
  }

  auto get_strategy_if_present(this const ApplicationState& self,
                               backtest::StrategyStoreHandle handle) noexcept
   -> const backtest::Strategy*
  {
    return self.store_.get_strategy_if_present(handle);
  }

  auto update_strategy(this ApplicationState& self,
                       backtest::StrategyStoreHandle handle,
                       backtest::Strategy edit_strategy) -> bool
  {
    const auto& strategy = self.get_strategy_if_present(handle);
    if(!strategy) {
      return false;
    }

    const auto reset_backtests = !strategy->equivalent_rules(edit_strategy);

    if(self.store_.update_strategy(handle, std::move(edit_strategy))) {
      if(reset_backtests) {
        const auto& backtest_handles = self.document_state_.backtest_handles();
        for(const auto& backtest_handle : backtest_handles) {
          auto backtest_ptr =
           self.store_.get_backtest_if_present(backtest_handle);
          if(backtest_ptr && backtest_ptr->strategy_handle() == handle) {
            self.reset_backtest(backtest_handle);
          }
        }
      }

      return true;
    }

    return false;
  }

  auto remove_strategy(this ApplicationState& self,
                       backtest::StrategyStoreHandle handle) -> bool
  {
    if(self.store_.remove_strategy(handle)) {
      self.document_state_.remove_strategy_handle(handle);

      const auto& backtest_handles = self.document_state_.backtest_handles();
      for(const auto& backtest_handle : backtest_handles) {
        const auto backtest_ptr =
         self.store_.get_backtest_if_present(backtest_handle);
        if(backtest_ptr && backtest_ptr->strategy_handle() == handle) {
          self.reset_backtest(backtest_handle);
        }
      }

      return true;
    }

    return false;
  }

  auto get_market_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::MarketStoreHandle>&
  {
    return self.document_state_.market_handles();
  }

  void reorder_list_market(this ApplicationState& self,
                           std::size_t from_index,
                           std::size_t to_index)
  {
    self.document_state_.reorder_market_handle(from_index, to_index);
  }

  void add_market(this ApplicationState& self, backtest::Market market)
  {
    const auto handle_opt = self.store_.add_market(std::move(market));
    if(handle_opt) {
      self.document_state_.add_market_handle(*handle_opt);
    }
  }

  auto get_market(this const ApplicationState& self,
                  backtest::MarketStoreHandle handle) noexcept
   -> const backtest::Market&
  {
    return self.store_.get_market(handle);
  }

  auto get_market_if_present(this const ApplicationState& self,
                             backtest::MarketStoreHandle handle) noexcept
   -> const backtest::Market*
  {
    return self.store_.get_market_if_present(handle);
  }

  auto update_market(this ApplicationState& self,
                     backtest::MarketStoreHandle handle,
                     backtest::Market edit_market) -> bool
  {
    const auto& market = self.get_market_if_present(handle);
    if(!market) {
      return false;
    }

    const auto reset_portfolios = !market->equivalent_rules(edit_market);

    if(self.store_.update_market(handle, std::move(edit_market))) {
      if(reset_portfolios) {
        for(const auto portfolio_handle :
            self.document_state_.portfolio_handles()) {
          const auto* portfolio =
           self.store_.get_portfolio_if_present(portfolio_handle);
          if(portfolio && portfolio->market_handle() == handle) {
            self.reset_portfolio(portfolio_handle);
          }
        }
      }

      return true;
    }

    return false;
  }

  auto remove_market(this ApplicationState& self,
                     backtest::MarketStoreHandle handle) -> bool
  {
    if(self.store_.remove_market(handle)) {
      self.document_state_.remove_market_handle(handle);

      for(const auto portfolio_handle :
          self.document_state_.portfolio_handles()) {
        const auto* portfolio =
         self.store_.get_portfolio_if_present(portfolio_handle);
        if(portfolio && portfolio->market_handle() == handle) {
          self.reset_portfolio(portfolio_handle);
        }
      }

      return true;
    }

    return false;
  }

  auto get_broker_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::BrokerStoreHandle>&
  {
    return self.document_state_.broker_handles();
  }

  void reorder_list_broker(this ApplicationState& self,
                           std::size_t from_index,
                           std::size_t to_index)
  {
    self.document_state_.reorder_broker_handle(from_index, to_index);
  }

  void add_broker(this ApplicationState& self, backtest::Broker broker)
  {
    const auto handle_opt = self.store_.add_broker(std::move(broker));
    if(handle_opt) {
      self.document_state_.add_broker_handle(*handle_opt);
    }
  }

  auto get_broker(this const ApplicationState& self,
                  backtest::BrokerStoreHandle handle) noexcept
   -> const backtest::Broker&
  {
    return self.store_.get_broker(handle);
  }

  auto get_broker_if_present(this const ApplicationState& self,
                             backtest::BrokerStoreHandle handle) noexcept
   -> const backtest::Broker*
  {
    return self.store_.get_broker_if_present(handle);
  }

  auto update_broker(this ApplicationState& self,
                     backtest::BrokerStoreHandle handle,
                     backtest::Broker edit_broker) -> bool
  {
    const auto& broker = self.get_broker_if_present(handle);
    if(!broker) {
      return false;
    }

    const auto reset_portfolios = !broker->equivalent_rules(edit_broker);

    if(self.store_.update_broker(handle, std::move(edit_broker))) {
      if(reset_portfolios) {
        for(const auto portfolio_handle :
            self.document_state_.portfolio_handles()) {
          const auto* portfolio =
           self.store_.get_portfolio_if_present(portfolio_handle);
          if(portfolio && portfolio->broker_handle() == handle) {
            self.reset_portfolio(portfolio_handle);
          }
        }
      }

      return true;
    }

    return false;
  }

  auto remove_broker(this ApplicationState& self,
                     backtest::BrokerStoreHandle handle) -> bool
  {
    if(self.store_.remove_broker(handle)) {
      self.document_state_.remove_broker_handle(handle);

      for(const auto portfolio_handle :
          self.document_state_.portfolio_handles()) {
        const auto* portfolio =
         self.store_.get_portfolio_if_present(portfolio_handle);
        if(portfolio && portfolio->broker_handle() == handle) {
          self.reset_portfolio(portfolio_handle);
        }
      }

      return true;
    }

    return false;
  }

  auto get_profile_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::ProfileStoreHandle>&
  {
    return self.document_state_.profile_handles();
  }

  void reorder_list_profile(this ApplicationState& self,
                            std::size_t from_index,
                            std::size_t to_index)
  {
    self.document_state_.reorder_profile_handle(from_index, to_index);
  }

  void add_profile(this ApplicationState& self, backtest::Profile profile)
  {
    const auto handle_opt = self.store_.add_profile(std::move(profile));
    if(handle_opt) {
      self.document_state_.add_profile_handle(*handle_opt);
    }
  }

  auto get_profile(this const ApplicationState& self,
                   backtest::ProfileStoreHandle handle) noexcept
   -> const backtest::Profile&
  {
    return self.store_.get_profile(handle);
  }

  auto get_profile_if_present(this const ApplicationState& self,
                              backtest::ProfileStoreHandle handle) noexcept
   -> const backtest::Profile*
  {
    return self.store_.get_profile_if_present(handle);
  }

  auto update_profile(this ApplicationState& self,
                      backtest::ProfileStoreHandle handle,
                      backtest::Profile edit_profile) -> bool
  {
    const auto& profile = self.get_profile_if_present(handle);
    if(!profile) {
      return false;
    }

    const auto reset_backtests = !profile->equivalent_rules(edit_profile);
    if(self.store_.update_profile(handle, std::move(edit_profile))) {
      if(reset_backtests) {
        const auto& backtest_handles = self.document_state_.backtest_handles();
        for(const auto& backtest_handle : backtest_handles) {
          auto backtest_ptr =
           self.store_.get_backtest_if_present(backtest_handle);
          if(backtest_ptr && backtest_ptr->profile_handle() == handle) {
            self.reset_backtest(backtest_handle);
          }
        }
      }

      return true;
    }

    return false;
  }

  auto remove_profile(this ApplicationState& self,
                      backtest::ProfileStoreHandle handle) -> bool
  {
    if(self.store_.remove_profile(handle)) {
      self.document_state_.remove_profile_handle(handle);

      const auto& backtest_handles = self.document_state_.backtest_handles();
      for(const auto& backtest_handle : backtest_handles) {
        const auto backtest_ptr =
         self.store_.get_backtest_if_present(backtest_handle);
        if(backtest_ptr && backtest_ptr->profile_handle() == handle) {
          self.reset_backtest(backtest_handle);
        }
      }

      return true;
    }

    return false;
  }

  auto get_portfolio_results(this const ApplicationState& self,
                             backtest::PortfolioStoreHandle handle) noexcept
   -> const backtest::PortfolioResults&
  {
    return self.store_.get_portfolio_results(handle);
  }

  auto get_portfolio_results(this ApplicationState& self,
                             backtest::PortfolioStoreHandle handle) noexcept
   -> backtest::PortfolioResults&
  {
    return self.store_.get_portfolio_results(handle);
  }

  auto get_portfolio_results_if_present(
   this const ApplicationState& self,
   backtest::PortfolioStoreHandle handle) noexcept
   -> const backtest::PortfolioResults*
  {
    return self.store_.get_portfolio_results_if_present(handle);
  }

  auto get_portfolio_results_if_present(
   this ApplicationState& self, backtest::PortfolioStoreHandle handle) noexcept
   -> backtest::PortfolioResults*
  {
    return self.store_.get_portfolio_results_if_present(handle);
  }

  auto update_portfolio_results(this ApplicationState& self,
                                backtest::PortfolioStoreHandle handle,
                                backtest::PortfolioResults results) -> bool
  {
    return self.store_.update_portfolio_results(handle, std::move(results));
  }

  void reset_all_portfolios(this ApplicationState& self)
  {
    for(const auto handle : self.document_state_.portfolio_handles()) {
      self.reset_portfolio(handle);
    }
  }

  auto is_backtest_ready(this const ApplicationState& self,
                         const backtest::Backtest& ready_backtest) noexcept
   -> bool
  {
    {
      const auto asset_handle = ready_backtest.asset_handle();
      const auto asset_ptr = self.get_asset_if_present(asset_handle);

      if(!asset_ptr) {
        return false;
      }
    }
    {
      const auto strategy_handle = ready_backtest.strategy_handle();
      const auto strategy_ptr = self.get_strategy_if_present(strategy_handle);

      if(!strategy_ptr) {
        return false;
      }
    }
    {
      const auto profile_handle = ready_backtest.profile_handle();
      const auto profile_ptr = self.get_profile_if_present(profile_handle);

      if(!profile_ptr) {
        return false;
      }
    }

    return true;
  }

  auto is_portfolio_ready(this const ApplicationState& self,
                          const backtest::Portfolio& portfolio) noexcept -> bool
  {
    if(!self.get_market_if_present(portfolio.market_handle()) ||
       !self.get_broker_if_present(portfolio.broker_handle()) ||
       portfolio.backtest_handles().empty()) {
      return false;
    }
    for(const auto handle : portfolio.backtest_handles()) {
      const auto* backtest = self.get_backtest_if_present(handle);
      if(!backtest || !self.is_backtest_ready(*backtest)) {
        return false;
      }
    }
    return true;
  }

private:
  backtest::Store store_{};
  DocumentState document_state_{};
  ViewState view_state_{};

  void normalize_portfolio_backtest_selection(
   this ApplicationState& self, backtest::PortfolioStoreHandle portfolio_handle)
  {
    auto& selections = self.view_state_.portfolio_backtest_selections();
    const auto* portfolio =
     self.store_.get_portfolio_if_present(portfolio_handle);
    if(!portfolio) {
      selections.remove_portfolio(portfolio_handle);
      return;
    }

    selections.normalize(
     portfolio_handle, portfolio->backtest_handles(), [&](const auto handle) {
       return self.store_.get_backtest_if_present(handle) != nullptr;
     });
  }

  void normalize_portfolio_backtest_selections(this ApplicationState& self)
  {
    const auto selected_portfolio =
     self.view_state_.selected_portfolio_handle();
    if(std::ranges::find(self.document_state_.portfolio_handles(),
                         selected_portfolio) ==
        self.document_state_.portfolio_handles().end() ||
       !self.store_.get_portfolio_if_present(selected_portfolio)) {
      self.view_state_.selected_portfolio_handle({});
    }

    const auto selected_backtest = self.view_state_.selected_backtest_handle();
    if(std::ranges::find(self.document_state_.backtest_handles(),
                         selected_backtest) ==
        self.document_state_.backtest_handles().end() ||
       !self.store_.get_backtest_if_present(selected_backtest)) {
      self.view_state_.selected_backtest_handle({});
    }

    const auto known_selections =
     self.view_state_.portfolio_backtest_selections().selections();
    for(const auto& selection : known_selections) {
      if(std::ranges::find(self.document_state_.portfolio_handles(),
                           selection.portfolio_handle) ==
          self.document_state_.portfolio_handles().end() ||
         !self.store_.get_portfolio_if_present(selection.portfolio_handle)) {
        self.view_state_.portfolio_backtest_selections().remove_portfolio(
         selection.portfolio_handle);
      }
    }

    for(const auto portfolio_handle :
        self.document_state_.portfolio_handles()) {
      self.normalize_portfolio_backtest_selection(portfolio_handle);
    }
  }

  void reset_backtest(this ApplicationState& self,
                      backtest::BacktestStoreHandle handle)
  {
    for(const auto portfolio_handle :
        self.document_state_.portfolio_handles()) {
      const auto* portfolio =
       self.store_.get_portfolio_if_present(portfolio_handle);
      if(portfolio &&
         std::ranges::find(portfolio->backtest_handles(), handle) !=
          portfolio->backtest_handles().end()) {
        self.reset_portfolio(portfolio_handle);
      }
    }
  }

  void reset_portfolio(this ApplicationState& self,
                       backtest::PortfolioStoreHandle handle)
  {
    self.store_.update_portfolio_results(handle, {});
  }
};

} // namespace pludux::apps
