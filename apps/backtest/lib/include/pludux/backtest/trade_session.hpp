#ifndef PLUDUX_PLUDUX_BACKTEST_TRADE_SESSION_HPP
#define PLUDUX_PLUDUX_BACKTEST_TRADE_SESSION_HPP

#include <cmath>
#include <cstddef>
#include <ctime>
#include <functional>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <vector>

#include <pludux/backtest/trade_entry.hpp>
#include <pludux/backtest/trade_exit.hpp>
#include <pludux/backtest/trade_position.hpp>
#include <pludux/backtest/trade_record.hpp>

namespace pludux::backtest {

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

} // namespace pludux::backtest

#endif
