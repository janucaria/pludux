module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:broker;

import :trade_entry;
import :trade_exit;

export namespace pludux::backtest {

class BrokerFee {
public:
  enum class FeeType { PercentageNotional, Fixed };

  enum class FeePosition { Long, Short, LongAndShort };

  enum class FeeTrigger { Entry, Exit, Buy, Sell, All };

  BrokerFee()
  : BrokerFee{
     "", FeeType::PercentageNotional, FeePosition::Long, FeeTrigger::All, 0.0}
  {
  }

  BrokerFee(std::string name,
            FeeType fee_type,
            FeePosition fee_position,
            FeeTrigger fee_trigger,
            double value)
  : name_{std::move(name)}
  , fee_type_{fee_type}
  , fee_position_{fee_position}
  , fee_trigger_{fee_trigger}
  , value_{value}
  {
  }

  auto operator==(const BrokerFee&) const noexcept -> bool = default;

  auto name(this const BrokerFee& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this BrokerFee& self, std::string name) noexcept
  {
    self.name_ = std::move(name);
  }

  auto fee_type(this const BrokerFee& self) noexcept -> FeeType
  {
    return self.fee_type_;
  }

  void fee_type(this BrokerFee& self, FeeType fee_type) noexcept
  {
    self.fee_type_ = fee_type;
  }

  auto fee_position(this const BrokerFee& self) noexcept -> FeePosition
  {
    return self.fee_position_;
  }

  void fee_position(this BrokerFee& self, FeePosition fee_position) noexcept
  {
    self.fee_position_ = fee_position;
  }

  auto fee_trigger(this const BrokerFee& self) noexcept -> FeeTrigger
  {
    return self.fee_trigger_;
  }

  void fee_trigger(this BrokerFee& self, FeeTrigger fee_trigger) noexcept
  {
    self.fee_trigger_ = fee_trigger;
  }

  auto value(this const BrokerFee& self) noexcept -> double
  {
    return self.value_;
  }

  void value(this BrokerFee& self, double value) noexcept
  {
    self.value_ = value;
  }

  auto equal_rules(this const BrokerFee& self, const BrokerFee& other) noexcept
   -> bool
  {
    return self.fee_type_ == other.fee_type_ &&
           self.fee_position_ == other.fee_position_ &&
           self.fee_trigger_ == other.fee_trigger_ &&
           self.value_ == other.value_;
  }

  template<typename TTradeAction>
    requires std::same_as<TTradeAction, TradeEntry> ||
             std::same_as<TTradeAction, TradeExit>
  auto calculate_fee(this const BrokerFee& self,
                     const TTradeAction& trade_action) noexcept -> double
  {
    auto fee = double{0.0};

    const auto is_position_applicable = [&]() noexcept -> bool {
      switch(self.fee_position()) {
      case FeePosition::Long:
        return trade_action.is_long_direction();
      case FeePosition::Short:
        return trade_action.is_short_direction();
      case FeePosition::LongAndShort:
        return true;
      }
      return false;
    }();

    const auto is_trigger_applicable = [&]() noexcept -> bool {
      switch(self.fee_trigger()) {
      case FeeTrigger::Entry:
        return std::is_same_v<TTradeAction, TradeEntry>;
      case FeeTrigger::Exit:
        return std::is_same_v<TTradeAction, TradeExit>;
      case FeeTrigger::Buy:
        return trade_action.is_buy();
      case FeeTrigger::Sell:
        return trade_action.is_sell();
      case FeeTrigger::All:
        return true;
      }
      return false;
    }();

    if(is_position_applicable && is_trigger_applicable) {
      switch(self.fee_type()) {
      case FeeType::PercentageNotional: {
        fee = trade_action.notional_value() * (self.value() / 100.0);
        break;
      }
      case FeeType::Fixed: {
        fee = self.value();
        break;
      }
      }
    }

    return fee;
  }

private:
  std::string name_;

  FeeType fee_type_;
  FeePosition fee_position_;
  FeeTrigger fee_trigger_;

  double value_;
};

class Broker {
public:
  Broker()
  : Broker{""}
  {
  }

  Broker(std::string name)
  : Broker{std::move(name), {}}
  {
  }

  Broker(std::string name, std::vector<BrokerFee> fees)
  : name_{std::move(name)}
  , fees_{std::move(fees)}
  {
  }

  auto operator==(const Broker&) const noexcept -> bool = default;

  auto name(this const Broker& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Broker& self, std::string name) noexcept
  {
    self.name_ = std::move(name);
  }

  auto fees(this const Broker& self) noexcept -> const std::vector<BrokerFee>&
  {
    return self.fees_;
  }

  void fees(this Broker& self, std::vector<BrokerFee> fees) noexcept
  {
    self.fees_ = std::move(fees);
  }

  auto equal_rules(this const Broker& self, const Broker& other) noexcept
   -> bool
  {
    if(self.fees_.size() != other.fees_.size()) {
      return false;
    }

    for(std::size_t i = 0; i < self.fees_.size(); ++i) {
      if(!self.fees_[i].equal_rules(other.fees_[i])) {
        return false;
      }
    }

    return true;
  }

  template<typename TTradeAction>
    requires std::same_as<TTradeAction, TradeEntry> ||
             std::same_as<TTradeAction, TradeExit>
  auto calculate_fee(this const Broker& self,
                     const TTradeAction& trade_action) noexcept -> double
  {
    return std::ranges::fold_left(
     self.fees_, 0.0, [&](double total_fee, const BrokerFee& fee) {
       return total_fee + fee.calculate_fee(trade_action);
     });
  }

private:
  std::string name_;

  std::vector<BrokerFee> fees_;
};

} // namespace pludux::backtest
