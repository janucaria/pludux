module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

#include <pludux/backtest/trade_record.hpp>

export module pludux.backtest.trade_position;

export namespace pludux::backtest {

class TradePosition {
public:
  TradePosition(double position_size,
                std::time_t entry_timestamp,
                double entry_price,
                std::size_t entry_index,
                double stop_loss_initial_price,
                double stop_loss_trailing_price,
                double take_profit_price);

  TradePosition(double position_size,
                double average_price,
                std::time_t entry_timestamp,
                double entry_price,
                std::size_t entry_index,
                double stop_loss_initial_price,
                double stop_loss_trailing_price,
                double take_profit_price,
                std::vector<TradeRecord> realized_records);

  auto position_size() const noexcept -> double;

  void position_size(double size) noexcept;

  auto average_price() const noexcept -> double;

  void average_price(double price) noexcept;

  auto entry_price() const noexcept -> double;

  void entry_price(double price) noexcept;

  auto stop_loss_initial_price() const noexcept -> double;

  void stop_loss_initial_price(double price) noexcept;

  auto stop_loss_trailing_price() const noexcept -> double;

  void stop_loss_trailing_price(double price) noexcept;

  auto take_profit_price() const noexcept -> double;

  void take_profit_price(double price) noexcept;

  auto entry_index() const noexcept -> std::size_t;

  void entry_index(std::size_t index) noexcept;

  auto entry_timestamp() const noexcept -> std::time_t;

  void entry_timestamp(std::time_t timestamp) noexcept;

  auto realized_records() const noexcept -> const std::vector<TradeRecord>&;

  void realized_records(std::vector<TradeRecord> records) noexcept;

  auto stop_loss_price() const noexcept -> double;

  auto unrealized_position_size() const noexcept -> double;

  auto unrealized_investment() const noexcept -> double;

  auto unrealized_pnl(double market_price) const noexcept -> double;

  auto unrealized_duration(std::time_t market_timestamp) const noexcept
   -> std::time_t;

  auto realized_position_size() const noexcept -> double;

  auto realized_investment() const noexcept -> double;

  auto realized_pnl() const noexcept -> double;

  auto realized_duration() const noexcept -> std::time_t;

  auto is_closed() const noexcept -> bool;

  auto is_long_direction() const noexcept -> bool;

  auto is_short_direction() const noexcept -> bool;

  void scaled_in(double action_position_size,
                 std::time_t action_timestamp,
                 double action_price,
                 std::size_t action_index);

  void scaled_out(double action_position_size,
                  std::time_t action_timestamp,
                  double action_price,
                  std::size_t action_index,
                  TradeRecord::Status trade_status);

  auto trigger_stop_loss(double prev_close, double high, double low) -> bool;

  auto trigger_take_profit(double high, double low) -> bool;

private:
  double position_size_;
  double average_price_;

  double entry_price_;

  double stop_loss_initial_price_;
  double stop_loss_trailing_price_;
  double take_profit_price_;

  std::size_t entry_index_;
  std::time_t entry_timestamp_;

  std::vector<TradeRecord> realized_records_;
};

// -------------------------------------------------------------------

TradePosition::TradePosition(double position_size,
                             std::time_t entry_timestamp,
                             double entry_price,
                             std::size_t entry_index,
                             double stop_loss_initial_price,
                             double stop_loss_trailing_price,
                             double take_profit_price)
: TradePosition{position_size,
                entry_price,
                entry_timestamp,
                entry_price,
                entry_index,
                stop_loss_initial_price,
                stop_loss_trailing_price,
                take_profit_price,
                {}}
{
}

TradePosition::TradePosition(double position_size,
                             double average_price,
                             std::time_t entry_timestamp,
                             double entry_price,
                             std::size_t entry_index,
                             double stop_loss_initial_price,
                             double stop_loss_trailing_price,
                             double take_profit_price,
                             std::vector<TradeRecord> realized_records)
: position_size_{position_size}
, average_price_{average_price}
, entry_price_{entry_price}
, stop_loss_initial_price_{stop_loss_initial_price}
, stop_loss_trailing_price_{stop_loss_trailing_price}
, take_profit_price_{take_profit_price}
, entry_timestamp_{entry_timestamp}
, entry_index_{entry_index}
, realized_records_{std::move(realized_records)}
{
}

auto TradePosition::position_size() const noexcept -> double
{
  return position_size_;
}

void TradePosition::position_size(double size) noexcept
{
  position_size_ = size;
}

auto TradePosition::average_price() const noexcept -> double
{
  return average_price_;
}

void TradePosition::average_price(double price) noexcept
{
  average_price_ = price;
}

auto TradePosition::entry_price() const noexcept -> double
{
  return entry_price_;
}

void TradePosition::entry_price(double price) noexcept
{
  entry_price_ = price;
}

auto TradePosition::stop_loss_initial_price() const noexcept -> double
{
  return stop_loss_initial_price_;
}

void TradePosition::stop_loss_initial_price(double price) noexcept
{
  stop_loss_initial_price_ = price;
}

auto TradePosition::stop_loss_trailing_price() const noexcept -> double
{
  return stop_loss_trailing_price_;
}

void TradePosition::stop_loss_trailing_price(double price) noexcept
{
  stop_loss_trailing_price_ = price;
}

auto TradePosition::take_profit_price() const noexcept -> double
{
  return take_profit_price_;
}

void TradePosition::take_profit_price(double price) noexcept
{
  take_profit_price_ = price;
}

auto TradePosition::entry_index() const noexcept -> std::size_t
{
  return entry_index_;
}

void TradePosition::entry_index(std::size_t index) noexcept
{
  entry_index_ = index;
}

auto TradePosition::entry_timestamp() const noexcept -> std::time_t
{
  return entry_timestamp_;
}

void TradePosition::entry_timestamp(std::time_t timestamp) noexcept
{
  entry_timestamp_ = timestamp;
}

auto TradePosition::realized_records() const noexcept
 -> const std::vector<TradeRecord>&
{
  return realized_records_;
}

void TradePosition::realized_records(std::vector<TradeRecord> records) noexcept
{
  realized_records_ = std::move(records);
}

auto TradePosition::stop_loss_price() const noexcept -> double
{
  return std::isnan(stop_loss_trailing_price()) ? stop_loss_initial_price()
                                                : stop_loss_trailing_price();
}

auto TradePosition::unrealized_position_size() const noexcept -> double
{
  return position_size();
}

auto TradePosition::unrealized_investment() const noexcept -> double
{
  return unrealized_position_size() * average_price();
}

auto TradePosition::unrealized_pnl(double market_price) const noexcept -> double
{
  return unrealized_position_size() * (market_price - average_price());
}

auto TradePosition::unrealized_duration(
 std::time_t market_timestamp) const noexcept -> std::time_t
{
  return market_timestamp - entry_timestamp();
}

auto TradePosition::realized_position_size() const noexcept -> double
{
  return std::ranges::fold_left(
   realized_records_, 0.0, [](double total, const TradeRecord& record) {
     return total + record.position_size();
   });
}

auto TradePosition::realized_investment() const noexcept -> double
{
  return std::ranges::fold_left(
   realized_records_, 0.0, [](double total, const TradeRecord& record) {
     return total + record.investment();
   });
}

auto TradePosition::realized_pnl() const noexcept -> double
{
  return std::ranges::fold_left(
   realized_records_, 0.0, [](double total, const TradeRecord& record) {
     return total + record.pnl();
   });
}

auto TradePosition::realized_duration() const noexcept -> std::time_t
{
  return std::ranges::fold_left(
   realized_records_,
   std::time_t{},
   [](std::time_t total, const TradeRecord& record) {
     return total + record.duration();
   });
}

auto TradePosition::is_closed() const noexcept -> bool
{
  return position_size() == 0;
}

auto TradePosition::is_long_direction() const noexcept -> bool
{
  return position_size() > 0;
}

auto TradePosition::is_short_direction() const noexcept -> bool
{
  return position_size() < 0;
}

void TradePosition::scaled_in(double action_position_size,
                              std::time_t action_timestamp,
                              double action_price,
                              std::size_t action_index)
{
  if(is_closed()) {
    throw std::runtime_error("Cannot scaled in to a closed trade.");
  }

  const auto last_position_size = position_size();
  const auto last_average_price = average_price();
  const auto last_investment = last_position_size * last_average_price;

  const auto new_position_size = last_position_size + action_position_size;
  const auto new_average_price =
   (last_investment + action_position_size * action_price) / new_position_size;

  auto scaled_in_record = TradeRecord{TradeRecord::Status::scaled_in,
                                      0.0,
                                      last_average_price,
                                      entry_timestamp(),
                                      entry_price(),
                                      entry_index(),
                                      action_timestamp,
                                      action_price,
                                      action_index,
                                      stop_loss_initial_price(),
                                      stop_loss_trailing_price(),
                                      take_profit_price()};

  position_size(new_position_size);
  average_price(new_average_price);

  realized_records_.emplace_back(std::move(scaled_in_record));
}

void TradePosition::scaled_out(double action_position_size,
                               std::time_t action_timestamp,
                               double action_price,
                               std::size_t action_index,
                               TradeRecord::Status trade_status)
{
  if(is_closed()) {
    throw std::runtime_error("Cannot add action to a closed trade.");
  }

  const auto last_position_size = position_size();
  const auto remaining_position_size =
   last_position_size - action_position_size;

  if(is_long_direction()) {
    if(remaining_position_size < 0) {
      throw std::runtime_error("Cannot reduce position size larger than "
                               "current long position size.");
    }
  } else if(is_short_direction()) {
    if(remaining_position_size > 0) {
      throw std::runtime_error("Cannot reduce position size larger than "
                               "current short position size.");
    }
  }

  auto exit_record = TradeRecord{trade_status,
                                 action_position_size,
                                 average_price(),
                                 entry_timestamp(),
                                 entry_price(),
                                 entry_index(),
                                 action_timestamp,
                                 action_price,
                                 action_index,
                                 stop_loss_initial_price(),
                                 stop_loss_trailing_price(),
                                 take_profit_price()};
  realized_records_.emplace_back(std::move(exit_record));

  position_size(remaining_position_size);
}

auto TradePosition::trigger_stop_loss(double prev_close,
                                      double high,
                                      double low) -> bool
{
  const auto initial_stop_price = stop_loss_initial_price();

  if(is_closed() || std::isnan(initial_stop_price)) {
    return false;
  }

  const auto is_short_position = is_short_direction();
  const auto risk = average_price() - initial_stop_price;
  const auto new_stop_price = prev_close - risk;

  if(is_short_position) {
    if(new_stop_price < stop_loss_trailing_price()) {
      stop_loss_trailing_price(new_stop_price);
    }

    return high >= stop_loss_price();
  }

  if(new_stop_price > stop_loss_trailing_price()) {
    stop_loss_trailing_price(new_stop_price);
  }

  return low <= stop_loss_price();
}

auto TradePosition::trigger_take_profit(double high, double low) -> bool
{
  const auto reference_price = take_profit_price();

  if(is_closed() || std::isnan(reference_price)) {
    return false;
  }

  return is_long_direction() ? reference_price < high : reference_price > low;
}

} // namespace pludux::backtest
