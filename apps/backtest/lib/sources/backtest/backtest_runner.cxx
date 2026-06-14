module;

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <format>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <unordered_map>
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
import :backtest_summary;
import :strategy;
import :broker;
import :market;
import :backtest;

export namespace pludux::backtest {

class BacktestRunner {
public:
  BacktestRunner(
   const Asset& asset,
   const Strategy& strategy,
   const Market& market,
   const Broker& broker,
   const Profile& profile,
   const OrderedNamedRegistry<ConstrainedNumericInput>& backtest_inputs,
   double total_equity = 0.0,
   std::size_t pyramiding_layers = 0,
   bool is_failed = false)
  : asset_{asset}
  , strategy_{strategy}
  , market_{market}
  , broker_{broker}
  , profile_{profile}
  , backtest_inputs_{backtest_inputs}
  , total_equity_{total_equity}
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
           std::vector<BacktestSummary>& summaries)
  {
    if(self.is_failed()) {
      return;
    }

    const auto summaries_size = summaries.size();
    const auto asset_size = self.asset_.size();
    if(summaries_size >= asset_size) {
      return;
    }

    const auto last_index = asset_size - 1;
    const auto asset_lookback =
     last_index - std::min(summaries_size, last_index);
    const auto asset_snapshot = self.asset_.get_snapshot(asset_lookback);

    const auto& series_registry = self.strategy_.series_registry();
    auto context = DefaultMethodContext{series_registry,
                                        series_evaluation_results,
                                        self.backtest_inputs_,
                                        summaries.size()};

    for(const auto& [series_name, series] : series_registry) {
      const auto series_value =
       evaluate_series_method(series, asset_snapshot, context);
      series_evaluation_results.put(series, series_value);
    }

    auto summary = !summaries.empty() ? summaries.back()
                                      : BacktestSummary{self.total_equity_};

    auto total_equity = summary.equity();
    auto trade_session = summary.trade_session();

    trade_session.market_update(
     static_cast<std::time_t>(asset_snapshot.datetime()),
     asset_snapshot.close(),
     asset_snapshot.lookback());

    const auto& open_position = trade_session.open_position();
    if(open_position) {
      {
        const auto is_long_direction = open_position->is_long_direction();
        if(is_long_direction) {
          auto pyramiding_trade =
           self.pyramiding_long_trade(asset_snapshot, context);
          if(pyramiding_trade) {
            const auto fee = self.broker_.calculate_fee(*pyramiding_trade);
            trade_session.entry_position(*pyramiding_trade, fee);
            self.pyramiding_layers_++;
          }
        } else {
          auto pyramiding_trade =
           self.pyramiding_short_trade(asset_snapshot, context);
          if(pyramiding_trade) {
            const auto fee = self.broker_.calculate_fee(*pyramiding_trade);
            trade_session.entry_position(*pyramiding_trade, fee);
            self.pyramiding_layers_++;
          }
        }
      }

      const auto exit_trade =
       trade_session
        .evaluate_exit_conditions(asset_snapshot[1].close(),
                                  asset_snapshot.open(),
                                  asset_snapshot.high(),
                                  asset_snapshot.low())
        .or_else([&]() {
          const auto position_size = open_position->unrealized_position_size();
          return self.exit_trade(asset_snapshot, position_size, context);
        });

      if(exit_trade) {
        const auto fee = self.broker_.calculate_fee(*exit_trade);
        trade_session.exit_position(*exit_trade, fee);
        self.pyramiding_layers_ = 0;
      }
    }

    if(trade_session.is_flat() || trade_session.is_closed()) {
      auto entry_trade = self.entry_trade(asset_snapshot, context);
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
        trade_session.entry_position(*entry_trade, fee);
        self.pyramiding_layers_ = 1;
      }
    }

    summary.update_to_next_summary(std::move(trade_session));

    summaries.emplace_back(std::move(summary));
  }

private:
  const Asset& asset_;

  const Strategy& strategy_;

  const Market& market_;

  const Broker& broker_;

  const Profile& profile_;

  const OrderedNamedRegistry<ConstrainedNumericInput>& backtest_inputs_;

  double total_equity_;

  std::size_t pyramiding_layers_;

  bool is_failed_;

  auto entry_long_trade(this const BacktestRunner& self,
                        const AssetSnapshot& asset_snapshot,
                        MethodContextable auto context) noexcept
   -> std::optional<TradeEntry>
  {
    auto result = std::optional<TradeEntry>{};

    const auto prev_snapshot = asset_snapshot[1];

    if(static_cast<bool>(evaluate_series_method(
        self.strategy_.long_entry_filter(), prev_snapshot, context))) {
      const auto entry_price = asset_snapshot.open();
      const auto risk_value = self.profile_.capital_risk() * self.total_equity_;
      const auto r_distance =
       self.profile_.get_r_distance(entry_price, prev_snapshot, context);

      // TODO: Handle the case when r_distance is zero, negative, or NAN.
      const auto position_size = risk_value / r_distance;

      const auto stop_loss_price =
       self.strategy_.stop_loss_enabled() ? entry_price - r_distance : NAN;
      const auto is_stop_loss_trailing =
       self.strategy_.stop_loss_trailing_enabled();
      const auto take_profit_price =
       self.strategy_.take_profit_enabled()
        ? entry_price + r_distance * self.strategy_.take_profit_r_multiple()
        : NAN;

      result = TradeEntry{position_size,
                          entry_price,
                          stop_loss_price,
                          is_stop_loss_trailing,
                          take_profit_price};
    }

    return result;
  }

  auto entry_short_trade(this const BacktestRunner& self,
                         const AssetSnapshot& asset_snapshot,
                         MethodContextable auto context) noexcept
   -> std::optional<TradeEntry>
  {
    auto result = std::optional<TradeEntry>{};

    const auto prev_snapshot = asset_snapshot[1];

    if(static_cast<bool>(evaluate_series_method(
        self.strategy_.short_entry_filter(), prev_snapshot, context))) {
      const auto entry_price = asset_snapshot.open();
      const auto risk_value = self.profile_.capital_risk() * self.total_equity_;
      const auto r_distance =
       -self.profile_.get_r_distance(entry_price, prev_snapshot, context);
      const auto position_size = risk_value / r_distance;

      const auto stop_loss_price =
       self.strategy_.stop_loss_enabled() ? entry_price - r_distance : NAN;
      const auto is_stop_loss_trailing =
       self.strategy_.stop_loss_trailing_enabled();
      const auto take_profit_price =
       self.strategy_.take_profit_enabled()
        ? entry_price + r_distance * self.strategy_.take_profit_r_multiple()
        : NAN;

      result = TradeEntry{position_size,
                          entry_price,
                          stop_loss_price,
                          is_stop_loss_trailing,
                          take_profit_price};
    }

    return result;
  }

  auto pyramiding_long_trade(this const BacktestRunner& self,
                             const AssetSnapshot& asset_snapshot,
                             MethodContextable auto context) noexcept
   -> std::optional<TradeEntry>
  {
    auto result = std::optional<TradeEntry>{};

    const auto prev_snapshot = asset_snapshot[1];

    const auto& pyramiding =
     self.strategy_.positions().long_side().pyramiding();
    const auto pyramiding_signal = pyramiding.signal();
    const auto pyramiding_max_layers = pyramiding.max_layers();
    if(static_cast<bool>(
        evaluate_series_method(pyramiding_signal, prev_snapshot, context)) &&
       self.pyramiding_layers_ < pyramiding_max_layers) {
      const auto entry_price = asset_snapshot.open();
      const auto risk_value = self.profile_.capital_risk() * self.total_equity_;
      const auto r_distance =
       self.profile_.get_r_distance(entry_price, prev_snapshot, context);
      const auto position_size = risk_value / r_distance;

      const auto stop_loss_price =
       self.strategy_.stop_loss_enabled() ? entry_price - r_distance : NAN;
      const auto is_stop_loss_trailing =
       self.strategy_.stop_loss_trailing_enabled();
      const auto take_profit_price =
       self.strategy_.take_profit_enabled()
        ? entry_price + r_distance * self.strategy_.take_profit_r_multiple()
        : NAN;

      result = TradeEntry{position_size,
                          entry_price,
                          stop_loss_price,
                          is_stop_loss_trailing,
                          take_profit_price};
    }

    return result;
  }

  auto pyramiding_short_trade(this const BacktestRunner& self,
                              const AssetSnapshot& asset_snapshot,
                              MethodContextable auto context) noexcept
   -> std::optional<TradeEntry>
  {
    auto result = std::optional<TradeEntry>{};

    const auto prev_snapshot = asset_snapshot[1];

    const auto& pyramiding =
     self.strategy_.positions().short_side().pyramiding();
    const auto pyramiding_signal = pyramiding.signal();
    const auto pyramiding_max_layers = pyramiding.max_layers();

    if(static_cast<bool>(
        evaluate_series_method(pyramiding_signal, prev_snapshot, context)) &&
       self.pyramiding_layers_ < pyramiding_max_layers) {
      const auto entry_price = asset_snapshot.open();
      const auto risk_value = self.profile_.capital_risk() * self.total_equity_;
      const auto r_distance =
       -self.profile_.get_r_distance(entry_price, prev_snapshot, context);
      const auto position_size = risk_value / r_distance;

      const auto stop_loss_price =
       self.strategy_.stop_loss_enabled() ? entry_price - r_distance : NAN;
      const auto is_stop_loss_trailing =
       self.strategy_.stop_loss_trailing_enabled();
      const auto take_profit_price =
       self.strategy_.take_profit_enabled()
        ? entry_price + r_distance * self.strategy_.take_profit_r_multiple()
        : NAN;

      result = TradeEntry{position_size,
                          entry_price,
                          stop_loss_price,
                          is_stop_loss_trailing,
                          take_profit_price};
    }

    return result;
  }

  auto entry_trade(this const BacktestRunner& self,
                   const AssetSnapshot& asset_snapshot,
                   MethodContextable auto context) noexcept
   -> std::optional<TradeEntry>
  {
    return self.entry_long_trade(asset_snapshot, context).or_else([&] {
      return self.entry_short_trade(asset_snapshot, context);
    });
  }

  auto exit_trade(this const BacktestRunner& self,
                  const AssetSnapshot& asset_snapshot,
                  double position_size,
                  MethodContextable auto context) noexcept
   -> std::optional<TradeExit>
  {
    const auto is_long_direction = position_size > 0;
    const auto is_short_direction = position_size < 0;
    const auto exit_price = asset_snapshot.open();

    const auto prev_snapshot = asset_snapshot[1];

    if(is_long_direction) {
      if(static_cast<bool>(evaluate_series_method(
          self.strategy_.long_exit_filter(), prev_snapshot, context))) {
        return TradeExit{position_size, exit_price, TradeExit::Reason::signal};
      }
    } else if(is_short_direction) {
      if(static_cast<bool>(evaluate_series_method(
          self.strategy_.short_exit_filter(), prev_snapshot, context))) {
        return TradeExit{position_size, exit_price, TradeExit::Reason::signal};
      }
    }

    return std::nullopt;
  }
};

} // namespace pludux::backtest
