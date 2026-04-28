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

export module pludux.apps.backtest:application_state;

import pludux.backtest;

import :ui_state;

export namespace pludux::apps {

class ApplicationState {
public:
  ApplicationState() = default;

  ApplicationState(backtest::Store store, UiState ui_state)
  : store_{std::move(store)}
  , ui_state_{std::move(ui_state)}
  {
  }

  auto store(this const ApplicationState& self) noexcept
   -> const backtest::Store&
  {
    return self.store_;
  }

  auto ui_state(this const ApplicationState& self) noexcept -> const UiState&
  {
    return self.ui_state_;
  }

  auto imgui_ini_settings(this const ApplicationState& self) noexcept
   -> const std::string&
  {
    return self.ui_state_.imgui_ini_settings();
  }

  void imgui_ini_settings(this ApplicationState& self,
                          std::string settings) noexcept
  {
    self.ui_state_.imgui_ini_settings(std::move(settings));
  }

  void select_backtest(this ApplicationState& self,
                       backtest::BacktestStoreHandle backtest_handle)
  {
    self.ui_state_.selected_backtest_handle(backtest_handle);
  }

  auto selected_backtest_handle(this const ApplicationState& self) noexcept
   -> backtest::BacktestStoreHandle
  {
    return self.ui_state_.selected_backtest_handle();
  }

  auto selected_backtest(this const ApplicationState& self) noexcept
   -> const backtest::Backtest&
  {
    const auto selected_backtest_handle =
     self.ui_state_.selected_backtest_handle();
    return self.store_.get_backtest(selected_backtest_handle);
  }

  auto selected_backtest(this ApplicationState& self) noexcept
   -> backtest::Backtest&
  {
    const auto selected_backtest_handle =
     self.ui_state_.selected_backtest_handle();
    return self.store_.get_backtest(selected_backtest_handle);
  }

  auto selected_backtest_if_present(this ApplicationState& self) noexcept
   -> backtest::Backtest*
  {
    const auto selected_backtest_handle =
     self.ui_state_.selected_backtest_handle();
    return self.store_.get_backtest_if_present(selected_backtest_handle);
  }

  auto selected_backtest_if_present(this const ApplicationState& self) noexcept
   -> const backtest::Backtest*
  {
    const auto selected_backtest_handle =
     self.ui_state_.selected_backtest_handle();
    return self.store_.get_backtest_if_present(selected_backtest_handle);
  }

  auto get_backtest_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::BacktestStoreHandle>&
  {
    return self.ui_state_.backtest_handles();
  }

  auto get_backtest_handles(this ApplicationState& self) noexcept
   -> std::vector<backtest::BacktestStoreHandle>&
  {
    return self.ui_state_.backtest_handles();
  }

  auto add_backtest(this ApplicationState& self, backtest::Backtest backtest)
   -> std::optional<backtest::BacktestStoreHandle>
  {
    const auto handle_opt = self.store_.add_backtest(std::move(backtest));
    if(handle_opt) {
      self.ui_state_.add_backtest_handle(*handle_opt);
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
      self.ui_state_.remove_backtest_handle(handle);

      if(self.ui_state_.selected_backtest_handle() == handle) {
        self.ui_state_.selected_backtest_handle({});
      }

      self.store_.remove_backtest(handle);
      self.store_.remove_backtest_summaries(handle);
      self.store_.remove_series_results(handle);

      return true;
    }
    return false;
  }

  auto get_asset_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::AssetStoreHandle>&
  {
    return self.ui_state_.asset_handles();
  }

  void add_asset(this ApplicationState& self, backtest::Asset asset)
  {
    const auto handle_opt = self.store_.add_asset(std::move(asset));
    if(handle_opt) {
      self.ui_state_.add_asset_handle(*handle_opt);
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
        const auto& backtest_handles = self.ui_state_.backtest_handles();
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
      self.ui_state_.remove_asset_handle(handle);

      const auto& backtest_handles = self.ui_state_.backtest_handles();
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
    return self.ui_state_.strategy_handles();
  }

  void add_strategy(this ApplicationState& self, backtest::Strategy strategy)
  {
    const auto handle_opt = self.store_.add_strategy(std::move(strategy));
    if(handle_opt) {
      self.ui_state_.add_strategy_handle(*handle_opt);
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
        const auto& backtest_handles = self.ui_state_.backtest_handles();
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
      self.ui_state_.remove_strategy_handle(handle);

      const auto& backtest_handles = self.ui_state_.backtest_handles();
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
    return self.ui_state_.market_handles();
  }

  void add_market(this ApplicationState& self, backtest::Market market)
  {
    const auto handle_opt = self.store_.add_market(std::move(market));
    if(handle_opt) {
      self.ui_state_.add_market_handle(*handle_opt);
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

    const auto reset_backtests = !market->equivalent_rules(edit_market);

    if(self.store_.update_market(handle, std::move(edit_market))) {
      if(reset_backtests) {
        const auto& backtest_handles = self.ui_state_.backtest_handles();
        for(const auto& backtest_handle : backtest_handles) {
          auto backtest_ptr =
           self.store_.get_backtest_if_present(backtest_handle);
          if(backtest_ptr && backtest_ptr->market_handle() == handle) {
            self.reset_backtest(backtest_handle);
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
      self.ui_state_.remove_market_handle(handle);

      const auto& backtest_handles = self.ui_state_.backtest_handles();
      for(const auto& backtest_handle : backtest_handles) {
        const auto backtest_ptr =
         self.store_.get_backtest_if_present(backtest_handle);
        if(backtest_ptr && backtest_ptr->market_handle() == handle) {
          self.reset_backtest(backtest_handle);
        }
      }

      return true;
    }

    return false;
  }

  auto get_broker_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::BrokerStoreHandle>&
  {
    return self.ui_state_.broker_handles();
  }

  void add_broker(this ApplicationState& self, backtest::Broker broker)
  {
    const auto handle_opt = self.store_.add_broker(std::move(broker));
    if(handle_opt) {
      self.ui_state_.add_broker_handle(*handle_opt);
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

    const auto reset_backtests = !broker->equivalent_rules(edit_broker);

    if(self.store_.update_broker(handle, std::move(edit_broker))) {
      if(reset_backtests) {
        const auto& backtest_handles = self.ui_state_.backtest_handles();
        for(const auto& backtest_handle : backtest_handles) {
          auto backtest_ptr =
           self.store_.get_backtest_if_present(backtest_handle);
          if(backtest_ptr && backtest_ptr->broker_handle() == handle) {
            self.reset_backtest(backtest_handle);
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
      self.ui_state_.remove_broker_handle(handle);

      const auto& backtest_handles = self.ui_state_.backtest_handles();
      for(const auto& backtest_handle : backtest_handles) {
        const auto backtest_ptr =
         self.store_.get_backtest_if_present(backtest_handle);
        if(backtest_ptr && backtest_ptr->broker_handle() == handle) {
          self.reset_backtest(backtest_handle);
        }
      }

      return true;
    }

    return false;
  }

  auto get_profile_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::ProfileStoreHandle>&
  {
    return self.ui_state_.profile_handles();
  }

  void add_profile(this ApplicationState& self, backtest::Profile profile)
  {
    const auto handle_opt = self.store_.add_profile(std::move(profile));
    if(handle_opt) {
      self.ui_state_.add_profile_handle(*handle_opt);
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
        const auto& backtest_handles = self.ui_state_.backtest_handles();
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
      self.ui_state_.remove_profile_handle(handle);

      const auto& backtest_handles = self.ui_state_.backtest_handles();
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

  auto add_backtest_summaries(this ApplicationState& self,
                              backtest::BacktestStoreHandle handle,
                              std::vector<backtest::BacktestSummary> summaries)
   -> bool
  {
    return self.store_.add_backtest_summaries(handle, std::move(summaries));
  }

  auto get_backtest_summaries(this const ApplicationState& self,
                              backtest::BacktestStoreHandle handle) noexcept
   -> const std::vector<backtest::BacktestSummary>&
  {
    return self.store_.get_backtest_summaries(handle);
  }

  auto get_backtest_summaries(this ApplicationState& self,
                              backtest::BacktestStoreHandle handle) noexcept
   -> std::vector<backtest::BacktestSummary>&
  {
    return self.store_.get_backtest_summaries(handle);
  }

  auto get_backtest_summaries_if_present(
   this const ApplicationState& self,
   backtest::BacktestStoreHandle handle) noexcept
   -> const std::vector<backtest::BacktestSummary>*
  {
    return self.store_.get_backtest_summaries_if_present(handle);
  }

  auto get_backtest_summaries_if_present(
   this ApplicationState& self, backtest::BacktestStoreHandle handle) noexcept
   -> std::vector<backtest::BacktestSummary>*
  {
    return self.store_.get_backtest_summaries_if_present(handle);
  }

  auto
  update_backtest_summaries(this ApplicationState& self,
                            backtest::BacktestStoreHandle handle,
                            std::vector<backtest::BacktestSummary> summaries)
   -> bool
  {
    return self.store_.update_backtest_summaries(handle, std::move(summaries));
  }

  auto remove_backtest_summaries(this ApplicationState& self,
                                 backtest::BacktestStoreHandle handle) -> bool
  {
    return self.store_.remove_backtest_summaries(handle);
  }

  auto add_series_results(this ApplicationState& self,
                          backtest::BacktestStoreHandle handle,
                          SeriesResultsCollector series_results_collector)
   -> bool
  {
    return self.store_.add_series_results(handle,
                                          std::move(series_results_collector));
  }

  auto get_series_results(this const ApplicationState& self,
                          backtest::BacktestStoreHandle handle) noexcept
   -> const SeriesResultsCollector&
  {
    return self.store_.get_series_results(handle);
  }

  auto get_series_results(this ApplicationState& self,
                          backtest::BacktestStoreHandle handle) noexcept
   -> SeriesResultsCollector&
  {
    return self.store_.get_series_results(handle);
  }

  auto
  get_series_results_if_present(this const ApplicationState& self,
                                backtest::BacktestStoreHandle handle) noexcept
   -> const SeriesResultsCollector*
  {
    return self.store_.get_series_results_if_present(handle);
  }

  auto
  get_series_results_if_present(this ApplicationState& self,
                                backtest::BacktestStoreHandle handle) noexcept
   -> SeriesResultsCollector*
  {
    return self.store_.get_series_results_if_present(handle);
  }

  auto update_series_results(this ApplicationState& self,
                             backtest::BacktestStoreHandle handle,
                             SeriesResultsCollector series_results_collector)
   -> bool
  {
    return self.store_.update_series_results(
     handle, std::move(series_results_collector));
  }

  auto remove_series_results(this ApplicationState& self,
                             backtest::BacktestStoreHandle handle) -> bool
  {
    return self.store_.remove_series_results(handle);
  }

  void reset_all_backtests(this ApplicationState& self)
  {
    const auto& backtest_handles = self.ui_state_.backtest_handles();
    for(const auto& backtest_handle : backtest_handles) {
      self.reset_backtest(backtest_handle);
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
      const auto broker_handle = ready_backtest.broker_handle();
      const auto broker_ptr = self.get_broker_if_present(broker_handle);

      if(!broker_ptr) {
        return false;
      }
    }
    {
      const auto market_handle = ready_backtest.market_handle();
      const auto market_ptr = self.get_market_if_present(market_handle);

      if(!market_ptr) {
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

private:
  UiState ui_state_{};
  backtest::Store store_{};

  void reset_backtest(this ApplicationState& self,
                      backtest::BacktestStoreHandle handle)
  {
    auto& backtest = self.store_.get_backtest(handle);
    backtest.is_failed(false);

    const auto summaries = self.store_.update_backtest_summaries(handle, {});
    const auto series_resulkt = self.store_.update_series_results(handle, {});

    if(summaries || series_resulkt) {
      // TODO: Handle error case where summaries or series results fail to
      // update after backtest reset
    }
  }
};

} // namespace pludux::apps
