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

import :state_arena;
import :state_descriptor;

export namespace pludux::apps {

class ApplicationState {
public:
  ApplicationState() = default;

  ApplicationState(StateDescriptor state_descriptor, StateArena state_arena)
  : state_descriptor_{std::move(state_descriptor)}
  , state_arena_{std::move(state_arena)}
  {
  }

  auto state_descriptor(this const ApplicationState& self) noexcept
   -> const StateDescriptor&
  {
    return self.state_descriptor_;
  }

  auto state_arena(this const ApplicationState& self) noexcept
   -> const StateArena&
  {
    return self.state_arena_;
  }

  auto imgui_ini_settings(this const ApplicationState& self) noexcept
   -> const std::string&
  {
    return self.state_arena_.imgui_ini_settings();
  }

  void imgui_ini_settings(this ApplicationState& self,
                          std::string settings) noexcept
  {
    self.state_arena_.imgui_ini_settings(std::move(settings));
  }

  void
  select_backtest(this ApplicationState& self,
                  backtest::StoreHandle<backtest::Backtest> backtest_handle)
  {
    self.state_descriptor_.selected_backtest_handle(backtest_handle);
  }

  auto selected_backtest_handle(this const ApplicationState& self) noexcept
   -> backtest::StoreHandle<backtest::Backtest>
  {
    return self.state_descriptor_.selected_backtest_handle();
  }

  auto selected_backtest(this const ApplicationState& self) noexcept
   -> const backtest::Backtest&
  {
    const auto selected_backtest_handle =
     self.state_descriptor_.selected_backtest_handle();

    return self.state_arena_.get_backtest(selected_backtest_handle);
  }

  auto selected_backtest(this ApplicationState& self) noexcept
   -> backtest::Backtest&
  {
    const auto selected_backtest_handle =
     self.state_descriptor_.selected_backtest_handle();

    return self.state_arena_.get_backtest(selected_backtest_handle);
  }

  auto selected_backtest_if_present(this ApplicationState& self) noexcept
   -> backtest::Backtest*
  {
    const auto selected_backtest_handle =
     self.state_descriptor_.selected_backtest_handle();

    return self.state_arena_.get_backtest_if_present(selected_backtest_handle);
  }

  auto selected_backtest_if_present(this const ApplicationState& self) noexcept
   -> const backtest::Backtest*
  {
    const auto selected_backtest_handle =
     self.state_descriptor_.selected_backtest_handle();

    return self.state_arena_.get_backtest_if_present(selected_backtest_handle);
  }

  auto get_backtest_store(this const ApplicationState& self) noexcept
   -> const backtest::Store<backtest::Backtest>&
  {
    return self.state_arena_.backtest_store();
  }

  auto get_backtest_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::StoreHandle<backtest::Backtest>>&
  {
    return self.state_descriptor_.backtest_handles();
  }

  auto get_backtest_handles(this ApplicationState& self) noexcept
   -> std::vector<backtest::StoreHandle<backtest::Backtest>>&
  {
    return self.state_descriptor_.backtest_handles();
  }

  auto add_backtest(this ApplicationState& self, backtest::Backtest backtest)
   -> std::optional<backtest::StoreHandle<backtest::Backtest>>
  {
    const auto handle_opt = self.state_arena_.add_backtest(std::move(backtest));
    if(handle_opt) {
      self.state_descriptor_.add_backtest_handle(*handle_opt);
    }
    return handle_opt;
  }

  auto get_backtest(this const ApplicationState& self,
                    backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> const backtest::Backtest&
  {
    return self.state_arena_.get_backtest(handle);
  }

  auto get_backtest(this ApplicationState& self,
                    backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> backtest::Backtest&
  {
    return self.state_arena_.get_backtest(handle);
  }

  auto get_backtest_if_present(
   this const ApplicationState& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> const backtest::Backtest*
  {
    return self.state_arena_.get_backtest_if_present(handle);
  }

  auto get_backtest_if_present(
   this ApplicationState& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> backtest::Backtest*
  {
    return self.state_arena_.get_backtest_if_present(handle);
  }

  auto update_backtest(this ApplicationState& self,
                       backtest::StoreHandle<backtest::Backtest> handle,
                       backtest::Backtest backtest) -> bool
  {
    if(self.state_arena_.update_backtest(handle, std::move(backtest))) {
      self.reset_backtest(handle);
      return true;
    }

    return false;
  }

  auto remove_backtest(this ApplicationState& self,
                       backtest::StoreHandle<backtest::Backtest> handle) -> bool
  {
    if(self.state_arena_.remove_backtest(handle)) {
      self.state_descriptor_.remove_backtest_handle(handle);

      if(self.state_descriptor_.selected_backtest_handle() == handle) {
        self.state_descriptor_.selected_backtest_handle({});
      }

      self.state_arena_.remove_backtest(handle);
      self.state_arena_.remove_backtest_summaries(handle);
      self.state_arena_.remove_series_results(handle);

      return true;
    }
    return false;
  }

  auto get_asset_store(this const ApplicationState& self) noexcept
   -> const backtest::Store<backtest::Asset>&
  {
    return self.state_arena_.asset_store();
  }

  auto get_asset_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::StoreHandle<backtest::Asset>>&
  {
    return self.state_descriptor_.asset_handles();
  }

  void add_asset(this ApplicationState& self, backtest::Asset asset)
  {
    const auto handle_opt = self.state_arena_.add_asset(std::move(asset));
    if(handle_opt) {
      self.state_descriptor_.add_asset_handle(*handle_opt);
    }
  }

  auto get_asset(this const ApplicationState& self,
                 backtest::StoreHandle<backtest::Asset> handle) noexcept
   -> const backtest::Asset&
  {
    return self.state_arena_.get_asset(handle);
  }

  auto
  get_asset_if_present(this const ApplicationState& self,
                       backtest::StoreHandle<backtest::Asset> handle) noexcept
   -> const backtest::Asset*
  {
    return self.state_arena_.get_asset_if_present(handle);
  }

  auto update_asset(this ApplicationState& self,
                    backtest::StoreHandle<backtest::Asset> handle,
                    backtest::Asset edit_asset) -> bool
  {
    const auto& asset = self.get_asset_if_present(handle);
    if(!asset) {
      return false;
    }

    const auto reset_backtests = !asset->equivalent_rules(edit_asset);

    if(self.state_arena_.update_asset(handle, std::move(edit_asset))) {
      if(reset_backtests) {
        const auto& backtest_handles =
         self.state_descriptor_.backtest_handles();
        for(const auto& backtest_handle : backtest_handles) {
          auto backtest_ptr =
           self.state_arena_.get_backtest_if_present(backtest_handle);
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
                    backtest::StoreHandle<backtest::Asset> handle) -> bool
  {
    if(self.state_arena_.remove_asset(handle)) {
      self.state_descriptor_.remove_asset_handle(handle);

      const auto& backtest_handles = self.state_descriptor_.backtest_handles();
      for(const auto& backtest_handle : backtest_handles) {
        const auto backtest_ptr =
         self.state_arena_.get_backtest_if_present(backtest_handle);
        if(backtest_ptr && backtest_ptr->asset_handle() == handle) {
          self.reset_backtest(backtest_handle);
        }
      }

      return true;
    }

    return false;
  }

  auto get_strategy_store(this const ApplicationState& self) noexcept
   -> const backtest::Store<backtest::Strategy>&
  {
    return self.state_arena_.strategy_store();
  }

  auto get_strategy_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::StoreHandle<backtest::Strategy>>&
  {
    return self.state_descriptor_.strategy_handles();
  }

  void add_strategy(this ApplicationState& self, backtest::Strategy strategy)
  {
    const auto handle_opt = self.state_arena_.add_strategy(std::move(strategy));
    if(handle_opt) {
      self.state_descriptor_.add_strategy_handle(*handle_opt);
    }
  }

  auto get_strategy(this const ApplicationState& self,
                    backtest::StoreHandle<backtest::Strategy> handle) noexcept
   -> const backtest::Strategy&
  {
    return self.state_arena_.get_strategy(handle);
  }

  auto get_strategy_if_present(
   this const ApplicationState& self,
   backtest::StoreHandle<backtest::Strategy> handle) noexcept
   -> const backtest::Strategy*
  {
    return self.state_arena_.get_strategy_if_present(handle);
  }

  auto update_strategy(this ApplicationState& self,
                       backtest::StoreHandle<backtest::Strategy> handle,
                       backtest::Strategy edit_strategy) -> bool
  {
    const auto& strategy = self.get_strategy_if_present(handle);
    if(!strategy) {
      return false;
    }

    const auto reset_backtests = !strategy->equivalent_rules(edit_strategy);

    if(self.state_arena_.update_strategy(handle, std::move(edit_strategy))) {
      if(reset_backtests) {
        const auto& backtest_handles =
         self.state_descriptor_.backtest_handles();
        for(const auto& backtest_handle : backtest_handles) {
          auto backtest_ptr =
           self.state_arena_.get_backtest_if_present(backtest_handle);
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
                       backtest::StoreHandle<backtest::Strategy> handle) -> bool
  {
    if(self.state_arena_.remove_strategy(handle)) {
      self.state_descriptor_.remove_strategy_handle(handle);

      const auto& backtest_handles = self.state_descriptor_.backtest_handles();
      for(const auto& backtest_handle : backtest_handles) {
        const auto backtest_ptr =
         self.state_arena_.get_backtest_if_present(backtest_handle);
        if(backtest_ptr && backtest_ptr->strategy_handle() == handle) {
          self.reset_backtest(backtest_handle);
        }
      }

      return true;
    }

    return false;
  }

  auto get_market_store(this const ApplicationState& self) noexcept
   -> const backtest::Store<backtest::Market>&
  {
    return self.state_arena_.market_store();
  }

  auto get_market_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::StoreHandle<backtest::Market>>&
  {
    return self.state_descriptor_.market_handles();
  }

  void add_market(this ApplicationState& self, backtest::Market market)
  {
    const auto handle_opt = self.state_arena_.add_market(std::move(market));
    if(handle_opt) {
      self.state_descriptor_.add_market_handle(*handle_opt);
    }
  }

  auto get_market(this const ApplicationState& self,
                  backtest::StoreHandle<backtest::Market> handle) noexcept
   -> const backtest::Market&
  {
    return self.state_arena_.get_market(handle);
  }

  auto
  get_market_if_present(this const ApplicationState& self,
                        backtest::StoreHandle<backtest::Market> handle) noexcept
   -> const backtest::Market*
  {
    return self.state_arena_.get_market_if_present(handle);
  }

  auto update_market(this ApplicationState& self,
                     backtest::StoreHandle<backtest::Market> handle,
                     backtest::Market edit_market) -> bool
  {
    const auto& market = self.get_market_if_present(handle);
    if(!market) {
      return false;
    }

    const auto reset_backtests = !market->equivalent_rules(edit_market);

    if(self.state_arena_.update_market(handle, std::move(edit_market))) {
      if(reset_backtests) {
        const auto& backtest_handles =
         self.state_descriptor_.backtest_handles();
        for(const auto& backtest_handle : backtest_handles) {
          auto backtest_ptr =
           self.state_arena_.get_backtest_if_present(backtest_handle);
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
                     backtest::StoreHandle<backtest::Market> handle) -> bool
  {
    if(self.state_arena_.remove_market(handle)) {
      self.state_descriptor_.remove_market_handle(handle);

      const auto& backtest_handles = self.state_descriptor_.backtest_handles();
      for(const auto& backtest_handle : backtest_handles) {
        const auto backtest_ptr =
         self.state_arena_.get_backtest_if_present(backtest_handle);
        if(backtest_ptr && backtest_ptr->market_handle() == handle) {
          self.reset_backtest(backtest_handle);
        }
      }

      return true;
    }

    return false;
  }

  auto get_broker_store(this const ApplicationState& self) noexcept
   -> const backtest::Store<backtest::Broker>&
  {
    return self.state_arena_.broker_store();
  }

  auto get_broker_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::StoreHandle<backtest::Broker>>&
  {
    return self.state_descriptor_.broker_handles();
  }

  void add_broker(this ApplicationState& self, backtest::Broker broker)
  {
    const auto handle_opt = self.state_arena_.add_broker(std::move(broker));
    if(handle_opt) {
      self.state_descriptor_.add_broker_handle(*handle_opt);
    }
  }

  auto get_broker(this const ApplicationState& self,
                  backtest::StoreHandle<backtest::Broker> handle) noexcept
   -> const backtest::Broker&
  {
    return self.state_arena_.get_broker(handle);
  }

  auto
  get_broker_if_present(this const ApplicationState& self,
                        backtest::StoreHandle<backtest::Broker> handle) noexcept
   -> const backtest::Broker*
  {
    return self.state_arena_.get_broker_if_present(handle);
  }

  auto update_broker(this ApplicationState& self,
                     backtest::StoreHandle<backtest::Broker> handle,
                     backtest::Broker edit_broker) -> bool
  {
    const auto& broker = self.get_broker_if_present(handle);
    if(!broker) {
      return false;
    }

    const auto reset_backtests = !broker->equivalent_rules(edit_broker);

    if(self.state_arena_.update_broker(handle, std::move(edit_broker))) {
      if(reset_backtests) {
        const auto& backtest_handles =
         self.state_descriptor_.backtest_handles();
        for(const auto& backtest_handle : backtest_handles) {
          auto backtest_ptr =
           self.state_arena_.get_backtest_if_present(backtest_handle);
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
                     backtest::StoreHandle<backtest::Broker> handle) -> bool
  {
    if(self.state_arena_.remove_broker(handle)) {
      self.state_descriptor_.remove_broker_handle(handle);

      const auto& backtest_handles = self.state_descriptor_.backtest_handles();
      for(const auto& backtest_handle : backtest_handles) {
        const auto backtest_ptr =
         self.state_arena_.get_backtest_if_present(backtest_handle);
        if(backtest_ptr && backtest_ptr->broker_handle() == handle) {
          self.reset_backtest(backtest_handle);
        }
      }

      return true;
    }

    return false;
  }

  auto get_profile_store(this const ApplicationState& self) noexcept
   -> const backtest::Store<backtest::Profile>&
  {
    return self.state_arena_.profile_store();
  }

  auto get_profile_handles(this const ApplicationState& self) noexcept
   -> const std::vector<backtest::StoreHandle<backtest::Profile>>&
  {
    return self.state_descriptor_.profile_handles();
  }

  void add_profile(this ApplicationState& self, backtest::Profile profile)
  {
    const auto handle_opt = self.state_arena_.add_profile(std::move(profile));
    if(handle_opt) {
      self.state_descriptor_.add_profile_handle(*handle_opt);
    }
  }

  auto get_profile(this const ApplicationState& self,
                   backtest::StoreHandle<backtest::Profile> handle) noexcept
   -> const backtest::Profile&
  {
    return self.state_arena_.get_profile(handle);
  }

  auto get_profile_if_present(
   this const ApplicationState& self,
   backtest::StoreHandle<backtest::Profile> handle) noexcept
   -> const backtest::Profile*
  {
    return self.state_arena_.get_profile_if_present(handle);
  }

  auto update_profile(this ApplicationState& self,
                      backtest::StoreHandle<backtest::Profile> handle,
                      backtest::Profile edit_profile) -> bool
  {
    const auto& profile = self.get_profile_if_present(handle);
    if(!profile) {
      return false;
    }

    const auto reset_backtests = !profile->equivalent_rules(edit_profile);
    if(self.state_arena_.update_profile(handle, std::move(edit_profile))) {
      if(reset_backtests) {
        const auto& backtest_handles =
         self.state_descriptor_.backtest_handles();
        for(const auto& backtest_handle : backtest_handles) {
          auto backtest_ptr =
           self.state_arena_.get_backtest_if_present(backtest_handle);
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
                      backtest::StoreHandle<backtest::Profile> handle) -> bool
  {
    if(self.state_arena_.remove_profile(handle)) {
      self.state_descriptor_.remove_profile_handle(handle);

      const auto& backtest_handles = self.state_descriptor_.backtest_handles();
      for(const auto& backtest_handle : backtest_handles) {
        const auto backtest_ptr =
         self.state_arena_.get_backtest_if_present(backtest_handle);
        if(backtest_ptr && backtest_ptr->profile_handle() == handle) {
          self.reset_backtest(backtest_handle);
        }
      }

      return true;
    }

    return false;
  }

  auto get_backtest_summaries_store(this const ApplicationState& self) noexcept
   -> const
   backtest::Store<std::vector<backtest::BacktestSummary>, backtest::Backtest>&
  {
    return self.state_arena_.backtest_summaries_store();
  }

  auto add_backtest_summaries(this ApplicationState& self,
                              backtest::StoreHandle<backtest::Backtest> handle,
                              std::vector<backtest::BacktestSummary> summaries)
   -> bool
  {
    return self.state_arena_.add_backtest_summaries(handle,
                                                    std::move(summaries));
  }

  auto get_backtest_summaries(
   this const ApplicationState& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> const std::vector<backtest::BacktestSummary>&
  {
    return self.state_arena_.get_backtest_summaries(handle);
  }

  auto get_backtest_summaries(
   this ApplicationState& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> std::vector<backtest::BacktestSummary>&
  {
    return self.state_arena_.get_backtest_summaries(handle);
  }

  auto get_backtest_summaries_if_present(
   this const ApplicationState& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> const std::vector<backtest::BacktestSummary>*
  {
    return self.state_arena_.get_backtest_summaries_if_present(handle);
  }

  auto get_backtest_summaries_if_present(
   this ApplicationState& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> std::vector<backtest::BacktestSummary>*
  {
    return self.state_arena_.get_backtest_summaries_if_present(handle);
  }

  auto
  update_backtest_summaries(this ApplicationState& self,
                            backtest::StoreHandle<backtest::Backtest> handle,
                            std::vector<backtest::BacktestSummary> summaries)
   -> bool
  {
    return self.state_arena_.update_backtest_summaries(handle,
                                                       std::move(summaries));
  }

  auto
  remove_backtest_summaries(this ApplicationState& self,
                            backtest::StoreHandle<backtest::Backtest> handle)
   -> bool
  {
    return self.state_arena_.remove_backtest_summaries(handle);
  }

  auto series_results_store(this const ApplicationState& self) noexcept
   -> const backtest::Store<SeriesResultsCollector, backtest::Backtest>&
  {
    return self.state_arena_.series_results_store();
  }

  auto add_series_results(this ApplicationState& self,
                          backtest::StoreHandle<backtest::Backtest> handle,
                          SeriesResultsCollector series_results_collector)
   -> bool
  {
    return self.state_arena_.add_series_results(
     handle, std::move(series_results_collector));
  }

  auto
  get_series_results(this const ApplicationState& self,
                     backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> const SeriesResultsCollector&
  {
    return self.state_arena_.get_series_results(handle);
  }

  auto
  get_series_results(this ApplicationState& self,
                     backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> SeriesResultsCollector&
  {
    return self.state_arena_.get_series_results(handle);
  }

  auto get_series_results_if_present(
   this const ApplicationState& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> const SeriesResultsCollector*
  {
    return self.state_arena_.get_series_results_if_present(handle);
  }

  auto get_series_results_if_present(
   this ApplicationState& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> SeriesResultsCollector*
  {
    return self.state_arena_.get_series_results_if_present(handle);
  }

  auto update_series_results(this ApplicationState& self,
                             backtest::StoreHandle<backtest::Backtest> handle,
                             SeriesResultsCollector series_results_collector)
   -> bool
  {
    return self.state_arena_.update_series_results(
     handle, std::move(series_results_collector));
  }

  auto remove_series_results(this ApplicationState& self,
                             backtest::StoreHandle<backtest::Backtest> handle)
   -> bool
  {
    return self.state_arena_.remove_series_results(handle);
  }

  void reset_all_backtests(this ApplicationState& self)
  {
    const auto& backtest_handles = self.state_descriptor_.backtest_handles();
    for(const auto& backtest_handle : backtest_handles) {
      self.reset_backtest(backtest_handle);
    }
  }

private:
  StateDescriptor state_descriptor_{};
  StateArena state_arena_{};

  void reset_backtest(this ApplicationState& self,
                      backtest::StoreHandle<backtest::Backtest> handle)
  {
    auto& backtest = self.state_arena_.get_backtest(handle);
    backtest.is_failed(false);

    const auto summaries =
     self.state_arena_.update_backtest_summaries(handle, {});
    const auto series_resulkt =
     self.state_arena_.update_series_results(handle, {});

    if(summaries || series_resulkt) {
      // TODO: Handle error case where summaries or series results fail to
      // update after backtest reset
    }
  }
};

} // namespace pludux::apps
