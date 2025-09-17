module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <functional>
#include <iterator>
#include <limits>
#include <numeric>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

#include <pludux/backtest/trade_entry.hpp>
#include <pludux/backtest/trade_exit.hpp>
#include <pludux/backtest/trade_position.hpp>
#include <pludux/backtest/trade_record.hpp>

export module pludux.backtest.trade_session;

export namespace pludux::backtest {

class TradeSession {
public:
  class ConstTradeRecordIterator {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = TradeRecord;
    using difference_type = std::ptrdiff_t;

    ConstTradeRecordIterator(const TradeSession& session);

    auto operator*() const -> TradeRecord;

    auto operator++() -> ConstTradeRecordIterator&;

    auto operator++(int) -> ConstTradeRecordIterator;

    auto operator==(std::default_sentinel_t) const -> bool;

  private:
    const TradeSession& session_;
    std::size_t closed_position_index_;
    std::size_t open_position_index_;
    bool waiting_for_open_record_;
  };

  class ConstTradeRecordRange {
  public:
    ConstTradeRecordRange(const TradeSession& session);

    auto begin() const -> ConstTradeRecordIterator;

    auto end() const -> std::default_sentinel_t;

  private:
    const TradeSession& session_;
  };

  TradeSession();

  TradeSession(std::time_t market_timestamp_,
               double market_price_,
               std::size_t market_index_);

  TradeSession(std::time_t market_timestamp_,
               double market_price_,
               std::size_t market_index_,
               std::optional<TradePosition> open_position,
               std::optional<TradePosition> closed_position);

  auto market_timestamp() const noexcept -> std::time_t;

  void market_timestamp(std::time_t timestamp) noexcept;

  auto market_price() const noexcept -> double;

  void market_price(double price) noexcept;

  auto market_index() const noexcept -> std::size_t;

  void market_index(std::size_t index) noexcept;

  auto open_position() const noexcept -> const std::optional<TradePosition>&;

  void open_position(std::optional<TradePosition> position) noexcept;

  auto closed_position() const noexcept -> const std::optional<TradePosition>&;

  void closed_position(std::optional<TradePosition> position) noexcept;

  void entry_position(const TradeEntry& entry) noexcept;

  void exit_position(const TradeExit& exit);

  void market_update(std::time_t timestamp,
                     double price,
                     std::size_t index) noexcept;

  auto trade_record_range() const noexcept -> ConstTradeRecordRange
  {
    return ConstTradeRecordRange{*this};
  }

  void evaluate_exit_conditions(double prev_close,
                                double open,
                                double high,
                                double low) noexcept;

  auto unrealized_pnl() const noexcept -> double;

  auto partial_realized_pnl() const noexcept -> double;

  auto unrealized_investment() const noexcept -> double;

  auto unrealized_duration() const noexcept -> std::time_t;

  auto realized_pnl() const noexcept -> double;

  auto realized_investment() const noexcept -> double;

  auto realized_duration() const noexcept -> std::time_t;

  auto is_flat() const noexcept -> bool;

  auto is_open() const noexcept -> bool;

  auto is_closed() const noexcept -> bool;

  auto is_reopen() const noexcept -> bool;

private:
  std::time_t market_timestamp_;
  double market_price_;
  std::size_t market_index_;

  std::optional<TradePosition> open_position_;

  std::optional<TradePosition> closed_position_;
};

// --------------------------------------------------------------------

TradeSession::TradeSession()
: TradeSession{0, NAN, 0}
{
}

TradeSession::TradeSession(std::time_t market_timestamp_,
                           double market_price_,
                           std::size_t market_index_)
: TradeSession{
   market_timestamp_, market_price_, market_index_, std::nullopt, std::nullopt}
{
}

TradeSession::TradeSession(std::time_t market_timestamp_,
                           double market_price_,
                           std::size_t market_index_,
                           std::optional<TradePosition> open_position,
                           std::optional<TradePosition> closed_position)
: market_timestamp_{market_timestamp_}
, market_price_{market_price_}
, market_index_{market_index_}
, open_position_{std::move(open_position)}
, closed_position_{std::move(closed_position)}
{
}

auto TradeSession::market_timestamp() const noexcept -> std::time_t
{
  return market_timestamp_;
}

void TradeSession::market_timestamp(std::time_t timestamp) noexcept
{
  market_timestamp_ = timestamp;
}

auto TradeSession::market_price() const noexcept -> double
{
  return market_price_;
}

void TradeSession::market_price(double price) noexcept
{
  market_price_ = price;
}

auto TradeSession::market_index() const noexcept -> std::size_t
{
  return market_index_;
}

void TradeSession::market_index(std::size_t index) noexcept
{
  market_index_ = index;
}

auto TradeSession::open_position() const noexcept
 -> const std::optional<TradePosition>&
{
  return open_position_;
}

void TradeSession::open_position(std::optional<TradePosition> position) noexcept
{
  open_position_ = std::move(position);
}

auto TradeSession::closed_position() const noexcept
 -> const std::optional<TradePosition>&
{
  return closed_position_;
}

void TradeSession::closed_position(
 std::optional<TradePosition> position) noexcept
{
  closed_position_ = std::move(position);
}

void TradeSession::entry_position(const TradeEntry& entry) noexcept
{
  if(open_position_) {
    open_position_->scaled_in(
     entry.position_size(), market_timestamp_, entry.price(), market_index_);

    return;
  }

  open_position_ = TradePosition{entry.position_size(),
                                 market_timestamp_,
                                 entry.price(),
                                 market_index_,
                                 entry.stop_loss_price(),
                                 entry.stop_loss_trailing_price(),
                                 entry.take_profit_price()};
}

void TradeSession::exit_position(const TradeExit& exit)
{
  if(!open_position_) {
    throw std::runtime_error("Cannot exit a closed trade.");
  }

  const auto trade_status = [](TradeExit::Reason reason) {
    switch(reason) {
    case TradeExit::Reason::stop_loss:
      return TradeRecord::Status::closed_stop_loss;
    case TradeExit::Reason::take_profit:
      return TradeRecord::Status::closed_take_profit;
    case TradeExit::Reason::signal:
    default:
      return TradeRecord::Status::closed_exit_signal;
    }
  }(exit.reason());

  open_position_->scaled_out(exit.position_size(),
                             market_timestamp_,
                             exit.price(),
                             market_index_,
                             trade_status);

  if(open_position_->is_closed()) {
    closed_position_ = std::move(*open_position_);
    open_position_ = std::nullopt;
  }
}

void TradeSession::market_update(std::time_t timestamp,
                                 double price,
                                 std::size_t index) noexcept
{
  market_timestamp(timestamp);
  market_price(price);
  market_index(index);
}

void TradeSession::evaluate_exit_conditions(double prev_close,
                                            double open,
                                            double high,
                                            double low) noexcept
{
  if(is_flat()) {
    return;
  }

  if(closed_position_) {
    closed_position_ = std::nullopt;
  }

  if(!open_position_) {
    return;
  }

  const auto position_size = open_position_->unrealized_position_size();

  if(open_position_->trigger_stop_loss(prev_close, high, low)) {
    const auto exit_price =
     open_position_->is_long_direction()
      ? std::min(open, open_position_->stop_loss_price())
      : std::max(open, open_position_->stop_loss_price());
    auto trade_exit =
     TradeExit{position_size, exit_price, TradeExit::Reason::stop_loss};

    exit_position(trade_exit);
  } else if(open_position_->trigger_take_profit(high, low)) {
    const auto exit_price =
     open_position_->is_long_direction()
      ? std::max(open, open_position_->take_profit_price())
      : std::min(open, open_position_->take_profit_price());
    auto trade_exit =
     TradeExit{position_size, exit_price, TradeExit::Reason::take_profit};

    exit_position(trade_exit);
  }
}

auto TradeSession::unrealized_pnl() const noexcept -> double
{
  return open_position_ ? open_position_->unrealized_pnl(market_price_) : 0.0;
}

auto TradeSession::partial_realized_pnl() const noexcept -> double
{
  return open_position_ ? open_position_->realized_pnl() : 0.0;
}

auto TradeSession::unrealized_investment() const noexcept -> double
{
  return open_position_ ? open_position_->unrealized_investment() : 0.0;
}

auto TradeSession::unrealized_duration() const noexcept -> std::time_t
{
  return open_position_ ? open_position_->unrealized_duration(market_timestamp_)
                        : 0;
}

auto TradeSession::realized_pnl() const noexcept -> double
{
  return closed_position_ ? closed_position_->realized_pnl() : 0.0;
}

auto TradeSession::realized_investment() const noexcept -> double
{
  return closed_position_ ? closed_position_->realized_investment() : 0.0;
}

auto TradeSession::realized_duration() const noexcept -> std::time_t
{
  return closed_position_ ? closed_position_->realized_duration() : 0;
}

auto TradeSession::is_flat() const noexcept -> bool
{
  return !open_position_ && !closed_position_;
}

auto TradeSession::is_open() const noexcept -> bool
{
  return open_position_.has_value();
}

auto TradeSession::is_closed() const noexcept -> bool
{
  return !is_open() && closed_position_.has_value();
}

auto TradeSession::is_reopen() const noexcept -> bool
{
  return open_position_.has_value() && closed_position_.has_value();
}

// -------------------------------------------------------------------

TradeSession::ConstTradeRecordIterator::ConstTradeRecordIterator(
 const TradeSession& session)
: session_{session}
, closed_position_index_{0}
, open_position_index_{0}
, waiting_for_open_record_{session.open_position().has_value()}
{
}

auto TradeSession::ConstTradeRecordIterator::operator*() const -> TradeRecord
{
  const auto& closed_position = session_.closed_position();
  if(closed_position &&
     closed_position_index_ < closed_position->realized_records().size()) {
    return closed_position->realized_records().at(closed_position_index_);
  }

  const auto& open_position = session_.open_position();
  if(open_position_index_ < open_position->realized_records().size()) {
    return open_position->realized_records().at(open_position_index_);
  }

  return TradeRecord{TradeRecord::Status::open,
                     open_position->position_size(),
                     open_position->average_price(),
                     open_position->entry_timestamp(),
                     open_position->entry_price(),
                     open_position->entry_index(),
                     session_.market_timestamp(),
                     session_.market_price(),
                     session_.market_index(),
                     open_position->stop_loss_initial_price(),
                     open_position->stop_loss_trailing_price(),
                     open_position->take_profit_price()};
}

auto TradeSession::ConstTradeRecordIterator::operator++()
 -> ConstTradeRecordIterator&
{
  const auto& closed_position = session_.closed_position();
  if(closed_position &&
     closed_position_index_ < closed_position->realized_records().size()) {
    closed_position_index_++;
  } else {
    const auto& open_position = session_.open_position();
    if(open_position &&
       open_position_index_ < open_position->realized_records().size()) {
      open_position_index_++;
    } else if(waiting_for_open_record_) {
      waiting_for_open_record_ = false;
    }
  }

  return *this;
}

auto TradeSession::ConstTradeRecordIterator::operator++(int)
 -> ConstTradeRecordIterator
{
  auto tmp = *this;
  ++(*this);
  return tmp;
}

auto TradeSession::ConstTradeRecordIterator::operator==(
 std::default_sentinel_t) const -> bool
{
  const auto& closed_position = session_.closed_position();
  const auto& open_position = session_.open_position();

  const auto closed_position_size =
   closed_position ? closed_position->realized_records().size() : 0;
  const auto open_position_size =
   open_position ? open_position->realized_records().size() : 0;

  return closed_position_index_ >= closed_position_size &&
         open_position_index_ >= open_position_size &&
         !waiting_for_open_record_;
}

// -----------------------------------------------------------------------

TradeSession::ConstTradeRecordRange::ConstTradeRecordRange(
 const TradeSession& session)
: session_(session)
{
}

auto TradeSession::ConstTradeRecordRange::begin() const
 -> ConstTradeRecordIterator
{
  return ConstTradeRecordIterator{session_};
}

auto TradeSession::ConstTradeRecordRange::end() const -> std::default_sentinel_t
{
  return std::default_sentinel;
}

} // namespace pludux::backtest
