module;

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <format>
#include <iostream>
#include <limits>
#include <optional>
#include <vector>

export module pludux.backtest:backtest;

import pludux;

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

export namespace pludux::backtest {

class Backtest {
public:
  Backtest()
  : Backtest{std::string{},
             nullptr,
             nullptr,
             nullptr,
             nullptr,
             nullptr,
             std::vector<BacktestSummary>{}}
  {
  }

  Backtest(std::string name,
           std::shared_ptr<Asset> asset_ptr,
           std::shared_ptr<Strategy> strategy_ptr,
           std::shared_ptr<Market> market_ptr,
           std::shared_ptr<Broker> broker_ptr,
           std::shared_ptr<Profile> profile_ptr)
  : Backtest{std::move(name),
             std::move(asset_ptr),
             std::move(strategy_ptr),
             std::move(market_ptr),
             std::move(broker_ptr),
             std::move(profile_ptr),
             std::vector<BacktestSummary>{}}
  {
  }

  Backtest(std::string name,
           std::shared_ptr<Asset> asset_ptr,
           std::shared_ptr<Strategy> strategy_ptr,
           std::shared_ptr<Market> market_ptr,
           std::shared_ptr<Broker> broker_ptr,
           std::shared_ptr<Profile> profile_ptr,
           std::vector<BacktestSummary> summaries)
  : name_{std::move(name)}
  , asset_ptr_{asset_ptr}
  , strategy_ptr_{strategy_ptr}
  , market_ptr_{market_ptr}
  , broker_ptr_{broker_ptr}
  , profile_ptr_{profile_ptr}
  , is_failed_{false}
  , summaries_{std::move(summaries)}
  {
  }

  auto name(this const Backtest& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Backtest& self, std::string new_name) noexcept
  {
    self.name_ = std::move(new_name);
  }

  auto strategy_ptr(this const Backtest& self) noexcept
   -> const std::shared_ptr<Strategy>
  {
    return self.strategy_ptr_;
  }

  void strategy_ptr(this Backtest& self,
                    std::shared_ptr<Strategy> new_strategy_ptr) noexcept
  {
    self.strategy_ptr_ = std::move(new_strategy_ptr);
  }

  auto strategy(this const Backtest& self) noexcept -> const Strategy&
  {
    return *self.strategy_ptr();
  }

  auto asset_ptr(this const Backtest& self) noexcept
   -> const std::shared_ptr<Asset>
  {
    return self.asset_ptr_;
  }

  void asset_ptr(this Backtest& self,
                 std::shared_ptr<Asset> new_asset_ptr) noexcept
  {
    self.asset_ptr_ = std::move(new_asset_ptr);
  }

  auto asset(this const Backtest& self) noexcept -> const Asset&
  {
    return *self.asset_ptr();
  }

  auto broker_ptr(this const Backtest& self) noexcept
   -> const std::shared_ptr<Broker>
  {
    return self.broker_ptr_;
  }

  void broker_ptr(this Backtest& self,
                  std::shared_ptr<Broker> new_broker_ptr) noexcept
  {
    self.broker_ptr_ = std::move(new_broker_ptr);
  }

  auto broker(this const Backtest& self) noexcept -> const Broker&
  {
    return *self.broker_ptr();
  }

  auto market_ptr(this const Backtest& self) noexcept
   -> const std::shared_ptr<Market>
  {
    return self.market_ptr_;
  }

  void market_ptr(this Backtest& self,
                  std::shared_ptr<Market> new_market_ptr) noexcept
  {
    self.market_ptr_ = std::move(new_market_ptr);
  }

  auto market(this const Backtest& self) noexcept -> const Market&
  {
    return *self.market_ptr();
  }

  auto profile_ptr(this const Backtest& self) noexcept
   -> const std::shared_ptr<Profile>
  {
    return self.profile_ptr_;
  }

  void profile_ptr(this Backtest& self,
                   std::shared_ptr<Profile> new_profile_ptr) noexcept
  {
    self.profile_ptr_ = std::move(new_profile_ptr);
  }

  auto profile(this const Backtest& self) noexcept -> const Profile&
  {
    return *self.profile_ptr();
  }

  void mark_as_failed(this Backtest& self) noexcept
  {
    self.is_failed_ = true;
  }

  auto is_failed(this const Backtest& self) noexcept -> bool
  {
    return self.is_failed_;
  }

  auto summaries(this const Backtest& self) noexcept
   -> const std::vector<BacktestSummary>&
  {
    return self.summaries_;
  }

  void reset(this Backtest& self) noexcept
  {
    self.is_failed_ = false;
    self.summaries_.clear();
  }

  auto should_run(this const Backtest& self) noexcept -> bool
  {
    if(!self.asset_ptr() || !self.strategy_ptr() || !self.market_ptr() ||
       !self.broker_ptr() || !self.profile_ptr()) {
      return false;
    }

    const auto summaries_size = self.summaries_.size();
    const auto asset_size = self.asset().size();

    return summaries_size < asset_size && !self.is_failed();
  }

  void run(this Backtest& self)
  {
    using namespace backtest;

    if(!self.should_run()) {
      return;
    }

    const auto summaries_size = self.summaries_.size();
    const auto asset_size = self.asset().size();
    const auto last_index = asset_size - 1;
    const auto asset_index = last_index - std::min(summaries_size, last_index);
    const auto asset_snapshot = self.asset().get_snapshot(asset_index);
    const auto asset_timestamp =
     static_cast<std::time_t>(asset_snapshot.datetime());
    const auto asset_price = asset_snapshot.close();
    const auto& strategy = self.strategy();
    const auto& profile = self.profile();
    const auto& broker = self.broker();
    const auto& market = self.market();

    auto summary = !self.summaries_.empty()
                    ? self.summaries_.back()
                    : BacktestSummary{profile.initial_capital()};

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
          return self.exit_trade(asset_snapshot, position_size);
        });

      if(exit_trade) {
        const auto fee = broker.calculate_fee(*exit_trade);
        trade_session.exit_position(*exit_trade, fee);
      }
    }

    if(trade_session.is_flat() || trade_session.is_closed()) {
      auto entry_trade =
       self.entry_trade(asset_snapshot, profile.get_risk_value());

      if(entry_trade) {
        {
          const auto qty_step = market.qty_step();
          const auto min_order_qty = market.min_order_qty();

          auto position_size = entry_trade->position_size();
          if(qty_step > 0.0 && std::fmod(position_size, qty_step) != 0.0) {
            position_size = qty_step * std::round(position_size / qty_step);
          }

          if(position_size > 0.0 && position_size < min_order_qty) {
            position_size = min_order_qty;
          } else if(position_size < 0.0 && position_size > -min_order_qty) {
            position_size = -min_order_qty;
          }

          entry_trade->position_size(position_size);
        }

        const auto fee = broker.calculate_fee(*entry_trade);
        trade_session.entry_position(*entry_trade, fee);
      }
    }

    summary.update_to_next_summary(std::move(trade_session));

    self.summaries_.emplace_back(std::move(summary));
  }

  auto entry_long_trade(this const Backtest& self,
                        const AssetSnapshot& asset_snapshot,
                        double risk_value) noexcept -> std::optional<TradeEntry>
  {
    auto result = std::optional<TradeEntry>{};

    const auto& strategy = self.strategy();
    const auto prev_snapshot = asset_snapshot[1];
    auto context = self.create_default_method_context();

    if(strategy.long_entry_filter()(prev_snapshot, context)) {
      const auto risk_size = strategy.risk_method()(prev_snapshot, context);
      const auto position_size = risk_value / risk_size;
      const auto entry_price = asset_snapshot.open();

      const auto stop_loss_price =
       strategy.stop_loss_enabled() ? entry_price - risk_size : NAN;
      const auto is_stop_loss_trailing = strategy.stop_loss_trailing_enabled();
      const auto take_profit_price =
       strategy.take_profit_enabled()
        ? entry_price + risk_size * strategy.take_profit_risk_multiplier()
        : NAN;

      result = TradeEntry{position_size,
                          entry_price,
                          stop_loss_price,
                          is_stop_loss_trailing,
                          take_profit_price};
    }

    return result;
  }

  auto entry_short_trade(this const Backtest& self,
                         const AssetSnapshot& asset_snapshot,
                         double risk_value) noexcept
   -> std::optional<TradeEntry>
  {
    auto result = std::optional<TradeEntry>{};

    const auto& strategy = self.strategy();
    auto context = self.create_default_method_context();
    const auto prev_snapshot = asset_snapshot[1];

    if(strategy.short_entry_filter()(prev_snapshot, context)) {
      const auto risk_size = -strategy.risk_method()(prev_snapshot, context);
      const auto position_size = risk_value / risk_size;
      const auto entry_price = asset_snapshot.open();

      const auto stop_loss_price =
       strategy.stop_loss_enabled() ? entry_price - risk_size : NAN;
      const auto is_stop_loss_trailing = strategy.stop_loss_trailing_enabled();
      const auto take_profit_price =
       strategy.take_profit_enabled()
        ? entry_price + risk_size * strategy.take_profit_risk_multiplier()
        : NAN;

      result = TradeEntry{position_size,
                          entry_price,
                          stop_loss_price,
                          is_stop_loss_trailing,
                          take_profit_price};
    }

    return result;
  }

  auto entry_trade(this const Backtest& self,
                   const AssetSnapshot& asset_snapshot,
                   double risk_value) noexcept -> std::optional<TradeEntry>
  {
    return self.entry_long_trade(asset_snapshot, risk_value).or_else([&] {
      return self.entry_short_trade(asset_snapshot, risk_value);
    });
  }

  auto exit_trade(this const Backtest& self,
                  const AssetSnapshot& asset_snapshot,
                  double position_size) noexcept -> std::optional<TradeExit>
  {
    const auto is_long_direction = position_size > 0;
    const auto is_short_direction = position_size < 0;
    const auto exit_price = asset_snapshot.open();

    auto const& strategy = self.strategy();
    auto context = self.create_default_method_context();
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

private:
  std::string name_;

  std::shared_ptr<Asset> asset_ptr_;
  std::shared_ptr<Strategy> strategy_ptr_;
  std::shared_ptr<Market> market_ptr_;
  std::shared_ptr<Broker> broker_ptr_;
  std::shared_ptr<Profile> profile_ptr_;

  bool is_failed_;
  std::vector<BacktestSummary> summaries_;

  auto create_default_method_context(this const Backtest& self)
   -> DefaultMethodContext
  {
    return DefaultMethodContext{self.strategy().series_registry(),
                                self.summaries_.size()};
  }
};

} // namespace pludux::backtest
