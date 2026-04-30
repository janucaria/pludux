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
  BacktestRunner(double initial_capital,
                 const Asset& asset,
                 const Strategy& strategy,
                 const Market& market,
                 const Broker& broker,
                 const Profile& profile,
                 std::vector<BacktestSummary>& summaries,
                 SeriesResultsCollector& series_results_collector)
  : initial_capital_{initial_capital}
  , asset_{asset}
  , strategy_{strategy}
  , market_{market}
  , broker_{broker}
  , profile_{profile}
  , summaries_{summaries}
  , series_results_collector_{series_results_collector}
  {
  }

  void run(this BacktestRunner& self)
  {
    const auto& asset = self.asset_;
    const auto& strategy = self.strategy_;
    const auto& market = self.market_;
    const auto& broker = self.broker_;
    const auto& profile = self.profile_;

    auto& summaries = self.summaries_;

    const auto summaries_size = summaries.size();
    const auto asset_size = asset.size();
    if(summaries_size >= asset_size) {
      return;
    }

    const auto last_index = asset_size - 1;
    const auto asset_lookback =
     last_index - std::min(summaries_size, last_index);
    const auto asset_snapshot = asset.get_snapshot(asset_lookback);

    {
      const auto& series_registry = strategy.series_registry();
      auto context = self.create_default_method_context(strategy);
      for(const auto& [series_name, series] : series_registry) {
        const auto series_value = series(asset_snapshot, context);
        self.series_results_collector_.collect(series_name, series_value);
      }
    }

    auto summary = !summaries.empty() ? summaries.back()
                                      : BacktestSummary{self.initial_capital_};

    auto trade_session = summary.trade_session();

    trade_session.market_update(
     static_cast<std::time_t>(asset_snapshot.datetime()),
     asset_snapshot.close(),
     asset_snapshot.lookback());

    const auto& open_position = trade_session.open_position();
    if(open_position) {
      const auto exit_trade =
       trade_session
        .evaluate_exit_conditions(asset_snapshot[1].close(),
                                  asset_snapshot.open(),
                                  asset_snapshot.high(),
                                  asset_snapshot.low())
        .or_else([&]() {
          const auto position_size = open_position->unrealized_position_size();
          return self.exit_trade(asset_snapshot, position_size, strategy);
        });

      if(exit_trade) {
        const auto fee = broker.calculate_fee(*exit_trade);
        trade_session.exit_position(*exit_trade, fee);
      }
    }

    if(trade_session.is_flat() || trade_session.is_closed()) {
      auto entry_trade = self.entry_trade(asset_snapshot, strategy, profile);
      if(entry_trade) {
        {
          const auto quantity_step = market.quantity_step();
          const auto min_order_quantity = market.min_order_quantity();

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

        const auto fee = broker.calculate_fee(*entry_trade);
        trade_session.entry_position(*entry_trade, fee);
      }
    }

    summary.update_to_next_summary(std::move(trade_session));

    summaries.emplace_back(std::move(summary));
  }

private:
  double initial_capital_;

  const Asset& asset_;
  const Strategy& strategy_;
  const Market& market_;
  const Broker& broker_;
  const Profile& profile_;

  std::vector<BacktestSummary>& summaries_;
  SeriesResultsCollector& series_results_collector_;

  auto create_default_method_context(this const BacktestRunner& self,
                                     const Strategy& strategy)
   -> DefaultMethodContext
  {
    return DefaultMethodContext{strategy.series_registry(),
                                self.series_results_collector_,
                                self.summaries_.size()};
  }

  auto entry_long_trade(this const BacktestRunner& self,
                        const AssetSnapshot& asset_snapshot,
                        const Strategy& strategy,
                        const Profile& profile) noexcept
   -> std::optional<TradeEntry>
  {
    auto result = std::optional<TradeEntry>{};

    const auto prev_snapshot = asset_snapshot[1];
    auto context = self.create_default_method_context(strategy);

    if(strategy.long_entry_filter()(prev_snapshot, context)) {
      const auto entry_price = asset_snapshot.open();
      const auto risk_value = profile.capital_risk() * self.initial_capital_;
      const auto r_distance =
       profile.get_r_distance(entry_price, prev_snapshot, context);
      const auto position_size = risk_value / r_distance;

      const auto stop_loss_price =
       strategy.stop_loss_enabled() ? entry_price - r_distance : NAN;
      const auto is_stop_loss_trailing = strategy.stop_loss_trailing_enabled();
      const auto take_profit_price =
       strategy.take_profit_enabled()
        ? entry_price + r_distance * strategy.take_profit_r_multiple()
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
                         const Strategy& strategy,
                         const Profile& profile) noexcept
   -> std::optional<TradeEntry>
  {
    auto result = std::optional<TradeEntry>{};

    auto context = self.create_default_method_context(strategy);
    const auto prev_snapshot = asset_snapshot[1];

    if(strategy.short_entry_filter()(prev_snapshot, context)) {
      const auto entry_price = asset_snapshot.open();
      const auto risk_value = profile.capital_risk() * self.initial_capital_;
      const auto r_distance =
       -profile.get_r_distance(entry_price, prev_snapshot, context);
      const auto position_size = risk_value / r_distance;

      const auto stop_loss_price =
       strategy.stop_loss_enabled() ? entry_price - r_distance : NAN;
      const auto is_stop_loss_trailing = strategy.stop_loss_trailing_enabled();
      const auto take_profit_price =
       strategy.take_profit_enabled()
        ? entry_price + r_distance * strategy.take_profit_r_multiple()
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
                   const Strategy& strategy,
                   const Profile& profile) noexcept -> std::optional<TradeEntry>
  {
    return self.entry_long_trade(asset_snapshot, strategy, profile)
     .or_else([&] {
       return self.entry_short_trade(asset_snapshot, strategy, profile);
     });
  }

  auto exit_trade(this const BacktestRunner& self,
                  const AssetSnapshot& asset_snapshot,
                  double position_size,
                  const Strategy& strategy) noexcept -> std::optional<TradeExit>
  {
    const auto is_long_direction = position_size > 0;
    const auto is_short_direction = position_size < 0;
    const auto exit_price = asset_snapshot.open();

    auto context = self.create_default_method_context(strategy);
    const auto prev_snapshot = asset_snapshot[1];

    if(is_long_direction) {
      if(strategy.long_exit_filter()(prev_snapshot, context)) {
        return TradeExit{position_size, exit_price, TradeExit::Reason::signal};
      }
    } else if(is_short_direction) {
      if(strategy.short_exit_filter()(prev_snapshot, context)) {
        return TradeExit{position_size, exit_price, TradeExit::Reason::signal};
      }
    }

    return std::nullopt;
  }
};

} // namespace pludux::backtest
