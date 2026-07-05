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
import :trade_record;
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
    PositionRule() = default;

    PositionRule(AnySeriesMethod entry_method,
                 AnySeriesMethod exit_method,
                 AnySeriesMethod pyramiding_signal,
                 std::size_t pyramiding_max_layers,
                 AnySeriesMethod stop_price_method,
                 bool stop_loss_enabled,
                 bool stop_loss_trailing_enabled,
                 AnySeriesMethod target_price_method,
                 bool take_profit_enabled,
                 std::size_t entry_signal_delay = 1,
                 AnySeriesMethod entry_price_method = OpenMethod{},
                 std::size_t exit_signal_delay = 1,
                 AnySeriesMethod exit_price_method = OpenMethod{},
                 std::size_t pyramiding_signal_delay = 1,
                 AnySeriesMethod pyramiding_price_method = OpenMethod{})
    : entry_method_{std::move(entry_method)}
    , exit_method_{std::move(exit_method)}
    , pyramiding_signal_{std::move(pyramiding_signal)}
    , pyramiding_max_layers_{pyramiding_max_layers}
    , stop_price_method_{std::move(stop_price_method)}
    , stop_loss_enabled_{stop_loss_enabled}
    , stop_loss_trailing_enabled_{stop_loss_trailing_enabled}
    , target_price_method_{std::move(target_price_method)}
    , take_profit_enabled_{take_profit_enabled}
    , entry_signal_delay_{entry_signal_delay}
    , entry_price_method_{std::move(entry_price_method)}
    , exit_signal_delay_{exit_signal_delay}
    , exit_price_method_{std::move(exit_price_method)}
    , pyramiding_signal_delay_{pyramiding_signal_delay}
    , pyramiding_price_method_{std::move(pyramiding_price_method)}
    {
    }

    auto entry_method(this const PositionRule& self) noexcept
     -> const AnySeriesMethod&
    {
      return self.entry_method_;
    }

    auto exit_method(this const PositionRule& self) noexcept
     -> const AnySeriesMethod&
    {
      return self.exit_method_;
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

    auto stop_loss_enabled(this const PositionRule& self) noexcept -> bool
    {
      return self.stop_loss_enabled_;
    }

    auto stop_loss_trailing_enabled(this const PositionRule& self) noexcept
     -> bool
    {
      return self.stop_loss_trailing_enabled_;
    }

    auto target_price_method(this const PositionRule& self) noexcept
     -> const AnySeriesMethod&
    {
      return self.target_price_method_;
    }

    auto take_profit_enabled(this const PositionRule& self) noexcept -> bool
    {
      return self.take_profit_enabled_;
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

    auto exit_signal_delay(this const PositionRule& self) noexcept
     -> std::size_t
    {
      return self.exit_signal_delay_;
    }

    auto exit_price_method(this const PositionRule& self) noexcept
     -> const AnySeriesMethod&
    {
      return self.exit_price_method_;
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

  private:
    AnySeriesMethod entry_method_{BooleanMethod<false>{}};
    AnySeriesMethod exit_method_{BooleanMethod<false>{}};
    AnySeriesMethod pyramiding_signal_{BooleanMethod<false>{}};
    std::size_t pyramiding_max_layers_{1};
    AnySeriesMethod stop_price_method_{OpenMethod{}};
    bool stop_loss_enabled_{false};
    bool stop_loss_trailing_enabled_{false};
    AnySeriesMethod target_price_method_{OpenMethod{}};
    bool take_profit_enabled_{false};
    std::size_t entry_signal_delay_{1};
    AnySeriesMethod entry_price_method_{OpenMethod{}};
    std::size_t exit_signal_delay_{1};
    AnySeriesMethod exit_price_method_{OpenMethod{}};
    std::size_t pyramiding_signal_delay_{1};
    AnySeriesMethod pyramiding_price_method_{OpenMethod{}};
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
  , total_equity_{total_equity}
  , capital_{total_equity}
  , peak_equity_{std::isnan(peak_equity) ? total_equity : peak_equity}
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

    const auto last_index = asset_size - 1;
    const auto asset_lookback =
     last_index - std::min(timeline_size, last_index);
    const auto asset_snapshot = self.asset_.get_snapshot(asset_lookback);

    const auto& series_methods = self.series_methods_;
    auto default_context = DefaultMethodContext{
     series_methods, series_evaluation_results, timeline.size()};
    auto context =
     BacktestMethodContext{default_context, series_methods, timeline};

    for(const auto& [series_name, series_method] : series_methods) {
      const auto series_value =
       evaluate_series_method(series_method, asset_snapshot, context);
      series_evaluation_results.put(series_method, series_value);
      series_evaluation_results.alias(series_name, series_method);
    }

    const auto current_drawdown_ratio = self.current_drawdown() / 100.0;

    self.trade_session_.begin_market_bar(
     static_cast<std::time_t>(asset_snapshot.datetime()),
     asset_snapshot.close(),
     asset_snapshot.lookback());

    auto closed_position_is_long = std::optional<bool>{};

    if(auto& open_position = self.trade_session_.open_position()) {
      {
        const auto is_long_direction = open_position->is_long_direction();
        if(is_long_direction) {
          auto pyramiding_trade = self.pyramiding_long_trade(
           asset_snapshot, context, current_drawdown_ratio);
          if(pyramiding_trade) {
            const auto fee = self.broker_.calculate_fee(*pyramiding_trade);
            self.trade_session_.entry_position(*pyramiding_trade, fee);
            self.pyramiding_layers_++;
          }
        } else {
          auto pyramiding_trade = self.pyramiding_short_trade(
           asset_snapshot, context, current_drawdown_ratio);
          if(pyramiding_trade) {
            const auto fee = self.broker_.calculate_fee(*pyramiding_trade);
            self.trade_session_.entry_position(*pyramiding_trade, fee);
            self.pyramiding_layers_++;
          }
        }
      }

      if(auto& updated_open_position = self.trade_session_.open_position()) {
        const auto exit_trade =
         self.exit_trade(asset_snapshot, *updated_open_position, context);

        if(exit_trade) {
          const auto fee = self.broker_.calculate_fee(*exit_trade);
          const auto closing_position_is_long =
           updated_open_position->is_long_direction();
          self.trade_session_.exit_position(*exit_trade, fee);

          if(!self.trade_session_.is_open()) {
            closed_position_is_long = closing_position_is_long;
          }

          self.pyramiding_layers_ = 0;
        }
      }
    }

    if(self.trade_session_.is_flat()) {
      auto entry_trade = self.entry_trade(asset_snapshot,
                                          context,
                                          current_drawdown_ratio,
                                          closed_position_is_long);
      if(entry_trade) {
        {
          const auto quantity_step = self.market_.quantity_step();
          const auto min_order_quantity = self.market_.min_order_quantity();

          auto position_size = entry_trade->position_size();
          if(quantity_step > 0.0 &&
             std::fmod(position_size, quantity_step) != 0.0) {
            position_size =
             quantity_step * std::round(position_size / quantity_step);
          }

          if(position_size > 0.0 && position_size < min_order_quantity) {
            position_size = min_order_quantity;
          } else if(position_size < 0.0 &&
                    position_size > -min_order_quantity) {
            position_size = -min_order_quantity;
          }

          entry_trade->position_size(position_size);
        }

        const auto fee = self.broker_.calculate_fee(*entry_trade);
        self.trade_session_.entry_position(*entry_trade, fee);
        self.pyramiding_layers_ = 1;
      }
    }

    self.update_accounting();
    self.total_equity_ = self.current_equity();

    auto trade_records = self.trade_records();

    timeline.append(BacktestTimeline::Row{
     .market_timestamp = self.trade_session_.market_timestamp(),
     .market_price = self.trade_session_.market_price(),
     .market_lookback = self.trade_session_.market_lookback(),
     .trade_records = std::move(trade_records),
     .capital = self.capital_,
     .equity = self.current_equity(),
     .peak_equity = self.peak_equity_,
     .drawdown = self.current_drawdown(),
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
  }

private:
  const Asset& asset_;

  const Market& market_;

  const Broker& broker_;

  const Profile& profile_;

  double total_equity_;
  double capital_;
  double peak_equity_;
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

  auto current_equity(this const BacktestRunner& self) noexcept -> double
  {
    return self.capital_ + self.trade_session_.unrealized_pnl();
  }

  auto current_drawdown(this const BacktestRunner& self) noexcept -> double
  {
    return self.peak_equity_ ? (self.peak_equity_ - self.current_equity()) /
                                self.peak_equity_ * 100.0
                             : 0.0;
  }

  void update_accounting(this BacktestRunner& self) noexcept
  {
    for(const auto& trade_record : self.trade_session_.trade_records()) {
      if(!trade_record.is_closed()) {
        continue;
      }

      const auto pnl = trade_record.pnl();

      self.sum_of_durations_ += trade_record.duration();
      self.cumulative_investments_ += trade_record.investment();

      if(pnl > 0) {
        self.profit_count_++;
        self.cumulative_profits_ += pnl;
      } else if(pnl < 0) {
        self.loss_count_++;
        self.cumulative_losses_ += pnl;
      } else {
        self.break_even_count_++;
      }

      self.capital_ += pnl;
    }

    self.peak_equity_ = std::max(self.peak_equity_, self.current_equity());
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
      const auto position_sizing = self.profile_.position_sizing();
      const auto uses_risk_distance =
       position_sizing.mode() == PositionSizing::Mode::RiskDistance;
      const auto needs_stop_price =
       uses_risk_distance || position.stop_loss_enabled();
      const auto stop_price =
       needs_stop_price ? evaluate_series_method(position.stop_price_method(),
                                                 asset_snapshot,
                                                 context)
                        : NAN;
      if(position.stop_loss_enabled() && !uses_risk_distance &&
         !std::isfinite(stop_price)) {
        throw std::runtime_error{"Invalid stop price for stop-loss exit"};
      }
      const auto position_quantity = self.calculate_position_quantity(
       position_sizing, is_long, entry_price, stop_price);
      const auto adjusted_position_quantity = self.apply_drawdown_adjustment(
       position_quantity, current_drawdown_ratio);
      const auto direction = is_long ? 1.0 : -1.0;
      const auto position_size = direction * adjusted_position_quantity;

      const auto stop_loss_price =
       position.stop_loss_enabled() ? stop_price : NAN;
      const auto is_stop_loss_trailing = position.stop_loss_trailing_enabled();
      const auto take_profit_price =
       position.take_profit_enabled()
        ? evaluate_series_method(
           position.target_price_method(), asset_snapshot, context)
        : NAN;

      result = TradeEntry{position_size,
                          entry_price,
                          stop_loss_price,
                          is_stop_loss_trailing,
                          take_profit_price};
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
                                   bool is_long,
                                   double entry_price,
                                   double stop_price) -> double
  {
    const auto value = position_sizing.value();

    if(!std::isfinite(value)) {
      throw std::runtime_error{"Invalid position sizing value"};
    }

    switch(position_sizing.mode()) {
    case PositionSizing::Mode::RiskDistance: {
      const auto risk_distance =
       is_long ? entry_price - stop_price : stop_price - entry_price;

      if(std::isnan(stop_price) || !std::isfinite(risk_distance) ||
         risk_distance <= 0.0) {
        throw std::runtime_error{
         "Invalid stop price for risk-based position sizing"};
      }

      const auto risk_value = value * self.total_equity_;
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
      return std::abs(self.total_equity_ * value / entry_price);
    }

    return 0.0;
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

  auto exit_trade(this BacktestRunner& self,
                  const AssetSnapshot& asset_snapshot,
                  TradePosition& position,
                  MethodContextable auto context) noexcept
   -> std::optional<TradeExit>
  {
    const auto position_size = position.unrealized_position_size();
    const auto is_long_direction = position_size > 0;
    const auto is_short_direction = position_size < 0;
    const auto open_exit_price = asset_snapshot.open();

    const auto prev_snapshot = asset_snapshot[1];

    position.update_trailing_stop(prev_snapshot.close());

    if(position.is_stop_loss_triggered(asset_snapshot.high(),
                                       asset_snapshot.low())) {
      const auto stop_price = position.stop_loss_price();
      const auto stop_exit_price = is_long_direction
                                    ? std::min(open_exit_price, stop_price)
                                    : std::max(open_exit_price, stop_price);
      return TradeExit{
       position_size, stop_exit_price, TradeExit::Reason::stop_loss};
    }

    if(position.is_take_profit_triggered(asset_snapshot.high(),
                                         asset_snapshot.low())) {
      const auto target_price = position.take_profit_price();
      const auto target_exit_price = is_long_direction
                                      ? std::max(open_exit_price, target_price)
                                      : std::min(open_exit_price, target_price);
      return TradeExit{
       position_size, target_exit_price, TradeExit::Reason::take_profit};
    }

    if(is_long_direction) {
      const auto signal_snapshot =
       asset_snapshot[self.long_position_.exit_signal_delay()];
      if(static_cast<bool>(evaluate_series_method(
          self.long_position_.exit_method(), signal_snapshot, context))) {
        const auto exit_price = evaluate_series_method(
         self.long_position_.exit_price_method(), asset_snapshot, context);
        return TradeExit{position_size, exit_price, TradeExit::Reason::signal};
      }
    } else if(is_short_direction) {
      const auto signal_snapshot =
       asset_snapshot[self.short_position_.exit_signal_delay()];
      if(static_cast<bool>(evaluate_series_method(
          self.short_position_.exit_method(), signal_snapshot, context))) {
        const auto exit_price = evaluate_series_method(
         self.short_position_.exit_price_method(), asset_snapshot, context);
        return TradeExit{position_size, exit_price, TradeExit::Reason::signal};
      }
    }

    return std::nullopt;
  }

  auto trade_records(this const BacktestRunner& self)
   -> std::vector<TradeRecord>
  {
    auto trade_records = self.trade_session_.trade_records();

    if(const auto& open_position = self.trade_session_.open_position()) {
      trade_records.emplace_back(TradeRecord::Status::open,
                                 open_position->position_size(),
                                 open_position->investment(),
                                 open_position->entry_timestamp(),
                                 open_position->entry_price(),
                                 open_position->total_entry_fees(),
                                 self.trade_session_.market_timestamp(),
                                 self.trade_session_.market_price(),
                                 0.0,
                                 open_position->stop_loss_initial_price(),
                                 open_position->stop_loss_trailing_price(),
                                 open_position->take_profit_price());
    }

    return trade_records;
  }
};

} // namespace pludux::backtest
