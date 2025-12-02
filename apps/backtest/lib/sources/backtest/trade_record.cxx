module;
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <limits>

export module pludux.backtest:trade_record;

export namespace pludux::backtest {

class TradeRecord {
public:
  enum class Status {
    open,
    closed_exit_signal,
    closed_take_profit,
    closed_stop_loss,
    scaled_in,
    scaled_out
  };

  TradeRecord(Status status,
              double position_size,
              double average_price,

              std::time_t entry_timestamp,
              double entry_price,
              std::size_t entry_index,

              std::time_t exit_timestamp,
              double exit_price,
              std::size_t exit_index,

              double stop_loss_price,
              double trailing_stop_price,
              double take_profit_price)
  : status_{status}
  , position_size_{position_size}
  , average_price_{average_price}
  , entry_price_{entry_price}
  , exit_price_{exit_price}
  , stop_loss_price_{stop_loss_price}
  , trailing_stop_price_{trailing_stop_price}
  , take_profit_price_{take_profit_price}
  , entry_index_{entry_index}
  , exit_index_{exit_index}
  , entry_timestamp_{entry_timestamp}
  , exit_timestamp_{exit_timestamp}
  {
  }

  auto status(this const TradeRecord& self) noexcept -> Status
  {
    return self.status_;
  }

  void status(this TradeRecord& self, Status status) noexcept
  {
    self.status_ = status;
  }

  auto position_size(this const TradeRecord& self) noexcept -> double
  {
    return self.position_size_;
  }

  void position_size(this TradeRecord& self, double size) noexcept
  {
    self.position_size_ = size;
  }

  auto entry_index(this const TradeRecord& self) noexcept -> std::size_t
  {
    return self.entry_index_;
  }

  void entry_index(this TradeRecord& self, std::size_t index) noexcept
  {
    self.entry_index_ = index;
  }

  auto exit_index(this const TradeRecord& self) noexcept -> std::size_t
  {
    return self.exit_index_;
  }

  void exit_index(this TradeRecord& self, std::size_t index) noexcept
  {
    self.exit_index_ = index;
  }

  auto entry_price(this const TradeRecord& self) noexcept -> double
  {
    return self.entry_price_;
  }

  void entry_price(this TradeRecord& self, double price) noexcept
  {
    self.entry_price_ = price;
  }

  auto average_price(this const TradeRecord& self) noexcept -> double
  {
    return self.average_price_;
  }

  void average_price(this TradeRecord& self, double price) noexcept
  {
    self.average_price_ = price;
  }

  auto exit_price(this const TradeRecord& self) noexcept -> double
  {
    return self.exit_price_;
  }

  void exit_price(this TradeRecord& self, double price) noexcept
  {
    self.exit_price_ = price;
  }

  auto entry_timestamp(this const TradeRecord& self) noexcept -> std::time_t
  {
    return self.entry_timestamp_;
  }

  void entry_timestamp(this TradeRecord& self, std::time_t timestamp) noexcept
  {
    self.entry_timestamp_ = timestamp;
  }

  auto exit_timestamp(this const TradeRecord& self) noexcept -> std::time_t
  {
    return self.exit_timestamp_;
  }

  void exit_timestamp(this TradeRecord& self, std::time_t timestamp) noexcept
  {
    self.exit_timestamp_ = timestamp;
  }

  auto stop_loss_price(this const TradeRecord& self) noexcept -> double
  {
    return self.stop_loss_price_;
  }

  void stop_loss_price(this TradeRecord& self, double price) noexcept
  {
    self.stop_loss_price_ = price;
  }

  auto trailing_stop_price(this const TradeRecord& self) noexcept -> double
  {
    return self.trailing_stop_price_;
  }

  void trailing_stop_price(this TradeRecord& self, double price) noexcept
  {
    self.trailing_stop_price_ = price;
  }

  auto take_profit_price(this const TradeRecord& self) noexcept -> double
  {
    return self.take_profit_price_;
  }

  void take_profit_price(this TradeRecord& self, double price) noexcept
  {
    self.take_profit_price_ = price;
  }

  auto entry_value(this const TradeRecord& self) noexcept -> double
  {
    return self.position_size() * self.entry_price();
  }

  auto exit_value(this const TradeRecord& self) noexcept -> double
  {
    return self.position_size() * self.exit_price();
  }

  auto investment(this const TradeRecord& self) noexcept -> double
  {
    return self.position_size() * self.average_price();
  }

  auto pnl(this const TradeRecord& self) noexcept -> double
  {
    return self.exit_value() - self.investment();
  }

  auto duration(this const TradeRecord& self) noexcept -> std::time_t
  {
    return self.exit_timestamp() - self.entry_timestamp();
  }

  auto is_open(this const TradeRecord& self) noexcept -> bool
  {
    return self.status_ == Status::open;
  }

  auto is_closed(this const TradeRecord& self) noexcept -> bool
  {
    return self.is_closed_exit_signal() || self.is_closed_take_profit() ||
           self.is_closed_stop_loss();
  }

  auto is_closed_exit_signal(this const TradeRecord& self) noexcept -> bool
  {
    return self.status_ == Status::closed_exit_signal;
  }

  auto is_closed_take_profit(this const TradeRecord& self) noexcept -> bool
  {
    return self.status_ == Status::closed_take_profit;
  }

  auto is_closed_stop_loss(this const TradeRecord& self) noexcept -> bool
  {
    return self.status_ == Status::closed_stop_loss;
  }

  auto is_scaled(this const TradeRecord& self) noexcept -> bool
  {
    return self.is_scaled_in() || self.is_scaled_out();
  }

  auto is_scaled_in(this const TradeRecord& self) noexcept -> bool
  {
    return self.status_ == Status::scaled_in;
  }

  auto is_scaled_out(this const TradeRecord& self) noexcept -> bool
  {
    return self.status_ == Status::scaled_out;
  }

  auto is_long_position(this const TradeRecord& self) noexcept -> bool
  {
    return self.position_size_ > 0.0;
  }

  auto is_short_position(this const TradeRecord& self) noexcept -> bool
  {
    return self.position_size_ < 0.0;
  }

private:
  Status status_;
  double position_size_;
  double average_price_;

  double entry_price_;
  double exit_price_;

  double stop_loss_price_;
  double trailing_stop_price_;
  double take_profit_price_;

  std::size_t entry_index_;
  std::size_t exit_index_;

  std::time_t entry_timestamp_;
  std::time_t exit_timestamp_;
};

} // namespace pludux::backtest