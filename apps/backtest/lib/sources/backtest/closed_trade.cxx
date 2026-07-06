module;

#include <cstddef>
#include <ctime>

export module pludux.backtest:closed_trade;

import :trade_event;

export namespace pludux::backtest {

class ClosedTrade {
public:
  ClosedTrade() = default;

  ClosedTrade(std::size_t trade_id,
              std::size_t exit_event_id,
              TradeEvent::Type exit_type,
              std::time_t entry_timestamp,
              std::time_t exit_timestamp,
              double position_size,
              double investment,
              double entry_price,
              double exit_price,
              double total_entry_fees,
              double total_exit_fees,
              double stop_price,
              double target_price,
              double stop_loss_price,
              double take_profit_price)
  : trade_id_{trade_id}
  , exit_event_id_{exit_event_id}
  , exit_type_{exit_type}
  , entry_timestamp_{entry_timestamp}
  , exit_timestamp_{exit_timestamp}
  , position_size_{position_size}
  , investment_{investment}
  , entry_price_{entry_price}
  , exit_price_{exit_price}
  , total_entry_fees_{total_entry_fees}
  , total_exit_fees_{total_exit_fees}
  , stop_price_{stop_price}
  , target_price_{target_price}
  , stop_loss_price_{stop_loss_price}
  , take_profit_price_{take_profit_price}
  {
  }

  auto operator==(const ClosedTrade&) const noexcept -> bool = default;

  auto trade_id(this const ClosedTrade& self) noexcept -> std::size_t
  {
    return self.trade_id_;
  }

  auto exit_event_id(this const ClosedTrade& self) noexcept -> std::size_t
  {
    return self.exit_event_id_;
  }

  auto exit_type(this const ClosedTrade& self) noexcept -> TradeEvent::Type
  {
    return self.exit_type_;
  }

  auto entry_timestamp(this const ClosedTrade& self) noexcept -> std::time_t
  {
    return self.entry_timestamp_;
  }

  auto exit_timestamp(this const ClosedTrade& self) noexcept -> std::time_t
  {
    return self.exit_timestamp_;
  }

  auto position_size(this const ClosedTrade& self) noexcept -> double
  {
    return self.position_size_;
  }

  auto investment(this const ClosedTrade& self) noexcept -> double
  {
    return self.investment_;
  }

  auto entry_price(this const ClosedTrade& self) noexcept -> double
  {
    return self.entry_price_;
  }

  auto exit_price(this const ClosedTrade& self) noexcept -> double
  {
    return self.exit_price_;
  }

  auto total_entry_fees(this const ClosedTrade& self) noexcept -> double
  {
    return self.total_entry_fees_;
  }

  auto total_exit_fees(this const ClosedTrade& self) noexcept -> double
  {
    return self.total_exit_fees_;
  }

  auto stop_price(this const ClosedTrade& self) noexcept -> double
  {
    return self.stop_price_;
  }

  auto target_price(this const ClosedTrade& self) noexcept -> double
  {
    return self.target_price_;
  }

  auto stop_loss_price(this const ClosedTrade& self) noexcept -> double
  {
    return self.stop_loss_price_;
  }

  auto take_profit_price(this const ClosedTrade& self) noexcept -> double
  {
    return self.take_profit_price_;
  }

  auto average_price(this const ClosedTrade& self) noexcept -> double
  {
    return self.position_size_ ? self.investment_ / self.position_size_ : 0.0;
  }

  auto exit_value(this const ClosedTrade& self) noexcept -> double
  {
    return self.position_size_ * self.exit_price_;
  }

  auto pnl(this const ClosedTrade& self) noexcept -> double
  {
    return self.position_size_
            ? self.exit_value() - self.total_exit_fees_ - self.investment_
            : 0.0;
  }

  auto duration(this const ClosedTrade& self) noexcept -> std::time_t
  {
    return self.exit_timestamp_ - self.entry_timestamp_;
  }

private:
  std::size_t trade_id_{};
  std::size_t exit_event_id_{};
  TradeEvent::Type exit_type_{TradeEvent::Type::exit_signal};

  std::time_t entry_timestamp_{};
  std::time_t exit_timestamp_{};
  double position_size_{};
  double investment_{};
  double entry_price_{};
  double exit_price_{};
  double total_entry_fees_{};
  double total_exit_fees_{};
  double stop_price_{};
  double target_price_{};
  double stop_loss_price_{};
  double take_profit_price_{};
};

} // namespace pludux::backtest
