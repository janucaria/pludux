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

export module pludux.apps.backtest:state_arena;

import pludux.backtest;

export namespace pludux::apps {

class StateArena {
public:
  StateArena() = default;

  StateArena(std::string imgui_ini_settings,
             backtest::Store<backtest::Backtest> backtest_store,
             backtest::Store<backtest::Asset> asset_store,
             backtest::Store<backtest::Strategy> strategy_store,
             backtest::Store<backtest::Market> market_store,
             backtest::Store<backtest::Broker> broker_store,
             backtest::Store<backtest::Profile> profile_store,
             backtest::Store<std::vector<backtest::BacktestSummary>,
                             backtest::Backtest> backtest_summaries_store,
             backtest::Store<SeriesResultsCollector, backtest::Backtest>
              series_results_store)
  : imgui_ini_settings_{std::move(imgui_ini_settings)}
  , backtest_store_{std::move(backtest_store)}
  , asset_store_{std::move(asset_store)}
  , strategy_store_{std::move(strategy_store)}
  , market_store_{std::move(market_store)}
  , broker_store_{std::move(broker_store)}
  , profile_store_{std::move(profile_store)}
  , backtest_summaries_store_{std::move(backtest_summaries_store)}
  , series_results_store_{std::move(series_results_store)}
  {
  }

  auto imgui_ini_settings(this const StateArena& self) noexcept
   -> const std::string&
  {
    return self.imgui_ini_settings_;
  }

  void imgui_ini_settings(this StateArena& self, std::string settings) noexcept
  {
    self.imgui_ini_settings_ = std::move(settings);
  }

  auto backtest_store(this const StateArena& self) noexcept
   -> const backtest::Store<backtest::Backtest>&
  {
    return self.backtest_store_;
  }

  auto add_backtest(this StateArena& self, backtest::Backtest backtest)
   -> std::optional<backtest::StoreHandle<backtest::Backtest>>
  {
    return self.backtest_store_.add(std::move(backtest));
  }

  auto get_backtest(this const StateArena& self,
                    backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> const backtest::Backtest&
  {
    return self.backtest_store_.get(handle);
  }

  auto get_backtest(this StateArena& self,
                    backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> backtest::Backtest&
  {
    return self.backtest_store_.get(handle);
  }

  auto get_backtest_if_present(
   this const StateArena& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> const backtest::Backtest*
  {
    return self.backtest_store_.get_if_present(handle);
  }

  auto get_backtest_if_present(
   this StateArena& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> backtest::Backtest*
  {
    return self.backtest_store_.get_if_present(handle);
  }

  auto update_backtest(this StateArena& self,
                       backtest::StoreHandle<backtest::Backtest> handle,
                       backtest::Backtest backtest) -> bool
  {
    return self.backtest_store_.update(handle, std::move(backtest));
  }

  auto remove_backtest(this StateArena& self,
                       backtest::StoreHandle<backtest::Backtest> handle) -> bool
  {
    return self.backtest_store_.remove(handle);
  }

  auto asset_store(this const StateArena& self) noexcept
   -> const backtest::Store<backtest::Asset>&
  {
    return self.asset_store_;
  }

  auto add_asset(this StateArena& self, backtest::Asset asset)
   -> std::optional<backtest::StoreHandle<backtest::Asset>>
  {
    return self.asset_store_.add(std::move(asset));
  }

  auto get_asset(this const StateArena& self,
                 backtest::StoreHandle<backtest::Asset> handle) noexcept
   -> const backtest::Asset&
  {
    return self.asset_store_.get(handle);
  }

  auto get_asset(this StateArena& self,
                 backtest::StoreHandle<backtest::Asset> handle) noexcept
   -> backtest::Asset&
  {
    return self.asset_store_.get(handle);
  }

  auto
  get_asset_if_present(this const StateArena& self,
                       backtest::StoreHandle<backtest::Asset> handle) noexcept
   -> const backtest::Asset*
  {
    return self.asset_store_.get_if_present(handle);
  }

  auto
  get_asset_if_present(this StateArena& self,
                       backtest::StoreHandle<backtest::Asset> handle) noexcept
   -> backtest::Asset*
  {
    return self.asset_store_.get_if_present(handle);
  }

  auto update_asset(this StateArena& self,
                    backtest::StoreHandle<backtest::Asset> handle,
                    backtest::Asset asset) -> bool
  {
    return self.asset_store_.update(handle, std::move(asset));
  }

  auto remove_asset(this StateArena& self,
                    backtest::StoreHandle<backtest::Asset> handle) -> bool
  {
    return self.asset_store_.remove(handle);
  }

  auto strategy_store(this const StateArena& self) noexcept
   -> const backtest::Store<backtest::Strategy>&
  {
    return self.strategy_store_;
  }

  auto add_strategy(this StateArena& self, backtest::Strategy strategy)
   -> std::optional<backtest::StoreHandle<backtest::Strategy>>
  {
    return self.strategy_store_.add(std::move(strategy));
  }

  auto get_strategy(this const StateArena& self,
                    backtest::StoreHandle<backtest::Strategy> handle) noexcept
   -> const backtest::Strategy&
  {
    return self.strategy_store_.get(handle);
  }

  auto get_strategy(this StateArena& self,
                    backtest::StoreHandle<backtest::Strategy> handle) noexcept
   -> backtest::Strategy&
  {
    return self.strategy_store_.get(handle);
  }

  auto get_strategy_if_present(
   this const StateArena& self,
   backtest::StoreHandle<backtest::Strategy> handle) noexcept
   -> const backtest::Strategy*
  {
    return self.strategy_store_.get_if_present(handle);
  }

  auto get_strategy_if_present(
   this StateArena& self,
   backtest::StoreHandle<backtest::Strategy> handle) noexcept
   -> backtest::Strategy*
  {
    return self.strategy_store_.get_if_present(handle);
  }

  auto update_strategy(this StateArena& self,
                       backtest::StoreHandle<backtest::Strategy> handle,
                       backtest::Strategy strategy) -> bool
  {
    return self.strategy_store_.update(handle, std::move(strategy));
  }

  auto remove_strategy(this StateArena& self,
                       backtest::StoreHandle<backtest::Strategy> handle) -> bool
  {
    return self.strategy_store_.remove(handle);
  }

  auto market_store(this const StateArena& self) noexcept
   -> const backtest::Store<backtest::Market>&
  {
    return self.market_store_;
  }

  auto add_market(this StateArena& self, backtest::Market market)
   -> std::optional<backtest::StoreHandle<backtest::Market>>
  {
    return self.market_store_.add(std::move(market));
  }

  auto get_market(this const StateArena& self,
                  backtest::StoreHandle<backtest::Market> handle) noexcept
   -> const backtest::Market&
  {
    return self.market_store_.get(handle);
  }

  auto get_market(this StateArena& self,
                  backtest::StoreHandle<backtest::Market> handle) noexcept
   -> backtest::Market&
  {
    return self.market_store_.get(handle);
  }

  auto
  get_market_if_present(this const StateArena& self,
                        backtest::StoreHandle<backtest::Market> handle) noexcept
   -> const backtest::Market*
  {
    return self.market_store_.get_if_present(handle);
  }

  auto
  get_market_if_present(this StateArena& self,
                        backtest::StoreHandle<backtest::Market> handle) noexcept
   -> backtest::Market*
  {
    return self.market_store_.get_if_present(handle);
  }

  auto update_market(this StateArena& self,
                     backtest::StoreHandle<backtest::Market> handle,
                     backtest::Market market) -> bool
  {
    return self.market_store_.update(handle, std::move(market));
  }

  auto remove_market(this StateArena& self,
                     backtest::StoreHandle<backtest::Market> handle) -> bool
  {
    return self.market_store_.remove(handle);
  }

  auto broker_store(this const StateArena& self) noexcept
   -> const backtest::Store<backtest::Broker>&
  {
    return self.broker_store_;
  }

  auto add_broker(this StateArena& self, backtest::Broker broker)
   -> std::optional<backtest::StoreHandle<backtest::Broker>>
  {
    return self.broker_store_.add(std::move(broker));
  }

  auto get_broker(this const StateArena& self,
                  backtest::StoreHandle<backtest::Broker> handle) noexcept
   -> const backtest::Broker&
  {
    return self.broker_store_.get(handle);
  }

  auto get_broker(this StateArena& self,
                  backtest::StoreHandle<backtest::Broker> handle) noexcept
   -> backtest::Broker&
  {
    return self.broker_store_.get(handle);
  }

  auto
  get_broker_if_present(this const StateArena& self,
                        backtest::StoreHandle<backtest::Broker> handle) noexcept
   -> const backtest::Broker*
  {
    return self.broker_store_.get_if_present(handle);
  }

  auto
  get_broker_if_present(this StateArena& self,
                        backtest::StoreHandle<backtest::Broker> handle) noexcept
   -> backtest::Broker*
  {
    return self.broker_store_.get_if_present(handle);
  }

  auto update_broker(this StateArena& self,
                     backtest::StoreHandle<backtest::Broker> handle,
                     backtest::Broker broker) -> bool
  {
    return self.broker_store_.update(handle, std::move(broker));
  }

  auto remove_broker(this StateArena& self,
                     backtest::StoreHandle<backtest::Broker> handle) -> bool
  {
    return self.broker_store_.remove(handle);
  }

  auto profile_store(this const StateArena& self) noexcept
   -> const backtest::Store<backtest::Profile>&
  {
    return self.profile_store_;
  }

  auto add_profile(this StateArena& self, backtest::Profile profile)
   -> std::optional<backtest::StoreHandle<backtest::Profile>>
  {
    return self.profile_store_.add(std::move(profile));
  }

  auto get_profile(this const StateArena& self,
                   backtest::StoreHandle<backtest::Profile> handle) noexcept
   -> const backtest::Profile&
  {
    return self.profile_store_.get(handle);
  }

  auto get_profile(this StateArena& self,
                   backtest::StoreHandle<backtest::Profile> handle) noexcept
   -> backtest::Profile&
  {
    return self.profile_store_.get(handle);
  }

  auto get_profile_if_present(
   this const StateArena& self,
   backtest::StoreHandle<backtest::Profile> handle) noexcept
   -> const backtest::Profile*
  {
    return self.profile_store_.get_if_present(handle);
  }

  auto get_profile_if_present(
   this StateArena& self,
   backtest::StoreHandle<backtest::Profile> handle) noexcept
   -> backtest::Profile*
  {
    return self.profile_store_.get_if_present(handle);
  }

  auto update_profile(this StateArena& self,
                      backtest::StoreHandle<backtest::Profile> handle,
                      backtest::Profile profile) -> bool
  {
    return self.profile_store_.update(handle, std::move(profile));
  }

  auto remove_profile(this StateArena& self,
                      backtest::StoreHandle<backtest::Profile> handle) -> bool
  {
    return self.profile_store_.remove(handle);
  }

  auto backtest_summaries_store(this const StateArena& self) noexcept -> const
   backtest::Store<std::vector<backtest::BacktestSummary>, backtest::Backtest>&
  {
    return self.backtest_summaries_store_;
  }

  auto add_backtest_summaries(this StateArena& self,
                              backtest::StoreHandle<backtest::Backtest> handle,
                              std::vector<backtest::BacktestSummary> summaries)
   -> bool
  {
    return self.backtest_summaries_store_.add(handle, std::move(summaries));
  }

  auto get_backtest_summaries(
   this const StateArena& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> const std::vector<backtest::BacktestSummary>&
  {
    return self.backtest_summaries_store_.get(handle);
  }

  auto get_backtest_summaries(
   this StateArena& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> std::vector<backtest::BacktestSummary>&
  {
    return self.backtest_summaries_store_.get(handle);
  }

  auto get_backtest_summaries_if_present(
   this const StateArena& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> const std::vector<backtest::BacktestSummary>*
  {
    return self.backtest_summaries_store_.get_if_present(handle);
  }

  auto get_backtest_summaries_if_present(
   this StateArena& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> std::vector<backtest::BacktestSummary>*
  {
    return self.backtest_summaries_store_.get_if_present(handle);
  }

  auto
  update_backtest_summaries(this StateArena& self,
                            backtest::StoreHandle<backtest::Backtest> handle,
                            std::vector<backtest::BacktestSummary> summaries)
   -> bool
  {
    return self.backtest_summaries_store_.update(handle, std::move(summaries));
  }

  auto
  remove_backtest_summaries(this StateArena& self,
                            backtest::StoreHandle<backtest::Backtest> handle)
   -> bool
  {
    return self.backtest_summaries_store_.remove(handle);
  }

  auto series_results_store(this const StateArena& self) noexcept
   -> const backtest::Store<SeriesResultsCollector, backtest::Backtest>&
  {
    return self.series_results_store_;
  }

  auto add_series_results(this StateArena& self,
                          backtest::StoreHandle<backtest::Backtest> handle,
                          SeriesResultsCollector series_results_collector)
   -> bool
  {
    return self.series_results_store_.add(handle,
                                          std::move(series_results_collector));
  }

  auto
  get_series_results(this const StateArena& self,
                     backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> const SeriesResultsCollector&
  {
    return self.series_results_store_.get(handle);
  }

  auto
  get_series_results(this StateArena& self,
                     backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> SeriesResultsCollector&
  {
    return self.series_results_store_.get(handle);
  }

  auto get_series_results_if_present(
   this const StateArena& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> const SeriesResultsCollector*
  {
    return self.series_results_store_.get_if_present(handle);
  }

  auto get_series_results_if_present(
   this StateArena& self,
   backtest::StoreHandle<backtest::Backtest> handle) noexcept
   -> SeriesResultsCollector*
  {
    return self.series_results_store_.get_if_present(handle);
  }

  auto update_series_results(this StateArena& self,
                             backtest::StoreHandle<backtest::Backtest> handle,
                             SeriesResultsCollector series_results_collector)
   -> bool
  {
    return self.series_results_store_.update(
     handle, std::move(series_results_collector));
  }

  auto remove_series_results(this StateArena& self,
                             backtest::StoreHandle<backtest::Backtest> handle)
   -> bool
  {
    return self.series_results_store_.remove(handle);
  }

private:
  std::string imgui_ini_settings_;

  backtest::Store<backtest::Backtest> backtest_store_;
  backtest::Store<backtest::Asset> asset_store_;
  backtest::Store<backtest::Strategy> strategy_store_;
  backtest::Store<backtest::Market> market_store_;
  backtest::Store<backtest::Broker> broker_store_;
  backtest::Store<backtest::Profile> profile_store_;

  backtest::Store<std::vector<backtest::BacktestSummary>, backtest::Backtest>
   backtest_summaries_store_;

  backtest::Store<SeriesResultsCollector, backtest::Backtest>
   series_results_store_;
};

} // namespace pludux::apps
