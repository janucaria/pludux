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
import pludux.apps.backtest.portfolio_strategy_selections;

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
    normalize_portfolio_strategy_selections();
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

  void select_system(this ApplicationState& self,
                     backtest::SystemStoreHandle system_handle)
  {
    self.view_state_.selected_system_handle(system_handle);
  }

  void select_strategy(this ApplicationState& self,
                       backtest::StrategyStoreHandle strategy_handle)
  {
    self.view_state_.selected_strategy_handle(strategy_handle);
  }

  void select_portfolio(this ApplicationState& self,
                        backtest::PortfolioStoreHandle handle)
  {
    self.view_state_.selected_portfolio_handle(handle);
    self.normalize_portfolio_strategy_selection(handle);
  }

  auto select_portfolio_strategy(
   this ApplicationState& self,
   backtest::PortfolioStoreHandle portfolio_handle,
    PortfolioStrategyKey strategy) -> bool
  {
    const auto* portfolio =
     self.store_.get_portfolio_if_present(portfolio_handle);
    if(!portfolio) {
      return false;
    }
    const auto strategies = self.expanded_system_strategies(*portfolio);
    if(std::ranges::find(strategies, strategy) == strategies.end()) {
      return false;
    }
    const auto* backtest =
     self.store_.get_system_if_present(strategy.run.system_handle);
    if(!backtest || strategy.strategy_index >= backtest->strategy_count() ||
        !self.store_.get_asset_if_present(strategy.run.asset_handle)) {
      return false;
    }
    const auto* configured_strategy = self.strategy_if_present(*backtest,
                                                               strategy.strategy_index);
    if(!configured_strategy ||
       !self.store_.get_model_if_present(configured_strategy->model_handle()) ||
       !self.store_.get_profile_if_present(configured_strategy->profile_handle())) {
      return false;
    }

    self.view_state_.selected_portfolio_handle(portfolio_handle);
    self.view_state_.portfolio_strategy_selections().remember(
      portfolio_handle, strategy);
    return true;
  }

  auto
   selected_portfolio_strategy(this const ApplicationState& self) noexcept
    -> std::optional<PortfolioStrategyKey>
  {
    return self.view_state_.portfolio_strategy_selections().lookup(
     self.view_state_.selected_portfolio_handle());
  }

  auto expanded_system_strategies(this const ApplicationState& self,
                                const backtest::Portfolio& portfolio)
    -> std::vector<PortfolioStrategyKey>
  {
    auto strategies = std::vector<PortfolioStrategyKey>{};
      for(const auto system_handle : portfolio.system_handles()) {
      const auto* configured_backtest =
         self.store_.get_system_if_present(system_handle);
      if(!configured_backtest) {
         strategies.push_back({{system_handle, {}}, 0});
        continue;
      }

      const auto append_asset_strategies = [&](const auto asset_handle) {
         for(auto strategy_index = std::size_t{};
             strategy_index < configured_backtest->strategy_count();
             ++strategy_index) {
            strategies.push_back({{system_handle, asset_handle}, strategy_index});
        }
      };
      const auto* watchlist = self.store_.get_watchlist_if_present(
       configured_backtest->watchlist_handle());
      if(!watchlist) {
         append_asset_strategies(backtest::AssetStoreHandle{});
        continue;
      }
      for(const auto asset_handle : watchlist->asset_handles()) {
         append_asset_strategies(asset_handle);
      }
    }
    return strategies;
  }

  auto expanded_strategy_runs(this const ApplicationState& self,
                              const backtest::Portfolio& portfolio)
   -> std::vector<backtest::BacktestRunKey>
  {
    auto runs = std::vector<backtest::BacktestRunKey>{};
      for(const auto system_handle : portfolio.system_handles()) {
      const auto* configured_backtest =
         self.store_.get_system_if_present(system_handle);
      if(!configured_backtest) {
        continue;
      }
      const auto* watchlist = self.store_.get_watchlist_if_present(
       configured_backtest->watchlist_handle());
      if(!watchlist) {
        continue;
      }
      for(const auto asset_handle : watchlist->asset_handles()) {
        if(self.store_.get_asset_if_present(asset_handle)) {
            runs.push_back({system_handle, asset_handle});
        }
      }
    }
    return runs;
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
    self.normalize_portfolio_strategy_selection(handle);
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
     self.view_state_.portfolio_strategy_selections().remove_portfolio(
     handle);
    if(self.view_state_.selected_portfolio_handle() == handle) {
      self.view_state_.selected_portfolio_handle({});
    }
    self.store_.remove_portfolio_results(handle);
    return true;
  }

  auto selected_system_handle(this const ApplicationState& self) noexcept
     -> backtest::SystemStoreHandle
  {
    return self.view_state_.selected_system_handle();
  }

  auto selected_system(this const ApplicationState& self) noexcept
     -> const backtest::System&
  {
    const auto selected_system_handle = self.view_state_.selected_system_handle();
    return self.store_.get_system(selected_system_handle);
  }

  auto selected_system(this ApplicationState& self) noexcept
     -> backtest::System&
  {
    const auto selected_system_handle = self.view_state_.selected_system_handle();
    return self.store_.get_system(selected_system_handle);
  }

  auto selected_system_if_present(this ApplicationState& self) noexcept
     -> backtest::System*
  {
    const auto selected_system_handle = self.view_state_.selected_system_handle();
    return self.store_.get_system_if_present(selected_system_handle);
  }

  auto selected_system_if_present(this const ApplicationState& self) noexcept
     -> const backtest::System*
  {
    const auto selected_system_handle = self.view_state_.selected_system_handle();
    return self.store_.get_system_if_present(selected_system_handle);
  }

  auto get_system_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::SystemStoreHandle>&
  {
    return self.document_state_.system_handles();
  }

  auto get_system_handles(this ApplicationState& self) noexcept
   -> std::vector<backtest::SystemStoreHandle>&
  {
    return self.document_state_.system_handles();
  }

  void reorder_list_system(this ApplicationState& self,
                             std::size_t from_index,
                             std::size_t to_index)
  {
    self.document_state_.reorder_system_handle(from_index, to_index);
  }

  auto add_system(this ApplicationState& self, backtest::System system)
    -> std::optional<backtest::SystemStoreHandle>
  {
    const auto handle_opt = self.store_.add_system(std::move(system));
    if(handle_opt) {
      self.document_state_.add_system_handle(*handle_opt);
    }
    return handle_opt;
  }

  auto get_system(this const ApplicationState& self,
                      backtest::SystemStoreHandle handle) noexcept
    -> const backtest::System&
  {
    return self.store_.get_system(handle);
  }

  auto get_system(this ApplicationState& self,
                      backtest::SystemStoreHandle handle) noexcept
    -> backtest::System&
  {
    return self.store_.get_system(handle);
  }

  auto get_system_if_present(this const ApplicationState& self,
                                 backtest::SystemStoreHandle handle) noexcept
    -> const backtest::System*
  {
    return self.store_.get_system_if_present(handle);
  }

  auto get_system_if_present(this ApplicationState& self,
                                 backtest::SystemStoreHandle handle) noexcept
    -> backtest::System*
  {
    return self.store_.get_system_if_present(handle);
  }

  auto update_system(this ApplicationState& self,
                         backtest::SystemStoreHandle handle,
                         backtest::System system) -> bool
  {
    if(self.store_.update_system(handle, std::move(system))) {
      self.reset_system(handle);
      self.normalize_portfolio_strategy_selections();
      return true;
    }

    return false;
  }

  auto remove_system(this ApplicationState& self,
                          backtest::SystemStoreHandle handle) -> bool
  {
    if(self.store_.remove_system(handle)) {
      self.document_state_.remove_system_handle(handle);
      self.view_state_.portfolio_strategy_selections().remove_system(
       handle);

      if(self.view_state_.selected_system_handle() == handle) {
        self.view_state_.selected_system_handle({});
      }

      // Portfolios deliberately retain the missing handle for repair, but
      // their results were produced with the removed System.
      self.reset_system(handle);
      self.normalize_portfolio_strategy_selections();

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
        for(const auto watchlist_handle :
            self.document_state_.watchlist_handles()) {
          const auto* watchlist =
           self.store_.get_watchlist_if_present(watchlist_handle);
          if(watchlist &&
             std::ranges::find(watchlist->asset_handles(), handle) !=
              watchlist->asset_handles().end()) {
            self.reset_watchlist(watchlist_handle);
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

      for(const auto watchlist_handle :
          self.document_state_.watchlist_handles()) {
        const auto* watchlist =
         self.store_.get_watchlist_if_present(watchlist_handle);
        if(watchlist && std::ranges::find(watchlist->asset_handles(), handle) !=
                         watchlist->asset_handles().end()) {
          self.reset_watchlist(watchlist_handle);
        }
      }
      self.view_state_.portfolio_strategy_selections().remove_asset(
       handle);
      self.normalize_portfolio_strategy_selections();

      return true;
    }

    return false;
  }

  auto get_watchlist_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::WatchlistStoreHandle>&
  {
    return self.document_state_.watchlist_handles();
  }

  void reorder_list_watchlist(this ApplicationState& self,
                              std::size_t from_index,
                              std::size_t to_index)
  {
    self.document_state_.reorder_watchlist_handle(from_index, to_index);
  }

  auto add_watchlist(this ApplicationState& self, backtest::Watchlist watchlist)
   -> std::optional<backtest::WatchlistStoreHandle>
  {
    const auto handle = self.store_.add_watchlist(std::move(watchlist));
    if(handle) {
      self.document_state_.add_watchlist_handle(*handle);
    }
    return handle;
  }

  auto get_watchlist(this const ApplicationState& self,
                     backtest::WatchlistStoreHandle handle) noexcept
   -> const backtest::Watchlist&
  {
    return self.store_.get_watchlist(handle);
  }

  auto get_watchlist_if_present(this const ApplicationState& self,
                                backtest::WatchlistStoreHandle handle) noexcept
   -> const backtest::Watchlist*
  {
    return self.store_.get_watchlist_if_present(handle);
  }

  auto update_watchlist(this ApplicationState& self,
                        backtest::WatchlistStoreHandle handle,
                        backtest::Watchlist watchlist) -> bool
  {
    const auto* existing = self.store_.get_watchlist_if_present(handle);
    if(!existing) {
      return false;
    }
    const auto rules_changed = !existing->equivalent_rules(watchlist);
    if(!self.store_.update_watchlist(handle, std::move(watchlist))) {
      return false;
    }
    if(rules_changed) {
      self.reset_watchlist(handle);
      self.normalize_portfolio_strategy_selections();
    }
    return true;
  }

  auto remove_watchlist(this ApplicationState& self,
                        backtest::WatchlistStoreHandle handle) -> bool
  {
    if(!self.store_.remove_watchlist(handle)) {
      return false;
    }
    self.document_state_.remove_watchlist_handle(handle);
    self.reset_watchlist(handle);
    self.normalize_portfolio_strategy_selections();
    return true;
  }

  auto get_model_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::ModelStoreHandle>&
  {
    return self.document_state_.model_handles();
  }

  void reorder_list_model(this ApplicationState& self,
                             std::size_t from_index,
                             std::size_t to_index)
  {
    self.document_state_.reorder_model_handle(from_index, to_index);
  }

  void add_model(this ApplicationState& self, backtest::Model model)
  {
    const auto handle_opt = self.store_.add_model(std::move(model));
    if(handle_opt) {
      self.document_state_.add_model_handle(*handle_opt);
    }
  }

  auto get_model(this const ApplicationState& self,
                     backtest::ModelStoreHandle handle) noexcept
   -> const backtest::Model&
  {
    return self.store_.get_model(handle);
  }

  auto get_model_if_present(this const ApplicationState& self,
                                backtest::ModelStoreHandle handle) noexcept
   -> const backtest::Model*
  {
    return self.store_.get_model_if_present(handle);
  }

  auto update_model(this ApplicationState& self,
                         backtest::ModelStoreHandle handle,
                         backtest::Model edit_model) -> bool
  {
    const auto& model = self.get_model_if_present(handle);
    if(!model) {
      return false;
    }

    const auto reset_strategies = !model->equivalent_rules(edit_model);

    if(self.store_.update_model(handle, std::move(edit_model))) {
      // This happens while an EditCommand is still working on its candidate
      // state, so both the Model and dependent stored input values participate
      // in the same StateDiff used by undo and redo.
      self.sync_strategies_referencing_model(handle);
      if(reset_strategies) {
        self.reset_strategies_referencing_model(handle);
      }

      return true;
    }

    return false;
  }

  auto remove_model(this ApplicationState& self,
                        backtest::ModelStoreHandle handle) -> bool
  {
    if(self.store_.remove_model(handle)) {
      self.document_state_.remove_model_handle(handle);

      self.reset_strategies_referencing_model(handle);
      self.normalize_portfolio_strategy_selections();

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
        self.reset_strategies_referencing_profile(handle);
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

      self.reset_strategies_referencing_profile(handle);
      self.normalize_portfolio_strategy_selections();

      return true;
    }

    return false;
  }

  auto get_strategy_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::StrategyStoreHandle>&
  {
    return self.document_state_.strategy_handles();
  }

  auto selected_strategy_handle(this const ApplicationState& self) noexcept
   -> backtest::StrategyStoreHandle
  {
    return self.view_state_.selected_strategy_handle();
  }

  auto selected_strategy_if_present(this ApplicationState& self) noexcept
   -> backtest::Strategy*
  {
    return self.store_.get_strategy_if_present(
     self.view_state_.selected_strategy_handle());
  }

  auto selected_strategy_if_present(this const ApplicationState& self) noexcept
   -> const backtest::Strategy*
  {
    return self.store_.get_strategy_if_present(
     self.view_state_.selected_strategy_handle());
  }

  void reorder_list_strategy(this ApplicationState& self,
                             std::size_t from_index,
                             std::size_t to_index)
  {
    self.document_state_.reorder_strategy_handle(from_index, to_index);
  }

  void add_strategy(this ApplicationState& self, backtest::Strategy strategy)
  {
    if(const auto handle = self.store_.add_strategy(std::move(strategy))) {
      self.document_state_.add_strategy_handle(*handle);
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
                       backtest::Strategy strategy) -> bool
  {
    if(!self.store_.update_strategy(handle, std::move(strategy))) return false;
    self.reset_strategy(handle);
    self.normalize_portfolio_strategy_selections();
    return true;
  }

  auto remove_strategy(this ApplicationState& self,
                       backtest::StrategyStoreHandle handle) -> bool
  {
    if(!self.store_.remove_strategy(handle)) return false;
    self.document_state_.remove_strategy_handle(handle);
    if(self.view_state_.selected_strategy_handle() == handle)
      self.view_state_.selected_strategy_handle({});
    // Systems intentionally retain missing strategy handles for repair.
    self.reset_strategy(handle);
    self.normalize_portfolio_strategy_selections();
    return true;
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

  auto is_system_ready(this const ApplicationState& self,
                          const backtest::System& ready_system) noexcept
   -> bool
  {
    {
      const auto* watchlist =
         self.get_watchlist_if_present(ready_system.watchlist_handle());
      if(!watchlist || watchlist->asset_handles().empty()) {
        return false;
      }
      for(const auto asset_handle : watchlist->asset_handles()) {
        if(!self.get_asset_if_present(asset_handle)) {
          return false;
        }
      }
    }
    for(auto index = std::size_t{}; index < ready_system.strategy_count();
        ++index) {
      const auto* strategy = self.strategy_if_present(ready_system, index);
      if(!strategy || !self.get_model_if_present(strategy->model_handle()) ||
          !self.get_profile_if_present(strategy->profile_handle())) {
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
         portfolio.system_handles().empty()) {
      return false;
    }
    for(const auto handle : portfolio.system_handles()) {
      const auto* system = self.get_system_if_present(handle);
      if(!system || !self.is_system_ready(*system)) {
        return false;
      }
    }
    return true;
  }

private:
  backtest::Store store_{};
  DocumentState document_state_{};
  ViewState view_state_{};

  void normalize_portfolio_strategy_selection(
   this ApplicationState& self, backtest::PortfolioStoreHandle portfolio_handle)
  {
    auto& selections = self.view_state_.portfolio_strategy_selections();
    const auto* portfolio =
     self.store_.get_portfolio_if_present(portfolio_handle);
    if(!portfolio) {
      selections.remove_portfolio(portfolio_handle);
      return;
    }

    const auto strategies = self.expanded_system_strategies(*portfolio);
    selections.normalize(portfolio_handle, strategies, [&](const auto strategy) {
      const auto* system =
          self.store_.get_system_if_present(strategy.run.system_handle);
      if(!system || strategy.strategy_index >= system->strategy_count() ||
          !self.store_.get_asset_if_present(strategy.run.asset_handle)) {
        return false;
      }
      const auto* configured_strategy = self.strategy_if_present(*system,
                                                                  strategy.strategy_index);
      return configured_strategy &&
             self.store_.get_model_if_present(configured_strategy->model_handle()) &&
              self.store_.get_profile_if_present(
                configured_strategy->profile_handle());
    });
  }

  void
   normalize_portfolio_strategy_selections(this ApplicationState& self)
  {
    const auto selected_portfolio =
     self.view_state_.selected_portfolio_handle();
    if(std::ranges::find(self.document_state_.portfolio_handles(),
                         selected_portfolio) ==
        self.document_state_.portfolio_handles().end() ||
       !self.store_.get_portfolio_if_present(selected_portfolio)) {
      self.view_state_.selected_portfolio_handle({});
    }

    const auto selected_system = self.view_state_.selected_system_handle();
    if(std::ranges::find(self.document_state_.system_handles(), selected_system) ==
          self.document_state_.system_handles().end() ||
        !self.store_.get_system_if_present(selected_system)) {
       self.view_state_.selected_system_handle({});
    }

    const auto selected_strategy = self.view_state_.selected_strategy_handle();
    if(std::ranges::find(self.document_state_.strategy_handles(), selected_strategy) ==
         self.document_state_.strategy_handles().end() ||
       !self.store_.get_strategy_if_present(selected_strategy)) {
      self.view_state_.selected_strategy_handle({});
    }

    const auto known_selections =
       self.view_state_.portfolio_strategy_selections().selections();
    for(const auto& selection : known_selections) {
      if(std::ranges::find(self.document_state_.portfolio_handles(),
                           selection.portfolio_handle) ==
          self.document_state_.portfolio_handles().end() ||
         !self.store_.get_portfolio_if_present(selection.portfolio_handle)) {
         self.view_state_.portfolio_strategy_selections().remove_portfolio(
         selection.portfolio_handle);
      }
    }

    for(const auto portfolio_handle :
        self.document_state_.portfolio_handles()) {
       self.normalize_portfolio_strategy_selection(portfolio_handle);
    }
  }

  void reset_system(this ApplicationState& self,
                    backtest::SystemStoreHandle handle)
  {
    for(const auto portfolio_handle :
        self.document_state_.portfolio_handles()) {
      const auto* portfolio =
       self.store_.get_portfolio_if_present(portfolio_handle);
      if(portfolio &&
           std::ranges::find(portfolio->system_handles(), handle) !=
            portfolio->system_handles().end()) {
        self.reset_portfolio(portfolio_handle);
      }
    }
  }

  auto strategy_if_present(this const ApplicationState& self,
                           const backtest::System& system,
                           std::size_t index) noexcept -> const backtest::Strategy*
  {
    return self.store_.get_strategy_if_present(system.strategy_handle(index));
  }

  void reset_strategy(this ApplicationState& self,
                      backtest::StrategyStoreHandle handle)
  {
    for(const auto system_handle : self.document_state_.system_handles()) {
      const auto* system = self.store_.get_system_if_present(system_handle);
      if(!system) continue;
      for(auto index = std::size_t{}; index < system->strategy_count(); ++index) {
        if(system->strategy_handle(index) == handle) {
          self.reset_system(system_handle);
          break;
        }
      }
    }
  }

  void reset_strategies_referencing_model(this ApplicationState& self,
                                          backtest::ModelStoreHandle handle)
  {
    for(const auto strategy_handle : self.document_state_.strategy_handles()) {
      const auto* strategy = self.store_.get_strategy_if_present(strategy_handle);
      if(strategy && strategy->references_model(handle)) self.reset_strategy(strategy_handle);
    }
  }

  void sync_strategies_referencing_model(this ApplicationState& self,
                                         backtest::ModelStoreHandle handle)
  {
    const auto* model = self.store_.get_model_if_present(handle);
    if(!model) {
      return;
    }

    for(const auto strategy_handle : self.document_state_.strategy_handles()) {
      auto* strategy = self.store_.get_strategy_if_present(strategy_handle);
      if(!strategy || !strategy->references_model(handle)) {
        continue;
      }

      auto synced_inputs = backtest::collect_model_inputs(*model);
      const auto& previous_inputs = strategy->inputs();
      for(auto index = std::size_t{}; index < synced_inputs.size() &&
                                      index < previous_inputs.size();
          ++index) {
        synced_inputs[index].value(previous_inputs[index].value());
      }
      strategy->inputs(std::move(synced_inputs));
    }
  }

  void reset_strategies_referencing_profile(this ApplicationState& self,
                                            backtest::ProfileStoreHandle handle)
  {
    for(const auto strategy_handle : self.document_state_.strategy_handles()) {
      const auto* strategy = self.store_.get_strategy_if_present(strategy_handle);
      if(strategy && strategy->references_profile(handle)) self.reset_strategy(strategy_handle);
    }
  }

  void reset_watchlist(this ApplicationState& self,
                       backtest::WatchlistStoreHandle handle)
  {
    for(const auto system_handle : self.document_state_.system_handles()) {
      const auto* configured_system =
        self.store_.get_system_if_present(system_handle);
      if(configured_system && configured_system->watchlist_handle() == handle) {
         self.reset_system(system_handle);
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
