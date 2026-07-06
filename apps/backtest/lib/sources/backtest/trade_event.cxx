module;

#include <cstddef>
#include <ctime>

export module pludux.backtest:trade_event;

export namespace pludux::backtest {

class TradeEvent {
public:
  enum class Type {
    entry,
    scale_in,
    scale_out,
    exit_signal,
    stop_loss,
    take_profit
  };

  TradeEvent() = default;

  TradeEvent(std::size_t trade_id,
             std::size_t event_id,
             std::size_t trade_event_index,
             Type type,
             std::time_t timestamp,
             double price,
             double position_size,
             double fees,
             double position_size_before,
             double investment_before,
             double average_price_before,
             double position_size_after,
             double investment_after,
             double average_price_after,
             double stop_price,
             double target_price,
             double stop_loss_price,
             double take_profit_price)
  : trade_id_{trade_id}
  , event_id_{event_id}
  , trade_event_index_{trade_event_index}
  , type_{type}
  , timestamp_{timestamp}
  , price_{price}
  , position_size_{position_size}
  , fees_{fees}
  , position_size_before_{position_size_before}
  , investment_before_{investment_before}
  , average_price_before_{average_price_before}
  , position_size_after_{position_size_after}
  , investment_after_{investment_after}
  , average_price_after_{average_price_after}
  , stop_price_{stop_price}
  , target_price_{target_price}
  , stop_loss_price_{stop_loss_price}
  , take_profit_price_{take_profit_price}
  {
  }

  auto operator==(const TradeEvent&) const noexcept -> bool = default;

  auto trade_id(this const TradeEvent& self) noexcept -> std::size_t
  {
    return self.trade_id_;
  }

  auto event_id(this const TradeEvent& self) noexcept -> std::size_t
  {
    return self.event_id_;
  }

  auto trade_event_index(this const TradeEvent& self) noexcept -> std::size_t
  {
    return self.trade_event_index_;
  }

  auto type(this const TradeEvent& self) noexcept -> Type
  {
    return self.type_;
  }

  auto timestamp(this const TradeEvent& self) noexcept -> std::time_t
  {
    return self.timestamp_;
  }

  auto price(this const TradeEvent& self) noexcept -> double
  {
    return self.price_;
  }

  auto position_size(this const TradeEvent& self) noexcept -> double
  {
    return self.position_size_;
  }

  auto fees(this const TradeEvent& self) noexcept -> double
  {
    return self.fees_;
  }

  auto position_size_before(this const TradeEvent& self) noexcept -> double
  {
    return self.position_size_before_;
  }

  auto investment_before(this const TradeEvent& self) noexcept -> double
  {
    return self.investment_before_;
  }

  auto average_price_before(this const TradeEvent& self) noexcept -> double
  {
    return self.average_price_before_;
  }

  auto position_size_after(this const TradeEvent& self) noexcept -> double
  {
    return self.position_size_after_;
  }

  auto investment_after(this const TradeEvent& self) noexcept -> double
  {
    return self.investment_after_;
  }

  auto average_price_after(this const TradeEvent& self) noexcept -> double
  {
    return self.average_price_after_;
  }

  auto stop_price(this const TradeEvent& self) noexcept -> double
  {
    return self.stop_price_;
  }

  auto target_price(this const TradeEvent& self) noexcept -> double
  {
    return self.target_price_;
  }

  auto stop_loss_price(this const TradeEvent& self) noexcept -> double
  {
    return self.stop_loss_price_;
  }

  auto take_profit_price(this const TradeEvent& self) noexcept -> double
  {
    return self.take_profit_price_;
  }

  void after_state(this TradeEvent& self,
                   double position_size,
                   double investment,
                   double average_price,
                   double stop_price,
                   double target_price,
                   double stop_loss_price,
                   double take_profit_price) noexcept
  {
    self.position_size_after_ = position_size;
    self.investment_after_ = investment;
    self.average_price_after_ = average_price;
    self.stop_price_ = stop_price;
    self.target_price_ = target_price;
    self.stop_loss_price_ = stop_loss_price;
    self.take_profit_price_ = take_profit_price;
  }

  auto is_entry(this const TradeEvent& self) noexcept -> bool
  {
    return self.type_ == Type::entry;
  }

  auto is_scale_in(this const TradeEvent& self) noexcept -> bool
  {
    return self.type_ == Type::scale_in;
  }

  auto is_scale_out(this const TradeEvent& self) noexcept -> bool
  {
    return self.type_ == Type::scale_out;
  }

  auto is_exit(this const TradeEvent& self) noexcept -> bool
  {
    return self.type_ == Type::exit_signal || self.type_ == Type::stop_loss ||
           self.type_ == Type::take_profit;
  }

private:
  std::size_t trade_id_{};
  std::size_t event_id_{};
  std::size_t trade_event_index_{};
  Type type_{Type::entry};

  std::time_t timestamp_{};
  double price_{};
  double position_size_{};
  double fees_{};

  double position_size_before_{};
  double investment_before_{};
  double average_price_before_{};

  double position_size_after_{};
  double investment_after_{};
  double average_price_after_{};

  double stop_price_{};
  double target_price_{};
  double stop_loss_price_{};
  double take_profit_price_{};
};

} // namespace pludux::backtest
