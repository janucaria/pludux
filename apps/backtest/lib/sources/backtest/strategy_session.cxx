module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

export module pludux.backtest:strategy_session;

import :strategy_closed_position;
import :strategy_intent;

export namespace pludux::backtest {

class StrategyOpenPositionSnapshot {
public:
  StrategyOpenPositionSnapshot() = default;

  StrategyOpenPositionSnapshot(std::size_t strategy_trade_id,
                               StrategyDirection direction,
                               std::time_t entry_timestamp,
                               double normalized_quantity,
                               double normalized_investment,
                               double realized_price_pnl,
                               double market_price) noexcept
  : strategy_trade_id_{strategy_trade_id}
  , direction_{direction}
  , entry_timestamp_{entry_timestamp}
  , normalized_quantity_{normalized_quantity}
  , normalized_investment_{normalized_investment}
  , realized_price_pnl_{realized_price_pnl}
  , market_price_{market_price}
  {
  }

  auto operator==(const StrategyOpenPositionSnapshot&) const noexcept
   -> bool = default;

  auto strategy_trade_id(this const StrategyOpenPositionSnapshot& self) noexcept
   -> std::size_t
  {
    return self.strategy_trade_id_;
  }

  auto direction(this const StrategyOpenPositionSnapshot& self) noexcept
   -> StrategyDirection
  {
    return self.direction_;
  }

  auto entry_timestamp(this const StrategyOpenPositionSnapshot& self) noexcept
   -> std::time_t
  {
    return self.entry_timestamp_;
  }

  auto
  normalized_quantity(this const StrategyOpenPositionSnapshot& self) noexcept
   -> double
  {
    return self.normalized_quantity_;
  }

  auto
  normalized_investment(this const StrategyOpenPositionSnapshot& self) noexcept
   -> double
  {
    return self.normalized_investment_;
  }

  auto market_price(this const StrategyOpenPositionSnapshot& self) noexcept
   -> double
  {
    return self.market_price_;
  }

  auto duration(this const StrategyOpenPositionSnapshot& self,
                std::time_t market_timestamp) noexcept -> std::time_t
  {
    return market_timestamp - self.entry_timestamp_;
  }

  auto average_price(this const StrategyOpenPositionSnapshot& self) noexcept
   -> double
  {
    return self.normalized_quantity_ > 0.0
            ? self.normalized_investment_ / self.normalized_quantity_
            : 0.0;
  }

  auto
  unrealized_price_pnl(this const StrategyOpenPositionSnapshot& self) noexcept
   -> double
  {
    const auto direction =
     self.direction_ == StrategyDirection::Long ? 1.0 : -1.0;
    return self.realized_price_pnl_ +
           direction * (self.normalized_quantity_ * self.market_price_ -
                        self.normalized_investment_);
  }

  auto unrealized_return_ratio(
   this const StrategyOpenPositionSnapshot& self) noexcept -> double
  {
    return self.normalized_investment_ > 0.0
            ? self.unrealized_price_pnl() / self.normalized_investment_
            : 0.0;
  }

private:
  std::size_t strategy_trade_id_{};
  StrategyDirection direction_{StrategyDirection::Long};
  std::time_t entry_timestamp_{};
  double normalized_quantity_{};
  double normalized_investment_{};
  double realized_price_pnl_{};
  double market_price_{};
};

class StrategySession {
public:
  StrategySession() = default;

  void begin_market_bar(this StrategySession& self,
                        std::time_t timestamp,
                        double market_price) noexcept
  {
    self.market_timestamp_ = timestamp;
    self.market_price_ = market_price;
    self.intents_.clear();
    self.closed_positions_.clear();
  }

  auto is_open(this const StrategySession& self) noexcept -> bool
  {
    return self.position_.has_value();
  }

  auto is_flat(this const StrategySession& self) noexcept -> bool
  {
    return !self.is_open();
  }

  auto position(this const StrategySession& self) noexcept
   -> const std::optional<StrategyOpenPositionSnapshot>
  {
    if(!self.position_) {
      return std::nullopt;
    }
    const auto& position = *self.position_;
    return StrategyOpenPositionSnapshot{position.strategy_trade_id,
                                        position.direction,
                                        position.entry_timestamp,
                                        position.normalized_quantity,
                                        position.normalized_investment,
                                        position.realized_price_pnl,
                                        self.market_price_};
  }

  auto intents(this const StrategySession& self) noexcept
   -> const std::vector<StrategyIntent>&
  {
    return self.intents_;
  }

  auto closed_positions(this const StrategySession& self) noexcept
   -> const std::vector<StrategyClosedPosition>&
  {
    return self.closed_positions_;
  }

  auto enter(this StrategySession& self,
             StrategyDirection direction,
             double price,
             bool pyramiding = false) -> const StrategyIntent&
  {
    if(!std::isfinite(price) || price <= 0.0) {
      throw std::invalid_argument{
       "Strategy entry price must be finite and positive"};
    }

    if(!self.position_) {
      if(pyramiding) {
        throw std::runtime_error{"Cannot pyramid a flat strategy position"};
      }
      const auto strategy_trade_id = self.next_strategy_trade_id_++;
      self.position_ = Position{.strategy_trade_id = strategy_trade_id,
                                .direction = direction,
                                .entry_timestamp = self.market_timestamp_};
    } else if(!pyramiding || self.position_->direction != direction) {
      throw std::runtime_error{
       "Strategy position entry does not match open position"};
    }

    auto& position = *self.position_;
    position.normalized_quantity += 1.0;
    position.normalized_entry_quantity += 1.0;
    position.normalized_investment += price;
    position.normalized_entry_notional += price;
    const auto type = pyramiding ? StrategyIntentType::PyramidingEntry
                                 : StrategyIntentType::InitialEntry;
    self.intents_.emplace_back(self.next_intent_id_++,
                               position.strategy_trade_id,
                               type,
                               direction,
                               self.market_timestamp_,
                               price);
    position.intents.push_back(self.intents_.back());
    return self.intents_.back();
  }

  auto exit(this StrategySession& self,
            StrategyIntentType type,
            double price,
            double reduce = 1.0,
            std::optional<std::size_t> rule_index = std::nullopt)
   -> const StrategyIntent&
  {
    if(!self.position_) {
      throw std::runtime_error{"Cannot exit a flat strategy position"};
    }
    if(type == StrategyIntentType::InitialEntry ||
       type == StrategyIntentType::PyramidingEntry) {
      throw std::invalid_argument{"Strategy exit requires an exit type"};
    }
    if(!std::isfinite(price) || price <= 0.0 || !std::isfinite(reduce) ||
       reduce <= 0.0 || reduce > 1.0) {
      throw std::invalid_argument{"Invalid strategy exit"};
    }

    auto& position = *self.position_;
    const auto exit_quantity = std::min(position.normalized_quantity,
                                        position.normalized_quantity * reduce);
    const auto average_price =
     position.normalized_investment / position.normalized_quantity;
    const auto direction =
     position.direction == StrategyDirection::Long ? 1.0 : -1.0;
    position.realized_price_pnl +=
     direction * exit_quantity * (price - average_price);
    position.normalized_quantity -= exit_quantity;
    position.normalized_investment -= exit_quantity * average_price;

    self.intents_.emplace_back(self.next_intent_id_++,
                               position.strategy_trade_id,
                               type,
                               position.direction,
                               self.market_timestamp_,
                               price,
                               reduce,
                               rule_index);
    position.intents.push_back(self.intents_.back());

    if(position.normalized_quantity <= std::numeric_limits<double>::epsilon()) {
      self.closed_positions_.emplace_back(position.strategy_trade_id,
                                          position.direction,
                                          position.entry_timestamp,
                                          self.market_timestamp_,
                                          position.normalized_entry_quantity,
                                          position.normalized_entry_notional,
                                          position.realized_price_pnl,
                                          std::move(position.intents));
      self.position_.reset();
    }

    return self.intents_.back();
  }

private:
  struct Position {
    std::size_t strategy_trade_id{};
    StrategyDirection direction{StrategyDirection::Long};
    std::time_t entry_timestamp{};
    double normalized_quantity{};
    double normalized_entry_quantity{};
    double normalized_investment{};
    double normalized_entry_notional{};
    double realized_price_pnl{};
    std::vector<StrategyIntent> intents{};
  };

  std::time_t market_timestamp_{};
  double market_price_{};
  std::optional<Position> position_{};
  std::vector<StrategyIntent> intents_{};
  std::vector<StrategyClosedPosition> closed_positions_{};
  std::size_t next_strategy_trade_id_{1};
  std::size_t next_intent_id_{1};
};

} // namespace pludux::backtest
