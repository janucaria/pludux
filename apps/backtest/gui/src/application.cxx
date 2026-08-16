module;

#include <algorithm>
#include <chrono>
#include <format>
#include <fstream>
#include <functional>
#include <list>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <imgui.h>
#include <implot.h>
#include <jsoncons/json.hpp>

export module pludux.apps.backtest;

export import pludux.apps.backtest.application_state;
export import pludux.apps.backtest.state_diff;
export import :window_context;
export import pludux.apps.backtest.serialization;
export import :actions;
export import :backtest_execution_status;
export import pludux.apps.backtest.command_executor;
import :ui.theme;
import :windows;

export namespace std {

template<>
struct hash<pludux::backtest::PortfolioStoreHandle> {
  auto operator()(const pludux::backtest::PortfolioStoreHandle& handle)
   const noexcept -> size_t
  {
    const auto slot_hash = std::hash<std::size_t>{}(handle.slot_index());
    const auto generation_hash = std::hash<std::size_t>{}(handle.generation());

    return slot_hash ^ (generation_hash << 1);
  }
};

} // namespace std

export namespace pludux::apps {

class Application {
public:
  Application()
  : window_size_{0, 0}
  {
  }

  void on_before_main_loop(this Application& self)
  {
    auto& app_state = self.app_state_;

    ImPlot::GetStyle().UseISO8601 = true;
    ImPlot::GetStyle().UseLocalTime = true;
    ImPlot::GetStyle().Use24HourClock = true;

    if(app_state.get_market_handles().empty()) {
      auto default_market = backtest::Market{"Default"};
      app_state.add_market(std::move(default_market));
    }

    if(app_state.get_broker_handles().empty()) {
      auto default_broker = backtest::Broker{"Default"};
      app_state.add_broker(std::move(default_broker));
    }

    if(app_state.get_profile_handles().empty()) {
      auto default_profile =
       backtest::Profile{"Default",
                         backtest::PositionSizingNode{
                          backtest::RiskDistancePositionSizing{0.01}}};
      app_state.add_profile(std::move(default_profile));
    }

// run in debug mode and not in emscripten
#if !defined(__EMSCRIPTEN__) && !defined(NDEBUG) && 1

    {
      const auto csv_path =
       get_env_var("PLUDUX_BACKTEST_CSV_DATA_PATH_1").value_or("");

      if(!csv_path.empty()) {
        LoadAssetCsvAction{csv_path}(app_state);
      }
    }

    {
      const auto csv_path =
       get_env_var("PLUDUX_BACKTEST_CSV_DATA_PATH_2").value_or("");

      if(!csv_path.empty()) {
        LoadAssetCsvAction{csv_path}(app_state);
      }
    }

#endif

    self.sync_stored_strategy_inputs(app_state);
    for(const auto portfolio_handle : app_state.get_portfolio_handles()) {
      self.recreate_portfolio_runner(app_state, portfolio_handle);
    }
  }

  void on_after_main_loop(this Application& self)
  {
  }

  void set_window_size(this Application& self, int width, int height)
  {
    self.window_size_ =
     ImVec2{static_cast<float>(width), static_cast<float>(height)};
  }

  void on_update(this Application& self)
  {
    auto& app_state = self.app_state_;
    auto& store = app_state.store();
    auto& alert_messages = self.alert_messages_;

    auto& portfolio_handles = app_state.get_portfolio_handles();

    // sync backtest runners with backtest handles in the app state, and run the
    // backtests
    for(auto it = self.running_portfolios_.begin();
        it != self.running_portfolios_.end();) {
      const auto& portfolio_handle = it->first;

      if(std::ranges::find(portfolio_handles, portfolio_handle) ==
         portfolio_handles.end()) {
        self.portfolio_execution_statuses_.erase(portfolio_handle);
        it = self.running_portfolios_.erase(it);
      } else {
        ++it;
      }
    }

    if(!portfolio_handles.empty()) {
      // Advance as many union timestamps as the frame budget permits.
      // A PortfolioRunner timestamp is atomic, so check the deadline before
      // every one-timestamp batch rather than after a larger batch.
      const auto update_deadline =
       std::chrono::steady_clock::now() + std::chrono::milliseconds{1000 / 60};

      auto made_progress = true;
      while(made_progress && std::chrono::steady_clock::now() < update_deadline) {
        made_progress = false;
        for(const auto portfolio_handle : portfolio_handles) {
          if(std::chrono::steady_clock::now() >= update_deadline) {
            break;
          }

          auto& portfolio = app_state.get_portfolio(portfolio_handle);

          const auto runner_it = self.running_portfolios_.find(portfolio_handle);
          if(runner_it == self.running_portfolios_.end()) {
            continue;
          }

          auto& results =
            store.get_or_create_portfolio_results(portfolio_handle);
          auto& portfolio_runner = runner_it->second;
          if(portfolio_runner.is_failed()) {
            continue;
          }
          const auto total = portfolio_runner.total_timestamps();
          if(results.timeline().size() >= total) {
            continue;
          }

          try {
            made_progress |= portfolio_runner.run_batch(results, 1) != 0;
          } catch(const std::exception& e) {
            portfolio_runner.is_failed(true);
            self.portfolio_execution_statuses_[portfolio_handle] =
             BacktestExecutionStatus{BacktestExecutionPhase::Failed,
                                      results.timeline().size(),
                                      total,
                                     e.what()};

            const auto error_message = std::format(
             "Portfolio '{}' failed: {}", portfolio.name(), e.what());
            alert_messages.push_back(error_message);
          }
        }

      }

      // Publish progress once per portfolio after scheduling. This keeps the
      // hot loop focused on executing individual timestamps.
      for(const auto portfolio_handle : portfolio_handles) {
        const auto runner_it = self.running_portfolios_.find(portfolio_handle);
        if(runner_it == self.running_portfolios_.end() ||
           runner_it->second.is_failed()) {
          continue;
        }

        const auto& results = store.get_or_create_portfolio_results(portfolio_handle);
        const auto total = runner_it->second.total_timestamps();
        self.portfolio_execution_statuses_[portfolio_handle] =
         BacktestExecutionStatus{results.timeline().size() >= total
                                  ? BacktestExecutionPhase::Completed
                                  : BacktestExecutionPhase::Running,
                                 results.timeline().size(),
                                 total};
      }
    }

    ui::apply_dark_theme();

    auto window_context = WindowContext{
     app_state,
     alert_messages,
     self.command_executor_,
     self.discard_all_drafts_requested_,
     [&self](const auto& handle) {
       const auto it = self.portfolio_execution_statuses_.find(handle);
       return it == self.portfolio_execution_statuses_.end() ? nullptr
                                                             : &it->second;
     }};

    try {
      self.dockspace_window_.render(window_context);
      if(self.discard_all_drafts_requested_) {
         self.systems_window_.discard_draft();
        self.portfolios_window_.discard_draft();
        self.assets_window_.discard_draft();
        self.watchlists_window_.discard_draft();
         self.models_window_.discard_draft();
        self.markets_window_.discard_draft();
        self.brokers_window_.discard_draft();
         self.profiles_window_.discard_draft();
         self.strategies_window_.discard_draft();
        self.discard_all_drafts_requested_ = false;
      }
      self.backtest_chart_window_.render(window_context);
      auto backtest_overview = BacktestOverviewWindow{};
      backtest_overview.render(window_context);
      self.trade_list_window_.render(window_context);
      self.portfolios_window_.render(window_context);
       self.systems_window_.render(window_context);
      self.assets_window_.render(window_context);
      self.watchlists_window_.render(window_context);
       self.models_window_.render(window_context);
      self.markets_window_.render(window_context);
      self.brokers_window_.render(window_context);
       self.profiles_window_.render(window_context);
       self.strategies_window_.render(window_context);
    } catch(const std::exception& e) {
      const auto error_message = std::format("Error: {}", e.what());
      alert_messages.push_back(error_message);
    }

    try {
      const auto effect = self.command_executor_.execute(app_state);
      if(effect == ExecutionEffect::ApplicationReplaced) {
        const auto& settings = app_state.imgui_ini_settings();
        ImGui::LoadIniSettingsFromMemory(settings.c_str(), settings.size());
      }
      if(effect == ExecutionEffect::DocumentChanged ||
          effect == ExecutionEffect::ApplicationReplaced) {
        // Every runner is replaced below.  Its result must be discarded before
        // the replacement can consume it, otherwise a new runner resumes an
        // old, partial timeline.
        app_state.reset_all_portfolios();
        self.running_portfolios_.clear();
        self.portfolio_execution_statuses_.clear();
        for(const auto portfolio_handle : app_state.get_portfolio_handles()) {
          self.recreate_portfolio_runner(app_state, portfolio_handle);
        }
      }
    } catch(const std::exception& e) {
      const auto error_message = std::format("Error: {}", e.what());
      alert_messages.push_back(error_message);
    }

    if(!alert_messages.empty()) {
      const auto& alert_message = alert_messages.front();

      ImGui::OpenPopup("Alerts");

      if(ImGui::BeginPopupModal(
          "Alerts", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s", alert_message.c_str());

        if(ImGui::Button("OK")) {
          alert_messages.pop_front();
          ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
      }
    }
  }

private:
  void
   resolve_and_sync_strategy_inputs(this Application& self,
                                  backtest::Strategy& strategy,
                                 const backtest::Model& model) noexcept
  {
      auto synced_inputs = backtest::collect_model_inputs(model);
    const auto& previous_inputs = strategy.inputs();

    for(auto index = std::size_t{0}; index < synced_inputs.size(); ++index) {
      if(index >= previous_inputs.size()) {
        continue;
      }

      synced_inputs[index].value(previous_inputs[index].value());
    }

    strategy.inputs(std::move(synced_inputs));
  }

  void sync_stored_strategy_inputs(this Application& self,
                                   ApplicationState& app_state) noexcept
  {
    for(const auto strategy_handle : app_state.get_strategy_handles()) {
      auto* strategy =
       app_state.store().get_strategy_if_present(strategy_handle);
      if(!strategy) continue;
      const auto* model =
       app_state.get_model_if_present(strategy->model_handle());
      if(model) self.resolve_and_sync_strategy_inputs(*strategy, *model);
    }
  }

  auto make_backtest_runner(this Application& self,
                            ApplicationState& app_state,
                              const backtest::System& system,
                            backtest::AssetStoreHandle asset_handle,
                            const backtest::Portfolio& portfolio)
   -> backtest::BacktestRunner
  {
    const auto* asset_ptr = app_state.get_asset_if_present(asset_handle);
    const auto* market_ptr =
     app_state.get_market_if_present(portfolio.market_handle());
    const auto* broker_ptr =
     app_state.get_broker_if_present(portfolio.broker_handle());

    if(!asset_ptr || !market_ptr || !broker_ptr) {
      throw std::invalid_argument{"Incomplete Strategy in Portfolio"};
    }

    auto runner_setups = std::vector<backtest::BacktestRunner::CompiledStrategy>{};
    runner_setups.reserve(system.strategy_count());
    for(auto strategy_index = std::size_t{};
        strategy_index < system.strategy_count(); ++strategy_index) {
      const auto* configured_strategy = app_state.get_strategy_if_present(
       system.strategy_handle(strategy_index));
      if(!configured_strategy) {
        throw std::invalid_argument{"Missing Strategy setup in Portfolio"};
      }
      const auto* model_ptr =
       app_state.get_model_if_present(configured_strategy->model_handle());
      const auto* profile_ptr =
        app_state.get_profile_if_present(configured_strategy->profile_handle());
      if(!model_ptr || !profile_ptr) {
        throw std::invalid_argument{"Incomplete Strategy setup in Portfolio"};
      }

      auto input_values = std::vector<double>{};
      input_values.reserve(configured_strategy->inputs().size());
      for(const auto& input : configured_strategy->inputs()) {
        input_values.emplace_back(input.value());
      }

      auto input_context = NodeToErasedMethodContext{input_values};
      auto series_methods =
       OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>{};
      for(const auto& [series_name, series_node] :
           model_ptr->series_nodes()) {
        series_methods.set(series_name,
                           node_to_erased_method<ErasedSeriesMethodContext>(
                            series_node, input_context));
      }

      const auto make_position_rule =
        [&input_context](const backtest::Model::Position& position) {
         auto signal_exits =
          std::vector<backtest::BacktestRunner::PositionRule::SignalExitRule>{};
         signal_exits.reserve(position.exits().size());
         for(const auto& exit : position.exits()) {
           signal_exits.emplace_back(
            exit.enabled(),
            node_to_erased_method<ErasedSeriesMethodContext>(exit.signal(),
                                                             input_context),
            exit.timing(),
            exit.reduce());
         }
         auto take_profits =
          std::vector<backtest::BacktestRunner::PositionRule::TakeProfitRule>{};
         take_profits.reserve(position.take_profits().size());
         for(const auto& take_profit : position.take_profits()) {
           take_profits.emplace_back(
            node_to_erased_method<ErasedSeriesMethodContext>(
             take_profit.target_price(), input_context),
            take_profit.enabled(),
            take_profit.reduce());
         }
         auto stop_losses =
          std::vector<backtest::BacktestRunner::PositionRule::StopLossRule>{};
         stop_losses.reserve(position.stop_losses().size());
         for(const auto& stop_loss : position.stop_losses()) {
           stop_losses.emplace_back(
            node_to_erased_method<ErasedSeriesMethodContext>(
             stop_loss.stop_price(), input_context),
            stop_loss.enabled(),
            stop_loss.trailing(),
            stop_loss.reduce());
         }
         return backtest::BacktestRunner::PositionRule{
          node_to_erased_method<ErasedSeriesMethodContext>(
           position.entry().signal(), input_context),
          std::move(signal_exits),
          node_to_erased_method<ErasedSeriesMethodContext>(
           position.pyramiding().signal(), input_context),
          position.pyramiding().max_layers(),
          position.pyramiding().cooldown(),
          node_to_erased_method<ErasedSeriesMethodContext>(
           position.risk_distance(), input_context),
          std::move(stop_losses),
          position.entry().timing(),
          position.pyramiding().timing(),
          position.pyramiding().favorable_stop_target_reference(),
          position.pyramiding().unfavorable_stop_target_reference(),
          std::move(take_profits),
          position.exits_activation(),
          position.stop_losses_activation(),
          position.take_profits_activation(),
          position.pyramiding().retrigger()};
       };

      auto entry_filter_conversion_context = NodeToErasedMethodContext{};
      auto entry_filter =
       node_to_erased_method<backtest::EntryFilterMethodContext>(
         configured_strategy->entry_filter(), entry_filter_conversion_context);
      const auto failsafe_activation =
        strategy_index == 0
         ? backtest::FailsafeStrategyActivation::Always
         : system.failsafe_strategies()[strategy_index - 1].activation();
      runner_setups.emplace_back(
       *profile_ptr,
       std::move(series_methods),
        make_position_rule(model_ptr->long_position()),
        make_position_rule(model_ptr->short_position()),
        model_ptr->intrabar_path(),
         system.model_performance(),
       std::move(entry_filter),
       failsafe_activation);
    }

    return backtest::BacktestRunner{*asset_ptr,
                                    *market_ptr,
                                    *broker_ptr,
                                    std::move(runner_setups),
                                    portfolio.initial_capital(),
                                    false,
                                    NAN};
  }

  void
  recreate_portfolio_runner(this Application& self,
                            ApplicationState& app_state,
                            backtest::PortfolioStoreHandle portfolio_handle)
  {
    const auto* portfolio =
     app_state.get_portfolio_if_present(portfolio_handle);
    if(!portfolio || !app_state.is_portfolio_ready(*portfolio)) {
      self.portfolio_execution_statuses_.erase(portfolio_handle);
      return;
    }
    auto backtests = std::vector<backtest::PortfolioRunner::BacktestRun>{};
     const auto runs = app_state.expanded_strategy_runs(*portfolio);
    backtests.reserve(runs.size());
      for(const backtest::SystemStoreHandle system_handle :
          portfolio->system_handles()) {
        const backtest::System& system = app_state.get_system(system_handle);
       const auto& watchlist =
        app_state.get_watchlist(system.watchlist_handle());
       for(const auto asset_handle : watchlist.asset_handles()) {
         backtests.emplace_back(
           system_handle,
          asset_handle,
          self.make_backtest_runner(
           app_state, system, asset_handle, *portfolio));
       }
     }
    auto runner =
     backtest::PortfolioRunner{portfolio->initial_capital(),
                               portfolio->maximum_open_trades(),
                               portfolio->maximum_combined_layers(),
                               std::move(backtests),
                               portfolio->entry_comparators()};
    const auto total = runner.total_timestamps();
    self.running_portfolios_.insert_or_assign(portfolio_handle,
                                              std::move(runner));
    self.portfolio_execution_statuses_.insert_or_assign(
     portfolio_handle,
     BacktestExecutionStatus{total == 0 ? BacktestExecutionPhase::Completed
                                        : BacktestExecutionPhase::Waiting,
                             0,
                             total});
  }

  ImVec2 window_size_;

  DockspaceWindow dockspace_window_;
  BacktestChartWindow backtest_chart_window_;
  PortfoliosWindow portfolios_window_;
  SystemsWindow systems_window_;
  AssetsWindow assets_window_;
  WatchlistsWindow watchlists_window_;
   ModelsWindow models_window_;
  MarketsWindow markets_window_;
  BrokersWindow brokers_window_;
   ProfilesWindow profiles_window_;
   StrategiesWindow strategies_window_;
  TradeListWindow trade_list_window_;

  ApplicationState app_state_;
  std::unordered_map<backtest::PortfolioStoreHandle, backtest::PortfolioRunner>
   running_portfolios_;
  std::unordered_map<backtest::PortfolioStoreHandle, BacktestExecutionStatus>
   portfolio_execution_statuses_;
  std::list<std::string> alert_messages_;
  CommandExecutor command_executor_{};
  bool discard_all_drafts_requested_{false};
};

} // namespace pludux::apps
