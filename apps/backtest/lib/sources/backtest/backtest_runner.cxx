module;

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <format>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

export module pludux.backtest:backtest_runner;

import pludux;

import :store;
import :execution_model;
import :intrabar_path;
import :asset;
import :profile;
import :portfolio;
import :position_sizing;
import :entry_order_sizing;
import :trade_entry;
import :trade_exit;
import :take_profit_level;
import :signal_exit_state;
import :stop_loss_level;
import :closed_trade;
import :open_position_snapshot;
import :trade_event;
import :trade_position;
import :trade_session;
import :backtest_timeline;
import :backtest_method_context;
import :execution_filter_method_context;
import :broker;
import :market;
import :backtest;
import :strategy_intent;
import :strategy_performance;
import :strategy_session;

export namespace pludux::backtest {

class BacktestRunner {
public:
  class PositionRule {
  public:
    class SignalExitRule {
    public:
      SignalExitRule(bool enabled = false,
                     ErasedSeriesMethod<ErasedSeriesMethodContext>
                      signal_method = BooleanMethod<false>{},
                     SignalTiming timing = SignalTiming::NextOpen,
                     double reduce = 1.0)
      : enabled_{enabled}
      , signal_method_{std::move(signal_method)}
      , timing_{timing}
      , reduce_{reduce}
      {
      }

      auto enabled(this const SignalExitRule& self) noexcept -> bool
      {
        return self.enabled_;
      }

      auto signal_method(this const SignalExitRule& self) noexcept
       -> const ErasedSeriesMethod<ErasedSeriesMethodContext>&
      {
        return self.signal_method_;
      }

      auto timing(this const SignalExitRule& self) noexcept -> SignalTiming
      {
        return self.timing_;
      }

      auto reduce(this const SignalExitRule& self) noexcept -> double
      {
        return self.reduce_;
      }

    private:
      bool enabled_;
      ErasedSeriesMethod<ErasedSeriesMethodContext> signal_method_;
      SignalTiming timing_;
      double reduce_;
    };

    class TakeProfitRule {
    public:
      TakeProfitRule(ErasedSeriesMethod<ErasedSeriesMethodContext>
                      price_method = OpenMethod{},
                     bool enabled = false,
                     double reduce = 1.0)
      : price_method_{std::move(price_method)}
      , enabled_{enabled}
      , reduce_{reduce}
      {
      }

      auto price_method(this const TakeProfitRule& self) noexcept
       -> const ErasedSeriesMethod<ErasedSeriesMethodContext>&
      {
        return self.price_method_;
      }

      auto enabled(this const TakeProfitRule& self) noexcept -> bool
      {
        return self.enabled_;
      }

      auto reduce(this const TakeProfitRule& self) noexcept -> double
      {
        return self.reduce_;
      }

    private:
      ErasedSeriesMethod<ErasedSeriesMethodContext> price_method_;
      bool enabled_;
      double reduce_;
    };

    class StopLossRule {
    public:
      StopLossRule(ErasedSeriesMethod<ErasedSeriesMethodContext> price_method =
                    Sl1RMethod{},
                   bool enabled = true,
                   bool trailing = false,
                   double reduce = 1.0)
      : price_method_{std::move(price_method)}
      , enabled_{enabled}
      , trailing_{trailing}
      , reduce_{reduce}
      {
      }

      auto price_method(this const StopLossRule& self) noexcept
       -> const ErasedSeriesMethod<ErasedSeriesMethodContext>&
      {
        return self.price_method_;
      }

      auto enabled(this const StopLossRule& self) noexcept -> bool
      {
        return self.enabled_;
      }

      auto trailing(this const StopLossRule& self) noexcept -> bool
      {
        return self.trailing_;
      }

      auto reduce(this const StopLossRule& self) noexcept -> double
      {
        return self.reduce_;
      }

    private:
      ErasedSeriesMethod<ErasedSeriesMethodContext> price_method_;
      bool enabled_;
      bool trailing_;
      double reduce_;
    };

    PositionRule() = default;

    PositionRule(
     ErasedSeriesMethod<ErasedSeriesMethodContext> entry_method,
     std::vector<SignalExitRule> signal_exits,
     ErasedSeriesMethod<ErasedSeriesMethodContext> pyramiding_signal,
     std::size_t pyramiding_max_layers,
     std::size_t pyramiding_cooldown,
     ErasedSeriesMethod<ErasedSeriesMethodContext> risk_distance_method,
     std::vector<StopLossRule> stop_losses,
     SignalTiming entry_timing = SignalTiming::CurrentClose,
     SignalTiming pyramiding_timing = SignalTiming::NextOpen,
     StopTargetReferencePrice favorable_stop_target_reference =
      StopTargetReferencePrice::AveragePrice,
     StopTargetReferencePrice unfavorable_stop_target_reference =
      StopTargetReferencePrice::AveragePrice,
     std::vector<TakeProfitRule> take_profits = {},
     ExitActivation signal_exits_activation = ExitActivation::Simultaneous,
     ExitActivation stop_losses_activation = ExitActivation::Simultaneous,
     ExitActivation take_profits_activation = ExitActivation::Simultaneous,
     PyramidingRetrigger pyramiding_retrigger =
      PyramidingRetrigger::EveryEvaluation)
    : entry_method_{std::move(entry_method)}
    , signal_exits_{std::move(signal_exits)}
    , pyramiding_signal_{std::move(pyramiding_signal)}
    , pyramiding_max_layers_{pyramiding_max_layers}
    , pyramiding_cooldown_{pyramiding_cooldown}
    , risk_distance_method_{std::move(risk_distance_method)}
    , stop_losses_{std::move(stop_losses)}
    , entry_timing_{entry_timing}
    , pyramiding_timing_{pyramiding_timing}
    , favorable_stop_target_reference_{favorable_stop_target_reference}
    , unfavorable_stop_target_reference_{unfavorable_stop_target_reference}
    , take_profits_{std::move(take_profits)}
    , signal_exits_activation_{signal_exits_activation}
    , stop_losses_activation_{stop_losses_activation}
    , take_profits_activation_{take_profits_activation}
    , pyramiding_retrigger_{pyramiding_retrigger}
    {
    }

    auto entry_method(this const PositionRule& self) noexcept
     -> const ErasedSeriesMethod<ErasedSeriesMethodContext>&
    {
      return self.entry_method_;
    }

    auto signal_exits(this const PositionRule& self) noexcept
     -> const std::vector<SignalExitRule>&
    {
      return self.signal_exits_;
    }

    auto pyramiding_signal(this const PositionRule& self) noexcept
     -> const ErasedSeriesMethod<ErasedSeriesMethodContext>&
    {
      return self.pyramiding_signal_;
    }

    auto pyramiding_max_layers(this const PositionRule& self) noexcept
     -> std::size_t
    {
      return self.pyramiding_max_layers_;
    }

    auto pyramiding_cooldown(this const PositionRule& self) noexcept
     -> std::size_t
    {
      return self.pyramiding_cooldown_;
    }

    auto risk_distance_method(this const PositionRule& self) noexcept
     -> const ErasedSeriesMethod<ErasedSeriesMethodContext>&
    {
      return self.risk_distance_method_;
    }

    auto stop_losses(this const PositionRule& self) noexcept
     -> const std::vector<StopLossRule>&
    {
      return self.stop_losses_;
    }

    auto entry_timing(this const PositionRule& self) noexcept -> SignalTiming
    {
      return self.entry_timing_;
    }

    auto pyramiding_timing(this const PositionRule& self) noexcept
     -> SignalTiming
    {
      return self.pyramiding_timing_;
    }

    auto pyramiding_retrigger(this const PositionRule& self) noexcept
     -> PyramidingRetrigger
    {
      return self.pyramiding_retrigger_;
    }

    auto favorable_stop_target_reference(this const PositionRule& self) noexcept
     -> StopTargetReferencePrice
    {
      return self.favorable_stop_target_reference_;
    }

    auto
    unfavorable_stop_target_reference(this const PositionRule& self) noexcept
     -> StopTargetReferencePrice
    {
      return self.unfavorable_stop_target_reference_;
    }

    auto take_profits(this const PositionRule& self) noexcept
     -> const std::vector<TakeProfitRule>&
    {
      return self.take_profits_;
    }

    auto signal_exits_activation(this const PositionRule& self) noexcept
     -> ExitActivation
    {
      return self.signal_exits_activation_;
    }

    auto stop_losses_activation(this const PositionRule& self) noexcept
     -> ExitActivation
    {
      return self.stop_losses_activation_;
    }

    auto take_profits_activation(this const PositionRule& self) noexcept
     -> ExitActivation
    {
      return self.take_profits_activation_;
    }

  private:
    ErasedSeriesMethod<ErasedSeriesMethodContext> entry_method_{
     BooleanMethod<false>{}};
    std::vector<SignalExitRule> signal_exits_;
    ErasedSeriesMethod<ErasedSeriesMethodContext> pyramiding_signal_{
     BooleanMethod<false>{}};
    std::size_t pyramiding_max_layers_{1};
    std::size_t pyramiding_cooldown_{};
    ErasedSeriesMethod<ErasedSeriesMethodContext> risk_distance_method_{
     ValueMethod{1.0}};
    std::vector<StopLossRule> stop_losses_;
    SignalTiming entry_timing_{SignalTiming::CurrentClose};
    SignalTiming pyramiding_timing_{SignalTiming::NextOpen};
    StopTargetReferencePrice favorable_stop_target_reference_{
     StopTargetReferencePrice::AveragePrice};
    StopTargetReferencePrice unfavorable_stop_target_reference_{
     StopTargetReferencePrice::AveragePrice};
    std::vector<TakeProfitRule> take_profits_;
    ExitActivation signal_exits_activation_{ExitActivation::Simultaneous};
    ExitActivation stop_losses_activation_{ExitActivation::Simultaneous};
    ExitActivation take_profits_activation_{ExitActivation::Simultaneous};
    PyramidingRetrigger pyramiding_retrigger_{
     PyramidingRetrigger::EveryEvaluation};
  };

  BacktestRunner(
   const Asset& asset,
   const Market& market,
   const Broker& broker,
   const Profile& profile,
   OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>
    series_methods,
   PositionRule long_position,
   PositionRule short_position,
   double total_equity = 0.0,
   std::size_t pyramiding_layers = 0,
   bool is_failed = false,
   double peak_equity = NAN,
   IntrabarPath intrabar_path = IntrabarPath::CandleDirection,
   StrategyPerformanceConfig strategy_performance_config = {},
   ErasedSeriesMethod<ExecutionFilterMethodContext> execution_filter =
    BooleanMethod<true>{},
   DrawdownAdjustment drawdown_adjustment = {},
   InsufficientCashPolicy insufficient_cash_policy =
    InsufficientCashPolicy::Reject)
  : asset_{asset}
  , market_{market}
  , broker_{broker}
  , profile_{profile}
  , drawdown_adjustment_{drawdown_adjustment}
  , insufficient_cash_policy_{insufficient_cash_policy}
  , current_account_state_{total_equity,
                           0.0,
                           std::isnan(peak_equity) ? total_equity : peak_equity,
                           total_equity}
  , max_drawdown_{0.0}
  , sum_of_durations_{}
  , cumulative_investments_{0.0}
  , profit_count_{0}
  , cumulative_profits_{0.0}
  , loss_count_{0}
  , cumulative_losses_{0.0}
  , break_even_count_{0}
  , strategy_trade_session_{}
  , execution_session_{}
  , strategy_session_{}
  , strategy_performance_{std::move(strategy_performance_config)}
  , execution_filter_{std::move(execution_filter)}
  , position_sizing_{profile.position_sizing().make_method()}
  , series_methods_{std::move(series_methods)}
  , long_position_{std::move(long_position)}
  , short_position_{std::move(short_position)}
  , pyramiding_layers_{pyramiding_layers}
  , is_failed_{is_failed}
  , intrabar_path_{intrabar_path}
  {
  }

  auto is_failed(this const BacktestRunner& self) noexcept -> bool
  {
    return self.is_failed_;
  }

  void is_failed(this BacktestRunner& self, bool is_failed) noexcept
  {
    self.is_failed_ = is_failed;
  }

  auto asset(this const BacktestRunner& self) noexcept -> const Asset&
  {
    return self.asset_;
  }

  void portfolio_account(this BacktestRunner& self,
                         BacktestAccountState account,
                         double reserved_notional) noexcept
  {
    self.other_backtests_unrealized_pnl_ =
     account.unrealized_pnl() - self.execution_session_.unrealized_pnl();
    self.current_account_state_ = account;
    self.other_backtests_reserved_notional_ =
     std::max(reserved_notional - self.reserved_notional(), 0.0);
  }

  auto account_state(this const BacktestRunner& self) noexcept
   -> const BacktestAccountState&
  {
    return self.current_account_state_;
  }

  auto unrealized_pnl(this const BacktestRunner& self) noexcept -> double
  {
    return self.execution_session_.unrealized_pnl();
  }

  auto reserved_notional(this const BacktestRunner& self) noexcept -> double
  {
    return std::abs(self.execution_session_.unrealized_investment());
  }

  auto net_exposure(this const BacktestRunner& self) noexcept -> double
  {
    const auto& position = self.execution_session_.open_position();
    if(!position) {
      return 0.0;
    }
    return position->position_size() * self.execution_session_.market_price();
  }

  auto has_open_position(this const BacktestRunner& self) noexcept -> bool
  {
    return self.execution_session_.is_open();
  }

  void run(this BacktestRunner& self,
           SeriesEvaluationResults& series_evaluation_results,
           BacktestTimeline& timeline)
  {
    if(!self.begin_bar(timeline)) {
      return;
    }
    self.run_open_exits();
    self.run_open_entries(series_evaluation_results);
    self.run_intrabar();
    self.run_close_exits(series_evaluation_results);
    self.run_close_entries(series_evaluation_results);
    self.finish_bar(series_evaluation_results, timeline);
  }

  auto begin_bar(this BacktestRunner& self, const BacktestTimeline& timeline)
   -> bool
  {
    if(self.is_failed() || self.active_snapshot_ ||
       timeline.size() >= self.asset_.size()) {
      return false;
    }
    self.active_timeline_index_ = timeline.size();
    self.active_snapshot_.emplace(
     self.asset_.get_snapshot(self.active_timeline_index_));
    const auto& asset_snapshot = *self.active_snapshot_;
    const auto market_timestamp =
     static_cast<std::time_t>(asset_snapshot.datetime());
    self.strategy_trade_session_.begin_market_bar(
     market_timestamp, asset_snapshot.close(), asset_snapshot.lookback());
    self.execution_session_.begin_market_bar(
     market_timestamp, asset_snapshot.close(), asset_snapshot.lookback());
    self.strategy_session_.begin_market_bar(market_timestamp,
                                            asset_snapshot.close());
    self.execution_filter_decisions_.clear();
    self.position_sizing_decisions_.clear();
    self.closed_position_is_long_.reset();
    self.settled_realized_exit_count_ = 0;
    self.settled_closed_trade_count_ = 0;

    self.current_account_state_.unrealized_pnl(
     self.other_backtests_unrealized_pnl_ +
     self.execution_session_.unrealized_pnl());
    return true;
  }

  void run_open_exits(this BacktestRunner& self)
  {
    const auto& asset_snapshot = *self.active_snapshot_;
    self.process_open_price(asset_snapshot.open(),
                            self.closed_position_is_long_);
    self.execute_pending_signal_exits(asset_snapshot.open(),
                                      self.closed_position_is_long_);
  }

  void run_open_entries(this BacktestRunner& self,
                        SeriesEvaluationResults& series_evaluation_results)
  {
    const auto asset_snapshot = *self.active_snapshot_;
    auto context = self.make_context(series_evaluation_results);
    const auto decision_snapshot =
     self.active_timeline_index_ > 0 ? asset_snapshot[1] : asset_snapshot;
    const auto current_drawdown_ratio =
     self.current_account_state_.drawdown_ratio();
    if(self.strategy_trade_session_.is_open() && self.pending_pyramiding_) {
      self.execute_pyramiding_action(asset_snapshot.open(),
                                     decision_snapshot,
                                     context,
                                     current_drawdown_ratio);
    }
    self.pending_pyramiding_ = false;

    if(self.strategy_trade_session_.is_flat()) {
      self.execute_pending_entry(asset_snapshot.open(),
                                 decision_snapshot,
                                 context,
                                 current_drawdown_ratio,
                                 self.closed_position_is_long_);
    }
    self.pending_entries_.fill(false);
  }

  void run_intrabar(this BacktestRunner& self)
  {
    const auto& asset_snapshot = *self.active_snapshot_;
    const auto prices = make_intrabar_prices(self.intrabar_path_,
                                             asset_snapshot.open(),
                                             asset_snapshot.high(),
                                             asset_snapshot.low(),
                                             asset_snapshot.close());
    for(auto index = std::size_t{1}; index < prices.size(); ++index) {
      self.process_price_segment(
       prices[index - 1], prices[index], self.closed_position_is_long_);
    }
  }

  void run_close_exits(this BacktestRunner& self,
                       SeriesEvaluationResults& series_evaluation_results)
  {
    const auto asset_snapshot = *self.active_snapshot_;
    auto context = self.make_context(series_evaluation_results);
    self.execute_signal_exits(SignalTiming::CurrentClose,
                              asset_snapshot.close(),
                              asset_snapshot,
                              context,
                              self.closed_position_is_long_);
  }

  void run_close_entries(this BacktestRunner& self,
                         SeriesEvaluationResults& series_evaluation_results)
  {
    const auto asset_snapshot = *self.active_snapshot_;
    auto context = self.make_context(series_evaluation_results);
    const auto current_drawdown_ratio =
     self.current_account_state_.drawdown_ratio();
    if(self.strategy_trade_session_.is_open()) {
      const auto& position = *self.strategy_trade_session_.open_position();
      const auto& rule = position.is_long_direction() ? self.long_position_
                                                      : self.short_position_;
      const auto position_context =
       self.with_open_position_context(context, position);
      if(rule.pyramiding_timing() == SignalTiming::CurrentClose &&
         !self.pyramiding_is_paused() &&
         self.pyramiding_signal_triggered(
          rule, asset_snapshot, position_context)) {
        self.execute_pyramiding_action(asset_snapshot.close(),
                                       asset_snapshot,
                                       context,
                                       current_drawdown_ratio);
      }
    }

    if(self.strategy_trade_session_.is_flat()) {
      self.execute_current_close_entry(asset_snapshot.close(),
                                       asset_snapshot,
                                       context,
                                       current_drawdown_ratio,
                                       self.closed_position_is_long_);
    }
  }

  void settle_portfolio_account(this BacktestRunner& self) noexcept
  {
    self.update_accounting();
  }

  void finish_bar(this BacktestRunner& self,
                  SeriesEvaluationResults& series_evaluation_results,
                  BacktestTimeline& timeline)
  {
    const auto asset_snapshot = *self.active_snapshot_;
    auto context = self.make_context(series_evaluation_results);
    if(self.active_timeline_index_ + 1 < self.asset_.size()) {
      self.schedule_next_open_actions(asset_snapshot, context);
    } else {
      self.pending_entries_.fill(false);
      self.pending_pyramiding_ = false;
      self.pending_signal_exit_indices_.clear();
      self.pending_signal_exit_trade_id_.reset();
    }

    self.update_accounting();

    timeline.append(BacktestTimeline::Row{
     .market_timestamp = self.execution_session_.market_timestamp(),
     .market_price = self.execution_session_.market_price(),
     .market_lookback = self.execution_session_.market_lookback(),
     .trade_events = self.execution_session_.trade_events(),
     .closed_trades = self.execution_session_.closed_trades(),
     .open_position = self.execution_session_.open_position_snapshot(),
     .strategy_intents = self.strategy_session_.intents(),
     .strategy_closed_positions = self.strategy_session_.closed_positions(),
     .strategy_open_position = self.strategy_session_.position(),
     .execution_filter_decisions = self.execution_filter_decisions_,
     .position_sizing_decisions = self.position_sizing_decisions_,
     .strategy_performance = self.strategy_performance_.snapshot(),
     .capital = self.current_account_state_.capital(),
     .equity = self.current_account_state_.equity(),
     .peak_equity = self.current_account_state_.peak_equity(),
     .drawdown = self.current_account_state_.drawdown(),
     .max_drawdown = self.max_drawdown_,
     .cumulative_duration = self.sum_of_durations_,
     .cumulative_investment = self.cumulative_investments_,
     .profit_count = self.profit_count_,
     .cumulative_profit = self.cumulative_profits_,
     .loss_count = self.loss_count_,
     .cumulative_loss = self.cumulative_losses_,
     .break_even_count = self.break_even_count_,
     .open_trade_count = self.execution_session_.is_open() ? 1U : 0U,
     .unrealized_pnl = self.execution_session_.unrealized_pnl(),
     .unrealized_investment = self.execution_session_.unrealized_investment(),
     .unrealized_duration = self.execution_session_.unrealized_duration()});

    const auto series_context = self.with_open_position_context(context);
    for(const auto& [series_name, series_method] : self.series_methods_) {
      const auto series_value =
       evaluate_series_method(series_method, asset_snapshot, series_context);
      series_evaluation_results.put(series_method, series_value);
      series_evaluation_results.alias(series_name, series_method);
    }
    self.active_snapshot_.reset();
  }

private:
  const Asset& asset_;
  const Market& market_;
  const Broker& broker_;
  const Profile& profile_;
  DrawdownAdjustment drawdown_adjustment_;
  InsufficientCashPolicy insufficient_cash_policy_;

  BacktestAccountState current_account_state_;
  double other_backtests_reserved_notional_{};
  double other_backtests_unrealized_pnl_{};
  std::optional<AssetSnapshot> active_snapshot_;
  std::size_t active_timeline_index_{};
  std::optional<bool> closed_position_is_long_;
  std::size_t settled_realized_exit_count_{};
  std::size_t settled_closed_trade_count_{};
  double max_drawdown_;

  std::time_t sum_of_durations_;
  double cumulative_investments_;

  std::size_t profit_count_;
  double cumulative_profits_;

  std::size_t loss_count_;
  double cumulative_losses_;

  std::size_t break_even_count_;

  TradeSession strategy_trade_session_;
  TradeSession execution_session_;
  StrategySession strategy_session_;
  StrategyPerformance strategy_performance_;
  ErasedSeriesMethod<ExecutionFilterMethodContext> execution_filter_;
  std::vector<ExecutionFilterDecision> execution_filter_decisions_;
  PositionSizingMethod position_sizing_;
  std::vector<PositionSizingDecision> position_sizing_decisions_;
  std::optional<std::size_t> execution_strategy_trade_id_;

  OrderedNamedRegistry<ErasedSeriesMethod<ErasedSeriesMethodContext>>
   series_methods_;

  PositionRule long_position_;
  PositionRule short_position_;

  std::size_t pyramiding_layers_;

  bool is_failed_;
  IntrabarPath intrabar_path_;

  std::array<bool, 2> pending_entries_{};
  bool pending_pyramiding_{};
  bool pyramiding_signal_ready_{true};
  std::optional<std::size_t> pyramiding_resume_index_;
  std::vector<std::size_t> pending_signal_exit_indices_;
  std::optional<std::size_t> pending_signal_exit_trade_id_;

  struct PriceExitCandidate {
    TradeExit::Reason reason;
    std::size_t index;
    double price;
    int priority;
  };

  auto make_context(this BacktestRunner& self,
                    SeriesEvaluationResults& series_evaluation_results)
   -> BacktestMethodContext
  {
    auto default_context = DefaultMethodContext{self.series_methods_,
                                                series_evaluation_results,
                                                self.active_timeline_index_};
    return BacktestMethodContext{std::move(default_context),
                                 self.series_methods_,
                                 self.current_account_state_,
                                 self.pyramiding_layers_};
  }

  auto with_open_position_context(this const BacktestRunner& self,
                                  BacktestMethodContext context,
                                  const TradePosition& position) noexcept
   -> BacktestMethodContext
  {
    const auto direction = position.is_long_direction() ? 1.0 : -1.0;
    return context
     .with_position_prices(position.entry_price(),
                           position.latest_entry_price(),
                           position.average_price(),
                           position.risk_reference_price(),
                           direction)
     .with_position_risk_distance(position.risk_distance());
  }

  auto with_open_position_context(this const BacktestRunner& self,
                                  BacktestMethodContext context) noexcept
   -> BacktestMethodContext
  {
    if(!self.strategy_trade_session_.open_position()) {
      return context;
    }
    return self.with_open_position_context(
     std::move(context), *self.strategy_trade_session_.open_position());
  }

  auto pyramiding_signal_triggered(this BacktestRunner& self,
                                   const PositionRule& rule,
                                   const AssetSnapshot& snapshot,
                                   MethodContextable auto context) -> bool
  {
    const auto signal = static_cast<bool>(
     evaluate_series_method(rule.pyramiding_signal(), snapshot, context));
    if(rule.pyramiding_retrigger() == PyramidingRetrigger::EveryEvaluation) {
      return signal;
    }
    if(!signal) {
      self.pyramiding_signal_ready_ = true;
      return false;
    }
    if(!self.pyramiding_signal_ready_) {
      return false;
    }
    self.pyramiding_signal_ready_ = false;
    return true;
  }

  auto pyramiding_is_paused(this const BacktestRunner& self) noexcept -> bool
  {
    return self.pyramiding_resume_index_ &&
           self.active_timeline_index_ < *self.pyramiding_resume_index_;
  }

  void restart_pyramiding_cooldown(this BacktestRunner& self,
                                   const PositionRule& rule) noexcept
  {
    if(rule.pyramiding_cooldown() == 0) {
      self.pyramiding_resume_index_.reset();
      return;
    }
    const auto remaining_indices =
     std::numeric_limits<std::size_t>::max() - self.active_timeline_index_;
    self.pyramiding_resume_index_ =
     rule.pyramiding_cooldown() >= remaining_indices
      ? std::numeric_limits<std::size_t>::max()
      : self.active_timeline_index_ + rule.pyramiding_cooldown() + 1;
  }

  auto current_equity(this const BacktestRunner& self) noexcept -> double
  {
    return self.current_account_state_.equity();
  }

  auto current_drawdown(this const BacktestRunner& self) noexcept -> double
  {
    return self.current_account_state_.drawdown();
  }

  auto available_cash(this const BacktestRunner& self) noexcept -> double
  {
    return std::max(self.current_account_state_.capital() -
                     self.other_backtests_reserved_notional_ -
                     self.reserved_notional(),
                    0.0);
  }

  auto prepare_entry_order(this BacktestRunner& self,
                           const EntryOrderSizingRequest& request,
                           PositionSizingDecision& decision)
   -> std::optional<TradeEntry>
  {
    const auto sized = size_entry_order(request, self.market_, self.broker_);
    if(!sized) {
      decision.outcome = PositionSizingDecisionOutcome::SizingLimitTooSmall;
      return std::nullopt;
    }

    decision.sizing_normalized_quantity =
     std::abs(sized->entry.position_size());
    decision.entry_cost = sized->entry_cost;
    decision.estimated_loss = sized->estimated_loss;
    const auto cash = self.available_cash();
    decision.cash_required = sized->entry_cost;
    decision.cash_available = cash;
    if(sized->entry_cost <= cash) {
      decision.final_quantity = std::abs(sized->entry.position_size());
      decision.final_entry_cost = sized->entry_cost;
      decision.outcome = PositionSizingDecisionOutcome::Executed;
      return sized->entry;
    }

    switch(self.insufficient_cash_policy_) {
    case InsufficientCashPolicy::Reject:
      self.execution_session_.reject_insufficient_cash(
       sized->entry, cash, sized->entry_cost);
      decision.outcome = PositionSizingDecisionOutcome::InsufficientCash;
      return std::nullopt;
    case InsufficientCashPolicy::CapToAvailableCash:
      if(cash > 0.0) {
        const auto capped = size_entry_order(
         EntryOrderSizingRequest{sized->entry.position_size(),
                                 sized->entry.price(),
                                 EntryCostBudgetConstraint{cash}},
         self.market_,
         self.broker_);
        if(capped) {
          decision.cash_adjusted = true;
          decision.final_quantity = std::abs(capped->entry.position_size());
          decision.final_entry_cost = capped->entry_cost;
          decision.outcome = PositionSizingDecisionOutcome::Executed;
          return capped->entry;
        }
      }
      decision.outcome = PositionSizingDecisionOutcome::InsufficientCash;
      return std::nullopt;
    }

    return std::nullopt;
  }

  void update_accounting(this BacktestRunner& self) noexcept
  {
    const auto& realized_exits = self.execution_session_.realized_exits();
    for(auto index = self.settled_realized_exit_count_;
        index < realized_exits.size();
        ++index) {
      const auto& realized_exit = realized_exits[index];
      self.current_account_state_.capital(
       self.current_account_state_.capital() + realized_exit.pnl());
    }
    self.settled_realized_exit_count_ = realized_exits.size();

    const auto& closed_trades = self.execution_session_.closed_trades();
    for(auto index = self.settled_closed_trade_count_;
        index < closed_trades.size();
        ++index) {
      const auto& closed_trade = closed_trades[index];
      const auto pnl = closed_trade.pnl();

      self.sum_of_durations_ += closed_trade.duration();
      self.cumulative_investments_ += closed_trade.investment();

      if(pnl > 0) {
        self.profit_count_++;
        self.cumulative_profits_ += pnl;
      } else if(pnl < 0) {
        self.loss_count_++;
        self.cumulative_losses_ += pnl;
      } else {
        self.break_even_count_++;
      }
    }
    self.settled_closed_trade_count_ = closed_trades.size();

    self.current_account_state_.unrealized_pnl(
     self.other_backtests_unrealized_pnl_ +
     self.execution_session_.unrealized_pnl());
    self.current_account_state_.update_peak_to_current_equity();
    self.max_drawdown_ = std::max(self.max_drawdown_, self.current_drawdown());
  }

  auto create_trade(this const BacktestRunner& self,
                    const PositionRule& position,
                    bool is_long,
                    double entry_price,
                    const AssetSnapshot& evaluation_snapshot,
                    MethodContextable auto context,
                    double current_drawdown_ratio,
                    const StrategyPerformanceSnapshot& performance_snapshot,
                    PositionSizingDecision& decision)
   -> std::optional<EntryOrderSizingRequest>
  {
    const auto direction = is_long ? 1.0 : -1.0;
    const auto initial_price_context =
     context.with_position_reference(entry_price, direction);
    const auto sizing_context = PositionSizingContext{
     self.current_account_state_.equity(),
     entry_price,
     direction,
     performance_snapshot,
     [&position, &evaluation_snapshot, &initial_price_context] {
       return evaluate_series_method(position.risk_distance_method(),
                                     evaluation_snapshot,
                                     initial_price_context);
     }};
    const auto evaluation = self.position_sizing_.evaluate(sizing_context);
    if(!std::isfinite(evaluation.requested_quantity) ||
       evaluation.requested_quantity < 0.0) {
      throw std::runtime_error{"Invalid position sizing quantity"};
    }
    decision.requested_quantity = evaluation.requested_quantity;
    decision.requested_limit = entry_order_sizing_limit(evaluation.constraint);
    decision.bayesian_kelly = evaluation.bayesian_kelly;
    if(evaluation.requested_quantity <= 0.0) {
      decision.outcome = PositionSizingDecisionOutcome::NoPositiveSize;
      return std::nullopt;
    }

    const auto adjusted_position_quantity = self.apply_drawdown_adjustment(
     evaluation.requested_quantity, current_drawdown_ratio);
    const auto adjustment_multiplier =
     adjusted_position_quantity / evaluation.requested_quantity;
    const auto adjusted_constraint = scale_entry_order_sizing_constraint(
     evaluation.constraint, adjustment_multiplier);
    decision.drawdown_adjusted_quantity = adjusted_position_quantity;
    decision.drawdown_adjusted_limit =
     entry_order_sizing_limit(adjusted_constraint);
    if(adjusted_position_quantity <= 0.0) {
      decision.outcome = PositionSizingDecisionOutcome::DrawdownSuppressed;
      return std::nullopt;
    }

    return EntryOrderSizingRequest{
     direction * adjusted_position_quantity, entry_price, adjusted_constraint};
  }

  auto apply_drawdown_adjustment(this const BacktestRunner& self,
                                 double position_quantity,
                                 double current_drawdown_ratio) -> double
  {
    const auto& drawdown_adjustment = self.drawdown_adjustment_;

    if(!drawdown_adjustment.enabled()) {
      return position_quantity;
    }

    const auto drawdown_step = drawdown_adjustment.drawdown_step();
    const auto size_reduction = drawdown_adjustment.size_reduction();
    if(!std::isfinite(current_drawdown_ratio) ||
       !std::isfinite(drawdown_step) || drawdown_step <= 0.0 ||
       !std::isfinite(size_reduction) || size_reduction < 0.0) {
      throw std::runtime_error{"Invalid drawdown adjustment"};
    }

    const auto steps =
     std::floor(std::max(current_drawdown_ratio, 0.0) / drawdown_step);
    const auto multiplier = std::max(1.0 - steps * size_reduction, 0.0);
    return position_quantity * multiplier;
  }

  auto stop_target_reference_price(this const BacktestRunner&,
                                   const PositionRule& position,
                                   const TradePosition& open_position,
                                   const TradeEntry& entry,
                                   bool is_pyramiding) noexcept -> double
  {
    const auto entry_price = entry.price();
    const auto average_price = open_position.average_price();
    if(!is_pyramiding) {
      return average_price;
    }

    const auto is_long = open_position.is_long_direction();
    const auto is_favorable =
     is_long ? entry_price >= average_price : entry_price <= average_price;
    const auto reference = is_favorable
                            ? position.favorable_stop_target_reference()
                            : position.unfavorable_stop_target_reference();

    switch(reference) {
    case StopTargetReferencePrice::LatestEntryPrice:
      return entry_price;
    case StopTargetReferencePrice::AveragePrice:
      return average_price;
    case StopTargetReferencePrice::InitialEntryPrice:
      return open_position.entry_price();
    }

    return entry_price;
  }

  void update_stop_target_prices(this const BacktestRunner& self,
                                 TradePosition& open_position,
                                 const TradeEntry& entry,
                                 const PositionRule& position,
                                 bool is_pyramiding,
                                 const AssetSnapshot& asset_snapshot,
                                 MethodContextable auto context)
  {
    const auto direction = open_position.is_long_direction() ? 1.0 : -1.0;
    const auto initial_entry_price = open_position.entry_price();
    const auto latest_entry_price = entry.price();
    const auto average_price = open_position.average_price();
    const auto reference_price = self.stop_target_reference_price(
     position, open_position, entry, is_pyramiding);
    const auto stop_context = context.with_position_prices(initial_entry_price,
                                                           latest_entry_price,
                                                           average_price,
                                                           reference_price,
                                                           direction);
    const auto risk_distance = evaluate_series_method(
     position.risk_distance_method(), asset_snapshot, stop_context);
    if(!std::isfinite(risk_distance) || risk_distance <= 0.0) {
      throw std::runtime_error{
       "Invalid risk distance: expected a finite positive value"};
    }
    const auto target_context =
     stop_context.with_position_risk_distance(risk_distance);

    const auto previous_stop_levels = open_position.stop_loss_levels();
    const auto stop_is_eligible = [&](std::size_t target_index) {
      if(position.stop_losses_activation() == ExitActivation::Simultaneous) {
        return true;
      }
      for(auto index = std::size_t{0}; index < position.stop_losses().size();
          ++index) {
        if(!position.stop_losses()[index].enabled()) {
          continue;
        }
        const auto consumed = index < previous_stop_levels.size() &&
                              previous_stop_levels[index].consumed();
        if(!consumed) {
          return index == target_index;
        }
      }
      return false;
    };
    auto stop_levels = std::vector<StopLossLevel>{};
    stop_levels.reserve(position.stop_losses().size());
    for(auto index = std::size_t{0}; index < position.stop_losses().size();
        ++index) {
      const auto& stop_loss = position.stop_losses()[index];
      const auto evaluated_price = evaluate_series_method(
       stop_loss.price_method(), asset_snapshot, target_context);
      if(stop_loss.enabled() && !std::isfinite(evaluated_price)) {
        throw std::runtime_error{"Invalid stop price for stop-loss exit"};
      }
      const auto has_previous = index < previous_stop_levels.size();
      const auto consumed =
       has_previous ? previous_stop_levels[index].consumed() : false;
      auto effective_price = evaluated_price;
      auto trail_distance = NAN;
      auto favorable_anchor = NAN;
      if(stop_loss.trailing()) {
        trail_distance = direction * (reference_price - evaluated_price);
        if(stop_loss.enabled() &&
           (!std::isfinite(trail_distance) || trail_distance < 0.0)) {
          throw std::runtime_error{
           "Invalid trailing stop: expected a non-negative directional "
           "distance"};
        }
        if(has_previous &&
           std::isfinite(previous_stop_levels[index].favorable_anchor())) {
          favorable_anchor = previous_stop_levels[index].favorable_anchor();
        } else if(stop_is_eligible(index) && !consumed) {
          favorable_anchor = entry.price();
        }
        if(std::isfinite(favorable_anchor)) {
          const auto candidate = favorable_anchor - direction * trail_distance;
          effective_price = open_position.is_long_direction()
                             ? std::max(effective_price, candidate)
                             : std::min(effective_price, candidate);
        }
      }
      if(is_pyramiding && has_previous &&
         std::isfinite(previous_stop_levels[index].effective_price())) {
        effective_price =
         open_position.is_long_direction()
          ? std::max(effective_price,
                     previous_stop_levels[index].effective_price())
          : std::min(effective_price,
                     previous_stop_levels[index].effective_price());
      }
      stop_levels.emplace_back(evaluated_price,
                               effective_price,
                               stop_loss.enabled(),
                               stop_loss.trailing(),
                               consumed,
                               trail_distance,
                               favorable_anchor);
    }

    const auto previous_levels = open_position.take_profit_levels();
    auto levels = std::vector<TakeProfitLevel>{};
    levels.reserve(position.take_profits().size());
    for(auto index = std::size_t{0}; index < position.take_profits().size();
        ++index) {
      const auto& take_profit = position.take_profits()[index];
      const auto target_price = evaluate_series_method(
       take_profit.price_method(), asset_snapshot, target_context);
      if(take_profit.enabled() && !std::isfinite(target_price)) {
        throw std::runtime_error{"Invalid target price for take-profit exit"};
      }
      const auto consumed = index < previous_levels.size()
                             ? previous_levels[index].consumed()
                             : false;
      levels.emplace_back(target_price, take_profit.enabled(), consumed);
    }

    const auto previous_exit_states = open_position.signal_exit_states();
    auto exit_states = std::vector<SignalExitState>{};
    exit_states.reserve(position.signal_exits().size());
    for(auto index = std::size_t{0}; index < position.signal_exits().size();
        ++index) {
      const auto consumed = index < previous_exit_states.size()
                             ? previous_exit_states[index].consumed()
                             : false;
      exit_states.emplace_back(position.signal_exits()[index].enabled(),
                               consumed);
    }

    open_position.stop_loss_levels(std::move(stop_levels));
    open_position.risk_distance(risk_distance);
    open_position.risk_reference_price(reference_price);
    open_position.risk_boundary_price(reference_price -
                                      direction * risk_distance);
    open_position.take_profit_levels(std::move(levels));
    open_position.signal_exit_states(std::move(exit_states));
  }

  auto
  make_exit_trade(this const BacktestRunner& self,
                  double remaining_position_size,
                  double exit_price,
                  TradeExit::Reason reason,
                  double reduce,
                  std::optional<std::size_t> stop_loss_index = std::nullopt,
                  std::optional<std::size_t> take_profit_index = std::nullopt,
                  std::optional<std::size_t> signal_exit_index = std::nullopt)
   -> std::optional<TradeExit>
  {
    if(!std::isfinite(reduce) || reduce <= 0.0 || reduce > 1.0) {
      throw std::runtime_error{"Invalid exit reduce value: expected (0, 1]"};
    }

    const auto remaining_quantity = std::abs(remaining_position_size);
    if(remaining_quantity == 0.0) {
      return std::nullopt;
    }

    auto exit_quantity = remaining_quantity;
    if(reduce < 1.0) {
      const auto quantity_step = self.market_.quantity_step();
      const auto minimum_quantity = self.market_.min_order_quantity();
      const auto snap_to_integer = [](double value) noexcept {
        const auto nearest_integer = std::round(value);
        const auto tolerance = 1e-12 * std::max(1.0, std::abs(value));
        return std::abs(value - nearest_integer) <= tolerance ? nearest_integer
                                                              : value;
      };
      const auto minimum_valid_quantity =
       quantity_step > 0.0
        ? std::max(quantity_step,
                   quantity_step * std::ceil(snap_to_integer(minimum_quantity /
                                                             quantity_step)))
        : minimum_quantity;

      exit_quantity = remaining_quantity * reduce;
      if(quantity_step > 0.0) {
        const auto step_ratio = snap_to_integer(exit_quantity / quantity_step);

        exit_quantity = quantity_step * std::floor(step_ratio + 0.5);
      }

      exit_quantity = std::max(exit_quantity, minimum_valid_quantity);
      exit_quantity = std::min(exit_quantity, remaining_quantity);

      const auto remaining_after_exit = remaining_quantity - exit_quantity;
      if(remaining_after_exit > 0.0 &&
         remaining_after_exit < minimum_valid_quantity) {
        exit_quantity = remaining_quantity;
      }
    }

    exit_quantity = std::min(exit_quantity, remaining_quantity);
    const auto direction = remaining_position_size > 0.0 ? 1.0 : -1.0;
    return TradeExit{direction * exit_quantity,
                     exit_price,
                     reason,
                     stop_loss_index,
                     take_profit_index,
                     signal_exit_index};
  }

  auto make_strategy_exit_trade(
   this const BacktestRunner&,
   double remaining_position_size,
   double exit_price,
   TradeExit::Reason reason,
   double reduce,
   std::optional<std::size_t> stop_loss_index = std::nullopt,
   std::optional<std::size_t> take_profit_index = std::nullopt,
   std::optional<std::size_t> signal_exit_index = std::nullopt)
   -> std::optional<TradeExit>
  {
    if(!std::isfinite(reduce) || reduce <= 0.0 || reduce > 1.0) {
      throw std::runtime_error{"Invalid exit reduce value: expected (0, 1]"};
    }
    const auto remaining_quantity = std::abs(remaining_position_size);
    if(remaining_quantity == 0.0) {
      return std::nullopt;
    }
    const auto direction = remaining_position_size > 0.0 ? 1.0 : -1.0;
    return TradeExit{direction * remaining_quantity * reduce,
                     exit_price,
                     reason,
                     stop_loss_index,
                     take_profit_index,
                     signal_exit_index};
  }

  void sync_execution_position_state(this BacktestRunner& self)
  {
    if(!self.strategy_trade_session_.open_position() ||
       !self.execution_session_.open_position()) {
      return;
    }
    const auto& source = *self.strategy_trade_session_.open_position();
    auto& target = *self.execution_session_.open_position();
    target.stop_loss_levels(source.stop_loss_levels());
    target.take_profit_levels(source.take_profit_levels());
    target.signal_exit_states(source.signal_exit_states());
    target.risk_distance(source.risk_distance());
    target.risk_reference_price(source.risk_reference_price());
    target.risk_boundary_price(source.risk_boundary_price());
    self.execution_session_.sync_latest_event_with_open_position();
  }

  void observe_strategy_closure(this BacktestRunner& self)
  {
    if(!self.strategy_session_.closed_positions().empty()) {
      self.strategy_performance_.observe(
       self.strategy_session_.closed_positions().back());
    }
  }

  void record_strategy_exit(this BacktestRunner& self,
                            TradeExit::Reason reason,
                            double price,
                            double reduce,
                            std::optional<std::size_t> rule_index)
  {
    const auto type = [&] {
      switch(reason) {
      case TradeExit::Reason::stop_loss:
        return StrategyIntentType::StopLoss;
      case TradeExit::Reason::take_profit:
        return StrategyIntentType::TakeProfit;
      case TradeExit::Reason::signal:
      default:
        return StrategyIntentType::SignalExit;
      }
    }();
    self.strategy_session_.exit(type, price, reduce, rule_index);
    self.observe_strategy_closure();
  }

  void mirror_execution_exit(
   this BacktestRunner& self,
   double price,
   TradeExit::Reason reason,
   double reduce,
   std::optional<std::size_t> stop_loss_index = std::nullopt,
   std::optional<std::size_t> take_profit_index = std::nullopt,
   std::optional<std::size_t> signal_exit_index = std::nullopt)
  {
    if(!self.execution_strategy_trade_id_ ||
       !self.execution_session_.open_position()) {
      return;
    }
    auto exit = self.make_exit_trade(
     self.execution_session_.open_position()->position_size(),
     price,
     reason,
     reduce,
     stop_loss_index,
     take_profit_index,
     signal_exit_index);
    if(!exit) {
      return;
    }
    const auto fee = self.broker_.calculate_fee(*exit);
    self.execution_session_.exit_position(*exit, fee);
  }

  auto stop_is_eligible(this const BacktestRunner& self,
                        const TradePosition& position,
                        const PositionRule& rule,
                        std::size_t target) noexcept -> bool
  {
    if(target >= position.stop_loss_levels().size() ||
       !position.stop_loss_levels()[target].active()) {
      return false;
    }
    if(rule.stop_losses_activation() == ExitActivation::Simultaneous) {
      return true;
    }
    for(auto index = std::size_t{0}; index < rule.stop_losses().size();
        ++index) {
      if(index < position.stop_loss_levels().size() &&
         position.stop_loss_levels()[index].active()) {
        return index == target;
      }
    }
    return false;
  }

  auto take_profit_is_eligible(this const BacktestRunner& self,
                               const TradePosition& position,
                               const PositionRule& rule,
                               std::size_t target) noexcept -> bool
  {
    if(target >= position.take_profit_levels().size() ||
       !position.take_profit_levels()[target].active()) {
      return false;
    }
    if(rule.take_profits_activation() == ExitActivation::Simultaneous) {
      return true;
    }
    for(auto index = std::size_t{0}; index < rule.take_profits().size();
        ++index) {
      if(index < position.take_profit_levels().size() &&
         position.take_profit_levels()[index].active()) {
        return index == target;
      }
    }
    return false;
  }

  auto signal_exit_is_eligible(this const BacktestRunner& self,
                               const TradePosition& position,
                               const PositionRule& rule,
                               std::size_t target) noexcept -> bool
  {
    if(target >= position.signal_exit_states().size() ||
       !position.signal_exit_states()[target].active()) {
      return false;
    }
    if(rule.signal_exits_activation() == ExitActivation::Simultaneous) {
      return true;
    }
    for(auto index = std::size_t{0}; index < rule.signal_exits().size();
        ++index) {
      if(index < position.signal_exit_states().size() &&
         position.signal_exit_states()[index].active()) {
        return index == target;
      }
    }
    return false;
  }

  void activate_trailing_stops(this BacktestRunner& self, double current_price)
  {
    if(!self.strategy_trade_session_.open_position()) {
      return;
    }
    auto& position = *self.strategy_trade_session_.open_position();
    const auto& rule =
     position.is_long_direction() ? self.long_position_ : self.short_position_;
    for(auto index = std::size_t{0}; index < rule.stop_losses().size();
        ++index) {
      if(self.stop_is_eligible(position, rule, index) &&
         rule.stop_losses()[index].trailing()) {
        position.update_trailing_stop(index, current_price);
      }
    }
    self.sync_execution_position_state();
  }

  auto immediate_price_exit(this const BacktestRunner& self,
                            double current_price)
   -> std::optional<PriceExitCandidate>
  {
    if(!self.strategy_trade_session_.open_position()) {
      return std::nullopt;
    }
    const auto& position = *self.strategy_trade_session_.open_position();
    const auto& rule =
     position.is_long_direction() ? self.long_position_ : self.short_position_;
    auto result = std::optional<PriceExitCandidate>{};
    const auto consider = [&](PriceExitCandidate candidate) {
      if(!result || candidate.priority < result->priority ||
         (candidate.priority == result->priority &&
          candidate.index < result->index)) {
        result = candidate;
      }
    };

    for(auto index = std::size_t{0}; index < rule.stop_losses().size();
        ++index) {
      if(!self.stop_is_eligible(position, rule, index)) {
        continue;
      }
      const auto price = position.stop_loss_levels()[index].effective_price();
      const auto marketable = position.is_long_direction()
                               ? current_price <= price
                               : current_price >= price;
      if(marketable) {
        consider({TradeExit::Reason::stop_loss, index, current_price, 0});
      }
    }
    for(auto index = std::size_t{0}; index < rule.take_profits().size();
        ++index) {
      if(!self.take_profit_is_eligible(position, rule, index)) {
        continue;
      }
      const auto price = position.take_profit_levels()[index].price();
      const auto marketable = position.is_long_direction()
                               ? current_price >= price
                               : current_price <= price;
      if(marketable) {
        consider({TradeExit::Reason::take_profit, index, current_price, 1});
      }
    }
    return result;
  }

  auto next_segment_exit(this const BacktestRunner& self,
                         double start,
                         double end) -> std::optional<PriceExitCandidate>
  {
    if(!self.strategy_trade_session_.open_position() || start == end) {
      return std::nullopt;
    }
    const auto& position = *self.strategy_trade_session_.open_position();
    const auto& rule =
     position.is_long_direction() ? self.long_position_ : self.short_position_;
    const auto rising = end > start;
    auto result = std::optional<PriceExitCandidate>{};
    const auto consider = [&](PriceExitCandidate candidate) {
      const auto distance = std::abs(candidate.price - start);
      const auto result_distance = result
                                    ? std::abs(result->price - start)
                                    : std::numeric_limits<double>::infinity();
      if(!result || distance < result_distance ||
         (distance == result_distance &&
          (candidate.priority < result->priority ||
           (candidate.priority == result->priority &&
            candidate.index < result->index)))) {
        result = candidate;
      }
    };

    for(auto index = std::size_t{0}; index < rule.stop_losses().size();
        ++index) {
      if(!self.stop_is_eligible(position, rule, index)) {
        continue;
      }
      const auto price = position.stop_loss_levels()[index].effective_price();
      const auto direction_matches =
       position.is_long_direction() ? !rising : rising;
      if(direction_matches && price >= std::min(start, end) &&
         price <= std::max(start, end)) {
        consider({TradeExit::Reason::stop_loss, index, price, 0});
      }
    }
    for(auto index = std::size_t{0}; index < rule.take_profits().size();
        ++index) {
      if(!self.take_profit_is_eligible(position, rule, index)) {
        continue;
      }
      const auto price = position.take_profit_levels()[index].price();
      const auto direction_matches =
       position.is_long_direction() ? rising : !rising;
      if(direction_matches && price >= std::min(start, end) &&
         price <= std::max(start, end)) {
        consider({TradeExit::Reason::take_profit, index, price, 1});
      }
    }
    return result;
  }

  auto execute_price_exit(this BacktestRunner& self,
                          const PriceExitCandidate& candidate,
                          std::optional<bool>& closed_position_is_long) -> bool
  {
    if(!self.strategy_trade_session_.open_position()) {
      return false;
    }
    const auto& position = *self.strategy_trade_session_.open_position();
    const auto was_long = position.is_long_direction();
    const auto& rule = was_long ? self.long_position_ : self.short_position_;
    const auto reduce = candidate.reason == TradeExit::Reason::stop_loss
                         ? rule.stop_losses()[candidate.index].reduce()
                         : rule.take_profits()[candidate.index].reduce();
    const auto stop_index = candidate.reason == TradeExit::Reason::stop_loss
                             ? std::optional{candidate.index}
                             : std::nullopt;
    const auto target_index = candidate.reason == TradeExit::Reason::take_profit
                               ? std::optional{candidate.index}
                               : std::nullopt;
    auto exit = self.make_strategy_exit_trade(position.position_size(),
                                              candidate.price,
                                              candidate.reason,
                                              reduce,
                                              stop_index,
                                              target_index);
    if(!exit) {
      return false;
    }
    self.strategy_trade_session_.exit_position(*exit);
    self.record_strategy_exit(
     candidate.reason, candidate.price, reduce, candidate.index);
    self.mirror_execution_exit(
     candidate.price, candidate.reason, reduce, stop_index, target_index);
    if(self.strategy_trade_session_.is_flat()) {
      closed_position_is_long = was_long;
      self.pyramiding_layers_ = 0;
      self.pyramiding_signal_ready_ = true;
      self.pyramiding_resume_index_.reset();
      self.execution_strategy_trade_id_.reset();
    }
    return true;
  }

  auto process_open_price(this BacktestRunner& self,
                          double open,
                          std::optional<bool>& closed_position_is_long) -> bool
  {
    auto filled = false;
    while(const auto candidate = self.immediate_price_exit(open)) {
      filled =
       self.execute_price_exit(*candidate, closed_position_is_long) || filled;
      if(self.strategy_trade_session_.is_flat()) {
        return filled;
      }
    }
    self.activate_trailing_stops(open);
    return filled;
  }

  void process_price_segment(this BacktestRunner& self,
                             double start,
                             double end,
                             std::optional<bool>& closed_position_is_long)
  {
    auto current = start;
    while(self.strategy_trade_session_.is_open()) {
      self.activate_trailing_stops(current);
      if(const auto immediate = self.immediate_price_exit(current)) {
        self.execute_price_exit(*immediate, closed_position_is_long);
        continue;
      }
      const auto candidate = self.next_segment_exit(current, end);
      if(!candidate) {
        self.activate_trailing_stops(end);
        return;
      }
      self.activate_trailing_stops(candidate->price);
      current = candidate->price;
      self.execute_price_exit(*candidate, closed_position_is_long);
    }
  }

  auto execute_signal_exit_indices(this BacktestRunner& self,
                                   const std::vector<std::size_t>& indices,
                                   double price,
                                   std::optional<bool>& closed_position_is_long)
   -> bool
  {
    auto filled = false;
    for(const auto index : indices) {
      if(!self.strategy_trade_session_.open_position()) {
        break;
      }
      const auto& position = *self.strategy_trade_session_.open_position();
      const auto was_long = position.is_long_direction();
      const auto& rule = was_long ? self.long_position_ : self.short_position_;
      if(index >= rule.signal_exits().size() ||
         !self.signal_exit_is_eligible(position, rule, index)) {
        continue;
      }
      const auto reduce = rule.signal_exits()[index].reduce();
      auto exit = self.make_strategy_exit_trade(position.position_size(),
                                                price,
                                                TradeExit::Reason::signal,
                                                reduce,
                                                std::nullopt,
                                                std::nullopt,
                                                index);
      if(!exit) {
        continue;
      }
      self.strategy_trade_session_.exit_position(*exit);
      self.record_strategy_exit(
       TradeExit::Reason::signal, price, reduce, index);
      self.mirror_execution_exit(price,
                                 TradeExit::Reason::signal,
                                 reduce,
                                 std::nullopt,
                                 std::nullopt,
                                 index);
      filled = true;
      if(self.strategy_trade_session_.is_flat()) {
        closed_position_is_long = was_long;
        self.pyramiding_layers_ = 0;
        self.pyramiding_signal_ready_ = true;
        self.pyramiding_resume_index_.reset();
        self.execution_strategy_trade_id_.reset();
      }
    }
    return filled;
  }

  auto
  execute_pending_signal_exits(this BacktestRunner& self,
                               double price,
                               std::optional<bool>& closed_position_is_long)
   -> bool
  {
    auto valid = self.strategy_trade_session_.open_position() &&
                 self.pending_signal_exit_trade_id_ &&
                 self.strategy_trade_session_.open_position()->trade_id() ==
                  *self.pending_signal_exit_trade_id_;
    auto filled = false;
    if(valid) {
      filled = self.execute_signal_exit_indices(
       self.pending_signal_exit_indices_, price, closed_position_is_long);
    }
    self.pending_signal_exit_indices_.clear();
    self.pending_signal_exit_trade_id_.reset();
    return filled;
  }

  auto execute_signal_exits(this BacktestRunner& self,
                            SignalTiming timing,
                            double price,
                            const AssetSnapshot& snapshot,
                            MethodContextable auto context,
                            std::optional<bool>& closed_position_is_long)
   -> bool
  {
    if(!self.strategy_trade_session_.open_position()) {
      return false;
    }
    const auto& position = *self.strategy_trade_session_.open_position();
    const auto& rule =
     position.is_long_direction() ? self.long_position_ : self.short_position_;
    const auto position_context =
     self.with_open_position_context(context, position);
    auto indices = std::vector<std::size_t>{};
    for(auto index = std::size_t{0}; index < rule.signal_exits().size();
        ++index) {
      const auto& signal_exit = rule.signal_exits()[index];
      if(signal_exit.timing() == timing &&
         index < position.signal_exit_states().size() &&
         position.signal_exit_states()[index].enabled() &&
         !position.signal_exit_states()[index].consumed() &&
         static_cast<bool>(evaluate_series_method(
          signal_exit.signal_method(), snapshot, position_context))) {
        indices.push_back(index);
      }
    }
    return self.execute_signal_exit_indices(
     indices, price, closed_position_is_long);
  }

  auto execute_entry_action(this BacktestRunner& self,
                            bool is_long,
                            double price,
                            const AssetSnapshot& evaluation_snapshot,
                            MethodContextable auto context,
                            double current_drawdown_ratio) -> bool
  {
    const auto& rule = is_long ? self.long_position_ : self.short_position_;
    self.pyramiding_signal_ready_ = true;
    const auto strategy_entry = TradeEntry{is_long ? 1.0 : -1.0, price};
    self.strategy_trade_session_.entry_position(strategy_entry);
    auto& position = *self.strategy_trade_session_.open_position();
    self.update_stop_target_prices(
     position, strategy_entry, rule, false, evaluation_snapshot, context);
    self.strategy_trade_session_.sync_latest_event_with_open_position();
    self.pyramiding_layers_ = 1;
    self.restart_pyramiding_cooldown(rule);

    const auto direction =
     is_long ? StrategyDirection::Long : StrategyDirection::Short;
    const auto& intent = self.strategy_session_.enter(direction, price, false);
    const auto performance_snapshot = self.strategy_performance_.snapshot();
    auto sizing_decision =
     PositionSizingDecision{.intent_id = intent.intent_id(),
                            .strategy_trade_id = intent.strategy_trade_id(),
                            .direction = direction,
                            .pyramiding = false,
                            .method = std::string{self.position_sizing_.name()},
                            .entry_price = price,
                            .outcome = PositionSizingDecisionOutcome::Filtered};
    auto filter_context =
     ExecutionFilterMethodContext{context, performance_snapshot};
    const auto allowed = static_cast<bool>(evaluate_series_method(
     self.execution_filter_, evaluation_snapshot, filter_context));
    self.execution_filter_decisions_.emplace_back(intent.intent_id(), allowed);
    if(!allowed) {
      self.execution_strategy_trade_id_.reset();
      self.position_sizing_decisions_.push_back(std::move(sizing_decision));
      return true;
    }

    const auto sizing_request = self.create_trade(rule,
                                                  is_long,
                                                  price,
                                                  evaluation_snapshot,
                                                  context,
                                                  current_drawdown_ratio,
                                                  performance_snapshot,
                                                  sizing_decision);
    const auto entry =
     sizing_request ? self.prepare_entry_order(*sizing_request, sizing_decision)
                    : std::nullopt;
    if(!entry) {
      self.execution_strategy_trade_id_.reset();
      self.position_sizing_decisions_.push_back(std::move(sizing_decision));
      return true;
    }
    const auto fee = self.broker_.calculate_fee(*entry);
    self.execution_session_.entry_position(*entry, fee);
    self.execution_strategy_trade_id_ = intent.strategy_trade_id();
    self.sync_execution_position_state();
    self.position_sizing_decisions_.push_back(std::move(sizing_decision));
    return true;
  }

  auto execute_pyramiding_action(this BacktestRunner& self,
                                 double price,
                                 const AssetSnapshot& evaluation_snapshot,
                                 MethodContextable auto context,
                                 double current_drawdown_ratio) -> bool
  {
    if(!self.strategy_trade_session_.open_position()) {
      return false;
    }
    const auto is_long =
     self.strategy_trade_session_.open_position()->is_long_direction();
    const auto& rule = is_long ? self.long_position_ : self.short_position_;
    if(self.pyramiding_layers_ >= rule.pyramiding_max_layers()) {
      return false;
    }
    const auto strategy_entry = TradeEntry{is_long ? 1.0 : -1.0, price};
    self.strategy_trade_session_.entry_position(strategy_entry);
    auto& strategy_position = *self.strategy_trade_session_.open_position();
    self.update_stop_target_prices(strategy_position,
                                   strategy_entry,
                                   rule,
                                   true,
                                   evaluation_snapshot,
                                   context);
    self.strategy_trade_session_.sync_latest_event_with_open_position();
    const auto direction =
     is_long ? StrategyDirection::Long : StrategyDirection::Short;
    const auto& intent = self.strategy_session_.enter(direction, price, true);
    const auto performance_snapshot = self.strategy_performance_.snapshot();
    auto sizing_decision = PositionSizingDecision{
     .intent_id = intent.intent_id(),
     .strategy_trade_id = intent.strategy_trade_id(),
     .direction = direction,
     .pyramiding = true,
     .method = std::string{self.position_sizing_.name()},
     .entry_price = price,
     .outcome = PositionSizingDecisionOutcome::ShadowOnly};

    ++self.pyramiding_layers_;
    self.restart_pyramiding_cooldown(rule);

    if(!self.execution_strategy_trade_id_ ||
       !self.execution_session_.open_position()) {
      self.position_sizing_decisions_.push_back(std::move(sizing_decision));
      return true;
    }
    const auto sizing_request = self.create_trade(rule,
                                                  is_long,
                                                  price,
                                                  evaluation_snapshot,
                                                  context,
                                                  current_drawdown_ratio,
                                                  performance_snapshot,
                                                  sizing_decision);
    const auto entry =
     sizing_request ? self.prepare_entry_order(*sizing_request, sizing_decision)
                    : std::nullopt;
    if(!entry) {
      self.position_sizing_decisions_.push_back(std::move(sizing_decision));
      return true;
    }
    const auto fee = self.broker_.calculate_fee(*entry);
    self.execution_session_.entry_position(*entry, fee);
    self.sync_execution_position_state();
    self.position_sizing_decisions_.push_back(std::move(sizing_decision));
    return true;
  }

  void execute_pending_entry(this BacktestRunner& self,
                             double price,
                             const AssetSnapshot& evaluation_snapshot,
                             MethodContextable auto context,
                             double current_drawdown_ratio,
                             std::optional<bool> closed_position_is_long)
  {
    if(self.pending_entries_[0] &&
       (!closed_position_is_long || !*closed_position_is_long) &&
       self.execute_entry_action(
        true, price, evaluation_snapshot, context, current_drawdown_ratio)) {
      return;
    }
    if(self.pending_entries_[1] &&
       (!closed_position_is_long || *closed_position_is_long)) {
      self.execute_entry_action(
       false, price, evaluation_snapshot, context, current_drawdown_ratio);
    }
  }

  void execute_current_close_entry(this BacktestRunner& self,
                                   double price,
                                   const AssetSnapshot& snapshot,
                                   MethodContextable auto context,
                                   double current_drawdown_ratio,
                                   std::optional<bool> closed_position_is_long)
  {
    const auto can_enter = [&](const PositionRule& rule) {
      return rule.entry_timing() == SignalTiming::CurrentClose &&
             static_cast<bool>(
              evaluate_series_method(rule.entry_method(), snapshot, context));
    };
    if((!closed_position_is_long || !*closed_position_is_long) &&
       can_enter(self.long_position_) &&
       self.execute_entry_action(
        true, price, snapshot, context, current_drawdown_ratio)) {
      return;
    }
    if((!closed_position_is_long || *closed_position_is_long) &&
       can_enter(self.short_position_)) {
      self.execute_entry_action(
       false, price, snapshot, context, current_drawdown_ratio);
    }
  }

  void schedule_next_open_actions(this BacktestRunner& self,
                                  const AssetSnapshot& snapshot,
                                  MethodContextable auto context)
  {
    self.pending_entries_[0] =
     self.long_position_.entry_timing() == SignalTiming::NextOpen &&
     static_cast<bool>(evaluate_series_method(
      self.long_position_.entry_method(), snapshot, context));
    self.pending_entries_[1] =
     self.short_position_.entry_timing() == SignalTiming::NextOpen &&
     static_cast<bool>(evaluate_series_method(
      self.short_position_.entry_method(), snapshot, context));

    self.pending_pyramiding_ = false;
    self.pending_signal_exit_indices_.clear();
    self.pending_signal_exit_trade_id_.reset();
    if(!self.strategy_trade_session_.open_position()) {
      self.pyramiding_signal_ready_ = true;
      self.pyramiding_resume_index_.reset();
      return;
    }
    const auto& position = *self.strategy_trade_session_.open_position();
    const auto& rule =
     position.is_long_direction() ? self.long_position_ : self.short_position_;
    const auto position_context =
     self.with_open_position_context(context, position);
    self.pending_pyramiding_ =
     rule.pyramiding_timing() == SignalTiming::NextOpen &&
     !self.pyramiding_is_paused() &&
     self.pyramiding_signal_triggered(rule, snapshot, position_context);
    for(auto index = std::size_t{0}; index < rule.signal_exits().size();
        ++index) {
      const auto& signal_exit = rule.signal_exits()[index];
      if(signal_exit.timing() == SignalTiming::NextOpen &&
         index < position.signal_exit_states().size() &&
         position.signal_exit_states()[index].enabled() &&
         !position.signal_exit_states()[index].consumed() &&
         static_cast<bool>(evaluate_series_method(
          signal_exit.signal_method(), snapshot, position_context))) {
        self.pending_signal_exit_indices_.push_back(index);
      }
    }
    if(!self.pending_signal_exit_indices_.empty()) {
      self.pending_signal_exit_trade_id_ = position.trade_id();
    }
  }
};

} // namespace pludux::backtest
