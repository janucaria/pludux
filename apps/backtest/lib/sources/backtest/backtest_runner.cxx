module;

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <format>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

export module pludux.backtest:backtest_runner;

import pludux;

import :store;
import :asset;
import :profile;
import :trade_entry;
import :trade_exit;
import :take_profit_level;
import :signal_exit_state;
import :closed_trade;
import :open_position_snapshot;
import :trade_event;
import :trade_position;
import :trade_session;
import :backtest_timeline;
import :backtest_method_context;
import :broker;
import :market;
import :backtest;

export namespace pludux::backtest {

class BacktestRunner {
public:
  class PositionRule {
  public:
    class SignalExitRule {
    public:
      SignalExitRule(bool enabled = false,
                     AnySeriesMethod signal_method = BooleanMethod<false>{},
                     std::size_t signal_delay = 1,
                     AnySeriesMethod price_method = OpenMethod{},
                     double reduce = 1.0)
      : enabled_{enabled}
      , signal_method_{std::move(signal_method)}
      , signal_delay_{signal_delay}
      , price_method_{std::move(price_method)}
      , reduce_{reduce}
      {
      }

      auto enabled(this const SignalExitRule& self) noexcept -> bool
      {
        return self.enabled_;
      }

      auto signal_method(this const SignalExitRule& self) noexcept
       -> const AnySeriesMethod&
      {
        return self.signal_method_;
      }

      auto signal_delay(this const SignalExitRule& self) noexcept -> std::size_t
      {
        return self.signal_delay_;
      }

      auto price_method(this const SignalExitRule& self) noexcept
       -> const AnySeriesMethod&
      {
        return self.price_method_;
      }

      auto reduce(this const SignalExitRule& self) noexcept -> double
      {
        return self.reduce_;
      }

    private:
      bool enabled_;
      AnySeriesMethod signal_method_;
      std::size_t signal_delay_;
      AnySeriesMethod price_method_;
      double reduce_;
    };

    class TakeProfitRule {
    public:
      TakeProfitRule(AnySeriesMethod price_method = OpenMethod{},
                     bool enabled = false,
                     double reduce = 1.0)
      : price_method_{std::move(price_method)}
      , enabled_{enabled}
      , reduce_{reduce}
      {
      }

      auto price_method(this const TakeProfitRule& self) noexcept
       -> const AnySeriesMethod&
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
      AnySeriesMethod price_method_;
      bool enabled_;
      double reduce_;
    };

    PositionRule() = default;

    PositionRule(AnySeriesMethod entry_method,
                 std::vector<SignalExitRule> signal_exits,
                 AnySeriesMethod pyramiding_signal,
                 std::size_t pyramiding_max_layers,
                 AnySeriesMethod risk_distance_method,
                 AnySeriesMethod stop_price_method,
                 bool stop_loss_enabled,
                 bool stop_loss_trailing_enabled,
                 std::size_t entry_signal_delay = 1,
                 AnySeriesMethod entry_price_method = OpenMethod{},
                 std::size_t pyramiding_signal_delay = 1,
                 AnySeriesMethod pyramiding_price_method = OpenMethod{},
                 StopTargetReferencePrice favorable_stop_target_reference =
                  StopTargetReferencePrice::AveragePrice,
                 StopTargetReferencePrice unfavorable_stop_target_reference =
                  StopTargetReferencePrice::AveragePrice,
                 double stop_loss_reduce = 1.0,
                 std::vector<TakeProfitRule> take_profits = {})
    : entry_method_{std::move(entry_method)}
    , signal_exits_{std::move(signal_exits)}
    , pyramiding_signal_{std::move(pyramiding_signal)}
    , pyramiding_max_layers_{pyramiding_max_layers}
    , risk_distance_method_{std::move(risk_distance_method)}
    , stop_price_method_{std::move(stop_price_method)}
    , stop_loss_enabled_{stop_loss_enabled}
    , stop_loss_trailing_enabled_{stop_loss_trailing_enabled}
    , entry_signal_delay_{entry_signal_delay}
    , entry_price_method_{std::move(entry_price_method)}
    , pyramiding_signal_delay_{pyramiding_signal_delay}
    , pyramiding_price_method_{std::move(pyramiding_price_method)}
    , favorable_stop_target_reference_{favorable_stop_target_reference}
    , unfavorable_stop_target_reference_{unfavorable_stop_target_reference}
    , stop_loss_reduce_{stop_loss_reduce}
    , take_profits_{std::move(take_profits)}
    {
    }

    auto entry_method(this const PositionRule& self) noexcept
     -> const AnySeriesMethod&
    {
      return self.entry_method_;
    }

    auto signal_exits(this const PositionRule& self) noexcept
     -> const std::vector<SignalExitRule>&
    {
      return self.signal_exits_;
    }

    auto pyramiding_signal(this const PositionRule& self) noexcept
     -> const AnySeriesMethod&
    {
      return self.pyramiding_signal_;
    }

    auto pyramiding_max_layers(this const PositionRule& self) noexcept
     -> std::size_t
    {
      return self.pyramiding_max_layers_;
    }

    auto stop_price_method(this const PositionRule& self) noexcept
     -> const AnySeriesMethod&
    {
      return self.stop_price_method_;
    }

    auto risk_distance_method(this const PositionRule& self) noexcept
     -> const AnySeriesMethod&
    {
      return self.risk_distance_method_;
    }

    auto stop_loss_enabled(this const PositionRule& self) noexcept -> bool
    {
      return self.stop_loss_enabled_;
    }

    auto stop_loss_trailing_enabled(this const PositionRule& self) noexcept
     -> bool
    {
      return self.stop_loss_trailing_enabled_;
    }

    auto entry_signal_delay(this const PositionRule& self) noexcept
     -> std::size_t
    {
      return self.entry_signal_delay_;
    }

    auto entry_price_method(this const PositionRule& self) noexcept
     -> const AnySeriesMethod&
    {
      return self.entry_price_method_;
    }

    auto pyramiding_signal_delay(this const PositionRule& self) noexcept
     -> std::size_t
    {
      return self.pyramiding_signal_delay_;
    }

    auto pyramiding_price_method(this const PositionRule& self) noexcept
     -> const AnySeriesMethod&
    {
      return self.pyramiding_price_method_;
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

    auto stop_loss_reduce(this const PositionRule& self) noexcept -> double
    {
      return self.stop_loss_reduce_;
    }

    auto take_profits(this const PositionRule& self) noexcept
     -> const std::vector<TakeProfitRule>&
    {
      return self.take_profits_;
    }

  private:
    AnySeriesMethod entry_method_{BooleanMethod<false>{}};
    std::vector<SignalExitRule> signal_exits_;
    AnySeriesMethod pyramiding_signal_{BooleanMethod<false>{}};
    std::size_t pyramiding_max_layers_{1};
    AnySeriesMethod risk_distance_method_{ValueMethod{1.0}};
    AnySeriesMethod stop_price_method_{OpenMethod{}};
    bool stop_loss_enabled_{false};
    bool stop_loss_trailing_enabled_{false};
    std::size_t entry_signal_delay_{1};
    AnySeriesMethod entry_price_method_{OpenMethod{}};
    std::size_t pyramiding_signal_delay_{1};
    AnySeriesMethod pyramiding_price_method_{OpenMethod{}};
    StopTargetReferencePrice favorable_stop_target_reference_{
     StopTargetReferencePrice::AveragePrice};
    StopTargetReferencePrice unfavorable_stop_target_reference_{
     StopTargetReferencePrice::AveragePrice};
    double stop_loss_reduce_{1.0};
    std::vector<TakeProfitRule> take_profits_;
  };

  BacktestRunner(const Asset& asset,
                 const Market& market,
                 const Broker& broker,
                 const Profile& profile,
                 OrderedNamedRegistry<AnySeriesMethod> series_methods,
                 PositionRule long_position,
                 PositionRule short_position,
                 double total_equity = 0.0,
                 std::size_t pyramiding_layers = 0,
                 bool is_failed = false,
                 double peak_equity = NAN)
  : asset_{asset}
  , market_{market}
  , broker_{broker}
  , profile_{profile}
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
  , trade_session_{}
  , series_methods_{std::move(series_methods)}
  , long_position_{std::move(long_position)}
  , short_position_{std::move(short_position)}
  , pyramiding_layers_{pyramiding_layers}
  , is_failed_{is_failed}
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

  void run(this BacktestRunner& self,
           SeriesEvaluationResults& series_evaluation_results,
           BacktestTimeline& timeline)
  {
    if(self.is_failed()) {
      return;
    }

    const auto timeline_size = timeline.size();
    const auto asset_size = self.asset_.size();
    if(timeline_size >= asset_size) {
      return;
    }

    const auto asset_snapshot = self.asset_.get_snapshot(timeline_size);

    const auto& series_methods = self.series_methods_;

    self.trade_session_.begin_market_bar(
     static_cast<std::time_t>(asset_snapshot.datetime()),
     asset_snapshot.close(),
     asset_snapshot.lookback());

    self.current_account_state_.unrealized_pnl(
     self.trade_session_.unrealized_pnl());

    auto default_context = DefaultMethodContext{
     series_methods, series_evaluation_results, timeline_size};

    auto context = BacktestMethodContext{
     std::move(default_context), series_methods, self.current_account_state_};

    const auto current_drawdown_ratio =
     self.current_account_state_.drawdown_ratio();

    auto closed_position_is_long = std::optional<bool>{};

    if(auto& open_position = self.trade_session_.open_position()) {
      auto exit_was_triggered = false;
      const auto exit_trade =
       self.exit_trade(asset_snapshot, *open_position, context);

      if(exit_trade) {
        const auto fee = self.broker_.calculate_fee(*exit_trade);
        const auto closing_position_is_long =
         open_position->is_long_direction();
        self.trade_session_.exit_position(*exit_trade, fee);
        self.mark_exit_consumed(exit_trade->reason());

        if(!self.trade_session_.is_open()) {
          closed_position_is_long = closing_position_is_long;
          self.pyramiding_layers_ = 0;
        }
        exit_was_triggered = true;
      }

      if(!exit_was_triggered) {
        const auto is_long_direction = open_position->is_long_direction();
        if(is_long_direction) {
          auto pyramiding_trade = self.pyramiding_long_trade(
           asset_snapshot, context, current_drawdown_ratio);
          if(pyramiding_trade && self.prepare_entry_order(*pyramiding_trade)) {
            const auto fee = self.broker_.calculate_fee(*pyramiding_trade);
            self.trade_session_.entry_position(*pyramiding_trade, fee);
            if(auto& updated_open_position =
                self.trade_session_.open_position()) {
              self.update_stop_target_prices(*updated_open_position,
                                             *pyramiding_trade,
                                             self.long_position_,
                                             true,
                                             asset_snapshot,
                                             context);
              self.trade_session_.sync_latest_event_with_open_position();
            }
            self.pyramiding_layers_++;
          }
        } else {
          auto pyramiding_trade = self.pyramiding_short_trade(
           asset_snapshot, context, current_drawdown_ratio);
          if(pyramiding_trade && self.prepare_entry_order(*pyramiding_trade)) {
            const auto fee = self.broker_.calculate_fee(*pyramiding_trade);
            self.trade_session_.entry_position(*pyramiding_trade, fee);
            if(auto& updated_open_position =
                self.trade_session_.open_position()) {
              self.update_stop_target_prices(*updated_open_position,
                                             *pyramiding_trade,
                                             self.short_position_,
                                             true,
                                             asset_snapshot,
                                             context);
              self.trade_session_.sync_latest_event_with_open_position();
            }
            self.pyramiding_layers_++;
          }
        }
      }
    }

    if(self.trade_session_.is_flat()) {
      auto entry_trade = self.entry_trade(asset_snapshot,
                                          context,
                                          current_drawdown_ratio,
                                          closed_position_is_long);
      if(entry_trade && self.prepare_entry_order(*entry_trade)) {
        const auto fee = self.broker_.calculate_fee(*entry_trade);
        self.trade_session_.entry_position(*entry_trade, fee);
        if(auto& open_position = self.trade_session_.open_position()) {
          self.update_stop_target_prices(*open_position,
                                         *entry_trade,
                                         entry_trade->is_long_direction()
                                          ? self.long_position_
                                          : self.short_position_,
                                         false,
                                         asset_snapshot,
                                         context);
          self.trade_session_.sync_latest_event_with_open_position();
        }
        self.pyramiding_layers_ = 1;
        self.reset_consumed_exits();
      }
    }

    self.update_accounting();

    timeline.append(BacktestTimeline::Row{
     .market_timestamp = self.trade_session_.market_timestamp(),
     .market_price = self.trade_session_.market_price(),
     .market_lookback = self.trade_session_.market_lookback(),
     .trade_events = self.trade_session_.trade_events(),
     .closed_trades = self.trade_session_.closed_trades(),
     .open_position = self.trade_session_.open_position_snapshot(),
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
     .open_trade_count = self.trade_session_.is_open() ? 1U : 0U,
     .unrealized_pnl = self.trade_session_.unrealized_pnl(),
     .unrealized_investment = self.trade_session_.unrealized_investment(),
     .unrealized_duration = self.trade_session_.unrealized_duration()});

    for(const auto& [series_name, series_method] : series_methods) {
      const auto series_value =
       evaluate_series_method(series_method, asset_snapshot, context);
      series_evaluation_results.put(series_method, series_value);
      series_evaluation_results.alias(series_name, series_method);
    }
  }

private:
  const Asset& asset_;
  const Market& market_;
  const Broker& broker_;
  const Profile& profile_;

  BacktestAccountState current_account_state_;
  double max_drawdown_;

  std::time_t sum_of_durations_;
  double cumulative_investments_;

  std::size_t profit_count_;
  double cumulative_profits_;

  std::size_t loss_count_;
  double cumulative_losses_;

  std::size_t break_even_count_;

  TradeSession trade_session_;

  OrderedNamedRegistry<AnySeriesMethod> series_methods_;

  PositionRule long_position_;
  PositionRule short_position_;

  std::size_t pyramiding_layers_;

  bool is_failed_;
  bool stop_loss_consumed_{};

  void reset_consumed_exits(this BacktestRunner& self) noexcept
  {
    self.stop_loss_consumed_ = false;
  }

  void mark_exit_consumed(this BacktestRunner& self,
                          TradeExit::Reason reason) noexcept
  {
    switch(reason) {
    case TradeExit::Reason::signal:
      break;
    case TradeExit::Reason::stop_loss:
      self.stop_loss_consumed_ = true;
      break;
    case TradeExit::Reason::take_profit:
      break;
    }
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
                     std::abs(self.trade_session_.unrealized_investment()),
                    0.0);
  }

  auto required_cash(this const BacktestRunner&,
                     const TradeEntry& entry,
                     double fee) noexcept -> double
  {
    return std::abs(entry.position_size() * entry.price()) + fee;
  }

  void normalize_order_size(this const BacktestRunner& self,
                            TradeEntry& entry) noexcept
  {
    const auto quantity_step = self.market_.quantity_step();
    const auto min_order_quantity = self.market_.min_order_quantity();

    auto position_size = entry.position_size();
    if(quantity_step > 0.0 && std::fmod(position_size, quantity_step) != 0.0) {
      position_size = quantity_step * std::round(position_size / quantity_step);
    }

    if(position_size > 0.0 && position_size < min_order_quantity) {
      position_size = min_order_quantity;
    } else if(position_size < 0.0 && position_size > -min_order_quantity) {
      position_size = -min_order_quantity;
    }

    entry.position_size(position_size);
  }

  auto cap_order_to_available_cash(this const BacktestRunner& self,
                                   const TradeEntry& entry,
                                   double available_cash)
   -> std::optional<TradeEntry>
  {
    const auto direction = entry.position_size() >= 0.0 ? 1.0 : -1.0;
    const auto intended_quantity = std::abs(entry.position_size());
    const auto min_order_quantity = self.market_.min_order_quantity();
    const auto quantity_step = self.market_.quantity_step();

    if(intended_quantity <= 0.0 || available_cash <= 0.0) {
      return std::nullopt;
    }

    auto low = 0.0;
    auto high = intended_quantity;
    for(auto i = 0; i < 64; ++i) {
      const auto mid = (low + high) / 2.0;
      auto candidate = entry;
      candidate.position_size(direction * mid);
      const auto fee = self.broker_.calculate_fee(candidate);
      if(self.required_cash(candidate, fee) <= available_cash) {
        low = mid;
      } else {
        high = mid;
      }
    }

    auto capped_quantity = low;
    if(quantity_step > 0.0) {
      capped_quantity =
       quantity_step * std::floor(capped_quantity / quantity_step);
    }

    if(capped_quantity <= 0.0 ||
       (min_order_quantity > 0.0 && capped_quantity < min_order_quantity)) {
      return std::nullopt;
    }

    auto capped_entry = entry;
    capped_entry.position_size(direction * capped_quantity);
    const auto capped_fee = self.broker_.calculate_fee(capped_entry);
    if(self.required_cash(capped_entry, capped_fee) > available_cash) {
      return std::nullopt;
    }

    return capped_entry;
  }

  auto prepare_entry_order(this BacktestRunner& self, TradeEntry& entry) -> bool
  {
    self.normalize_order_size(entry);

    const auto fee = self.broker_.calculate_fee(entry);
    const auto cash = self.available_cash();
    if(self.required_cash(entry, fee) <= cash) {
      return true;
    }

    switch(self.profile_.insufficient_cash_policy()) {
    case InsufficientCashPolicy::Reject:
      self.trade_session_.reject_insufficient_cash(entry);
      return false;
    case InsufficientCashPolicy::CapToAvailableCash:
      if(auto capped_entry = self.cap_order_to_available_cash(entry, cash)) {
        entry = *capped_entry;
        return true;
      }
      return false;
    }

    return false;
  }

  void update_accounting(this BacktestRunner& self) noexcept
  {
    for(const auto& realized_exit : self.trade_session_.realized_exits()) {
      self.current_account_state_.capital(
       self.current_account_state_.capital() + realized_exit.pnl());
    }

    for(const auto& closed_trade : self.trade_session_.closed_trades()) {
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

    self.current_account_state_.unrealized_pnl(
     self.trade_session_.unrealized_pnl());
    self.current_account_state_.update_peak_to_current_equity();
    self.max_drawdown_ = std::max(self.max_drawdown_, self.current_drawdown());
  }

  auto create_trade(this const BacktestRunner& self,
                    const PositionRule& position,
                    bool is_long,
                    bool is_pyramiding,
                    const AssetSnapshot& asset_snapshot,
                    MethodContextable auto context,
                    double current_drawdown_ratio) -> std::optional<TradeEntry>
  {
    auto result = std::optional<TradeEntry>{};

    const auto& signal =
     is_pyramiding ? position.pyramiding_signal() : position.entry_method();
    const auto signal_delay = is_pyramiding ? position.pyramiding_signal_delay()
                                            : position.entry_signal_delay();
    const auto& price_method = is_pyramiding
                                ? position.pyramiding_price_method()
                                : position.entry_price_method();
    const auto signal_snapshot = asset_snapshot[signal_delay];
    const auto can_enter = static_cast<bool>(
     evaluate_series_method(signal, signal_snapshot, context));
    const auto can_pyramid =
     !is_pyramiding ||
     self.pyramiding_layers_ < position.pyramiding_max_layers();

    if(can_enter && can_pyramid) {
      const auto entry_price =
       evaluate_series_method(price_method, asset_snapshot, context);
      const auto direction = is_long ? 1.0 : -1.0;
      const auto initial_price_context =
       context.with_position_reference(entry_price, direction);
      const auto position_sizing = self.profile_.position_sizing();
      const auto uses_risk_distance =
       position_sizing.mode() == PositionSizing::Mode::RiskDistance;
      const auto risk_distance = evaluate_series_method(
       position.risk_distance_method(), asset_snapshot, initial_price_context);
      if(!std::isfinite(risk_distance) || risk_distance <= 0.0) {
        throw std::runtime_error{
         "Invalid risk distance: expected a finite positive value"};
      }
      const auto position_quantity = self.calculate_position_quantity(
       position_sizing, entry_price, risk_distance);
      const auto adjusted_position_quantity = self.apply_drawdown_adjustment(
       position_quantity, current_drawdown_ratio);
      const auto position_size = direction * adjusted_position_quantity;

      result = TradeEntry{position_size, entry_price};
    }

    return result;
  }

  auto apply_drawdown_adjustment(this const BacktestRunner& self,
                                 double position_quantity,
                                 double current_drawdown_ratio) -> double
  {
    const auto& drawdown_adjustment = self.profile_.drawdown_adjustment();

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

  auto calculate_position_quantity(this const BacktestRunner& self,
                                   const PositionSizing& position_sizing,
                                   double entry_price,
                                   double risk_distance) -> double
  {
    const auto value = position_sizing.value();

    if(!std::isfinite(value)) {
      throw std::runtime_error{"Invalid position sizing value"};
    }

    switch(position_sizing.mode()) {
    case PositionSizing::Mode::RiskDistance: {
      if(!std::isfinite(risk_distance) || risk_distance <= 0.0) {
        throw std::runtime_error{
         "Invalid risk distance for risk-based position sizing"};
      }

      const auto risk_value = value * self.current_account_state_.equity();
      return std::abs(risk_value / risk_distance);
    }
    case PositionSizing::Mode::FixedQuantity:
      return std::abs(value);
    case PositionSizing::Mode::FixedNotional:
      if(!std::isfinite(entry_price) || entry_price <= 0.0) {
        throw std::runtime_error{
         "Invalid entry price for fixed-notional position sizing"};
      }
      return std::abs(value / entry_price);
    case PositionSizing::Mode::EquityPercent:
      if(!std::isfinite(entry_price) || entry_price <= 0.0) {
        throw std::runtime_error{
         "Invalid entry price for equity-percent position sizing"};
      }
      return std::abs(self.current_account_state_.equity() * value /
                      entry_price);
    }

    return 0.0;
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
    const auto stop_price = evaluate_series_method(
     position.stop_price_method(),
     asset_snapshot,
     stop_context.with_position_risk_distance(risk_distance));
    const auto target_context =
     stop_context.with_position_risk_distance(risk_distance);
    if(position.stop_loss_enabled() && !std::isfinite(stop_price)) {
      throw std::runtime_error{"Invalid stop price for stop-loss exit"};
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

    open_position.stop_price(stop_price);
    open_position.risk_distance(risk_distance);
    open_position.risk_reference_price(reference_price);
    open_position.risk_boundary_price(reference_price -
                                      direction * risk_distance);
    open_position.stop_loss_price(position.stop_loss_enabled() ? stop_price
                                                               : NAN);
    open_position.stop_loss_trailing_enabled(
     position.stop_loss_trailing_enabled());
    open_position.take_profit_levels(std::move(levels));
    open_position.signal_exit_states(std::move(exit_states));
  }

  auto entry_long_trade(this const BacktestRunner& self,
                        const AssetSnapshot& asset_snapshot,
                        MethodContextable auto context,
                        double current_drawdown_ratio)
   -> std::optional<TradeEntry>
  {
    return self.create_trade(self.long_position_,
                             true,
                             false,
                             asset_snapshot,
                             context,
                             current_drawdown_ratio);
  }

  auto entry_short_trade(this const BacktestRunner& self,
                         const AssetSnapshot& asset_snapshot,
                         MethodContextable auto context,
                         double current_drawdown_ratio)
   -> std::optional<TradeEntry>
  {
    return self.create_trade(self.short_position_,
                             false,
                             false,
                             asset_snapshot,
                             context,
                             current_drawdown_ratio);
  }

  auto pyramiding_long_trade(this const BacktestRunner& self,
                             const AssetSnapshot& asset_snapshot,
                             MethodContextable auto context,
                             double current_drawdown_ratio)
   -> std::optional<TradeEntry>
  {
    return self.create_trade(self.long_position_,
                             true,
                             true,
                             asset_snapshot,
                             context,
                             current_drawdown_ratio);
  }

  auto pyramiding_short_trade(this const BacktestRunner& self,
                              const AssetSnapshot& asset_snapshot,
                              MethodContextable auto context,
                              double current_drawdown_ratio)
   -> std::optional<TradeEntry>
  {
    return self.create_trade(self.short_position_,
                             false,
                             true,
                             asset_snapshot,
                             context,
                             current_drawdown_ratio);
  }

  auto entry_trade(this const BacktestRunner& self,
                   const AssetSnapshot& asset_snapshot,
                   MethodContextable auto context,
                   double current_drawdown_ratio,
                   std::optional<bool> closed_position_is_long = std::nullopt)
   -> std::optional<TradeEntry>
  {
    const auto can_enter_long =
     !closed_position_is_long || !*closed_position_is_long;
    const auto can_enter_short =
     !closed_position_is_long || *closed_position_is_long;

    if(can_enter_long) {
      auto entry_trade =
       self.entry_long_trade(asset_snapshot, context, current_drawdown_ratio);
      if(entry_trade) {
        return entry_trade;
      }
    }

    if(can_enter_short) {
      return self.entry_short_trade(
       asset_snapshot, context, current_drawdown_ratio);
    }

    return std::nullopt;
  }

  auto
  make_exit_trade(this const BacktestRunner& self,
                  double remaining_position_size,
                  double exit_price,
                  TradeExit::Reason reason,
                  double reduce,
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
                     take_profit_index,
                     signal_exit_index};
  }

  auto exit_trade(this BacktestRunner& self,
                  const AssetSnapshot& asset_snapshot,
                  TradePosition& position,
                  MethodContextable auto context) -> std::optional<TradeExit>
  {
    const auto position_size = position.unrealized_position_size();
    const auto is_long_direction = position_size > 0;
    const auto& position_rule =
     is_long_direction ? self.long_position_ : self.short_position_;
    const auto open_exit_price = asset_snapshot.open();

    const auto prev_snapshot = asset_snapshot[1];

    position.update_trailing_stop(prev_snapshot.close());

    if(!self.stop_loss_consumed_ &&
       position.is_stop_loss_triggered(asset_snapshot.high(),
                                       asset_snapshot.low())) {
      const auto stop_price = position.stop_loss_price();
      const auto stop_exit_price = is_long_direction
                                    ? std::min(open_exit_price, stop_price)
                                    : std::max(open_exit_price, stop_price);
      if(auto exit = self.make_exit_trade(position_size,
                                          stop_exit_price,
                                          TradeExit::Reason::stop_loss,
                                          position_rule.stop_loss_reduce())) {
        return exit;
      }
    }

    for(auto index = std::size_t{0};
        index < position_rule.take_profits().size();
        ++index) {
      if(!position.is_take_profit_triggered(
          index, asset_snapshot.high(), asset_snapshot.low())) {
        continue;
      }
      const auto target_price = position.take_profit_levels()[index].price();
      const auto target_exit_price = is_long_direction
                                      ? std::max(open_exit_price, target_price)
                                      : std::min(open_exit_price, target_price);
      if(auto exit =
          self.make_exit_trade(position_size,
                               target_exit_price,
                               TradeExit::Reason::take_profit,
                               position_rule.take_profits()[index].reduce(),
                               index)) {
        return exit;
      }
    }

    for(auto index = std::size_t{0};
        index < position_rule.signal_exits().size();
        ++index) {
      if(index >= position.signal_exit_states().size() ||
         !position.signal_exit_states()[index].active()) {
        continue;
      }
      const auto& signal_exit = position_rule.signal_exits()[index];
      const auto signal_snapshot = asset_snapshot[signal_exit.signal_delay()];
      if(!static_cast<bool>(evaluate_series_method(
          signal_exit.signal_method(), signal_snapshot, context))) {
        continue;
      }
      const auto exit_price = evaluate_series_method(
       signal_exit.price_method(), asset_snapshot, context);
      if(auto exit = self.make_exit_trade(position_size,
                                          exit_price,
                                          TradeExit::Reason::signal,
                                          signal_exit.reduce(),
                                          std::nullopt,
                                          index)) {
        return exit;
      }
    }

    return std::nullopt;
  }
};

} // namespace pludux::backtest
