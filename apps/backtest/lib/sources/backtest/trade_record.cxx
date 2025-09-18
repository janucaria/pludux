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
              double take_profit_price);

  auto status() const noexcept -> Status;

  void status(Status status) noexcept;

  auto position_size() const noexcept -> double;

  void position_size(double size) noexcept;

  auto entry_index() const noexcept -> std::size_t;

  void entry_index(std::size_t index) noexcept;

  auto exit_index() const noexcept -> std::size_t;

  auto exit_index(std::size_t index) noexcept -> void;

  auto entry_price() const noexcept -> double;

  void entry_price(double price) noexcept;

  auto average_price() const noexcept -> double;

  void average_price(double price) noexcept;

  auto exit_price() const noexcept -> double;

  void exit_price(double price) noexcept;

  auto stop_loss_price() const noexcept -> double;

  void stop_loss_price(double price) noexcept;

  auto trailing_stop_price() const noexcept -> double;

  void trailing_stop_price(double price) noexcept;

  auto take_profit_price() const noexcept -> double;

  void take_profit_price(double price) noexcept;

  auto entry_timestamp() const noexcept -> std::time_t;

  void entry_timestamp(std::time_t timestamp) noexcept;

  auto exit_timestamp() const noexcept -> std::time_t;

  void exit_timestamp(std::time_t timestamp) noexcept;

  auto entry_value() const noexcept -> double;

  auto exit_value() const noexcept -> double;

  auto investment() const noexcept -> double;

  auto pnl() const noexcept -> double;

  auto duration() const noexcept -> std::time_t;

  auto is_open() const noexcept -> bool;

  auto is_closed() const noexcept -> bool;

  auto is_closed_exit_signal() const noexcept -> bool;

  auto is_closed_take_profit() const noexcept -> bool;

  auto is_closed_stop_loss() const noexcept -> bool;

  auto is_scaled() const noexcept -> bool;

  auto is_scaled_in() const noexcept -> bool;

  auto is_scaled_out() const noexcept -> bool;

  auto is_long_position() const noexcept -> bool;

  auto is_short_position() const noexcept -> bool;

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

// -------------------------------------------------------------------

TradeRecord::TradeRecord(Status status,
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

auto TradeRecord::status() const noexcept -> Status
{
  return status_;
}

void TradeRecord::status(Status status) noexcept
{
  status_ = status;
}

auto TradeRecord::position_size() const noexcept -> double
{
  return position_size_;
}

void TradeRecord::position_size(double size) noexcept
{
  position_size_ = size;
}

auto TradeRecord::entry_index() const noexcept -> std::size_t
{
  return entry_index_;
}

void TradeRecord::entry_index(std::size_t index) noexcept
{
  entry_index_ = index;
}

auto TradeRecord::exit_index() const noexcept -> std::size_t
{
  return exit_index_;
}

void TradeRecord::exit_index(std::size_t index) noexcept
{
  exit_index_ = index;
}

auto TradeRecord::entry_price() const noexcept -> double
{
  return entry_price_;
}

void TradeRecord::entry_price(double price) noexcept
{
  entry_price_ = price;
}

auto TradeRecord::average_price() const noexcept -> double
{
  return average_price_;
}

void TradeRecord::average_price(double price) noexcept
{
  average_price_ = price;
}

auto TradeRecord::exit_price() const noexcept -> double
{
  return exit_price_;
}

void TradeRecord::exit_price(double price) noexcept
{
  exit_price_ = price;
}

auto TradeRecord::entry_timestamp() const noexcept -> std::time_t
{
  return entry_timestamp_;
}

void TradeRecord::entry_timestamp(std::time_t timestamp) noexcept
{
  entry_timestamp_ = timestamp;
}

auto TradeRecord::exit_timestamp() const noexcept -> std::time_t
{
  return exit_timestamp_;
}

void TradeRecord::exit_timestamp(std::time_t timestamp) noexcept
{
  exit_timestamp_ = timestamp;
}

auto TradeRecord::stop_loss_price() const noexcept -> double
{
  return stop_loss_price_;
}

void TradeRecord::stop_loss_price(double price) noexcept
{
  stop_loss_price_ = price;
}

auto TradeRecord::trailing_stop_price() const noexcept -> double
{
  return trailing_stop_price_;
}

void TradeRecord::trailing_stop_price(double price) noexcept
{
  trailing_stop_price_ = price;
}

auto TradeRecord::take_profit_price() const noexcept -> double
{
  return take_profit_price_;
}

void TradeRecord::take_profit_price(double price) noexcept
{
  take_profit_price_ = price;
}

auto TradeRecord::entry_value() const noexcept -> double
{
  return position_size() * entry_price();
}

auto TradeRecord::exit_value() const noexcept -> double
{
  return position_size() * exit_price();
}

auto TradeRecord::investment() const noexcept -> double
{
  return position_size() * average_price();
}

auto TradeRecord::pnl() const noexcept -> double
{
  return exit_value() - investment();
}

auto TradeRecord::duration() const noexcept -> std::time_t
{
  return exit_timestamp() - entry_timestamp();
}

auto TradeRecord::is_open() const noexcept -> bool
{
  return status_ == Status::open;
}

auto TradeRecord::is_closed() const noexcept -> bool
{
  return is_closed_exit_signal() || is_closed_take_profit() ||
         is_closed_stop_loss();
}

auto TradeRecord::is_closed_exit_signal() const noexcept -> bool
{
  return status_ == Status::closed_exit_signal;
}

auto TradeRecord::is_closed_take_profit() const noexcept -> bool
{
  return status_ == Status::closed_take_profit;
}

auto TradeRecord::is_closed_stop_loss() const noexcept -> bool
{
  return status_ == Status::closed_stop_loss;
}

auto TradeRecord::is_scaled() const noexcept -> bool
{
  return is_scaled_in() || is_scaled_out();
}

auto TradeRecord::is_scaled_in() const noexcept -> bool
{
  return status_ == Status::scaled_in;
}

auto TradeRecord::is_scaled_out() const noexcept -> bool
{
  return status_ == Status::scaled_out;
}

auto TradeRecord::is_long_position() const noexcept -> bool
{
  return position_size_ > 0.0;
}

auto TradeRecord::is_short_position() const noexcept -> bool
{
  return position_size_ < 0.0;
}

} // namespace pludux::backtest