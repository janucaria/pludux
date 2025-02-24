#include <utility>

#include <pludux/backtest/snapshot.hpp>

namespace pludux::backtest {

Snapshot::Snapshot(double timestamp,
                   double open,
                   double high,
                   double low,
                   double close,
                   double volume,
                   std::optional<TradeRecord> trade_record)
: timestamp_{timestamp}
, open_{open}
, high_{high}
, low_{low}
, close_{close}
, volume_{volume}
, trade_record_{std::move(trade_record)}
{
}

auto Snapshot::timestamp() const noexcept -> double
{
  return timestamp_;
}

void Snapshot::timestamp(double timestamp) noexcept
{
  timestamp_ = timestamp;
}

auto Snapshot::open() const noexcept -> double
{
  return open_;
}

void Snapshot::open(double open) noexcept
{
  open_ = open;
}

auto Snapshot::high() const noexcept -> double
{
  return high_;
}

void Snapshot::high(double high) noexcept
{
  high_ = high;
}

auto Snapshot::low() const noexcept -> double
{
  return low_;
}

void Snapshot::low(double low) noexcept
{
  low_ = low;
}

auto Snapshot::close() const noexcept -> double
{
  return close_;
}

void Snapshot::close(double close) noexcept
{
  close_ = close;
}

auto Snapshot::volume() const noexcept -> double
{
  return volume_;
}

void Snapshot::volume(double volume) noexcept
{
  volume_ = volume;
}

auto Snapshot::trade_record() const noexcept
 -> const std::optional<TradeRecord>&
{
  return trade_record_;
}

void Snapshot::trade_record(std::optional<TradeRecord> trade_record) noexcept
{
  trade_record_ = std::move(trade_record);
}

} // namespace pludux::backtest