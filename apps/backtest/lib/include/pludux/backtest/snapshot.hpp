#ifndef PLUDUX_PLUDUX_BACKTEST_SNAPSHOT_HPP
#define PLUDUX_PLUDUX_BACKTEST_SNAPSHOT_HPP

#include <optional>

#include "./trade_record.hpp"

namespace pludux::backtest {

class Snapshot {
public:
  Snapshot(double timestamp,
           double open,
           double high,
           double low,
           double close,
           double volume,
           std::optional<TradeRecord> trade_record);

  auto timestamp() const noexcept -> double;

  void timestamp(double timestamp) noexcept;

  auto open() const noexcept -> double;

  void open(double open) noexcept;

  auto high() const noexcept -> double;

  void high(double high) noexcept;

  auto low() const noexcept -> double;

  void low(double low) noexcept;

  auto close() const noexcept -> double;

  void close(double close) noexcept;

  auto volume() const noexcept -> double;

  void volume(double volume) noexcept;

  auto trade_record() const noexcept -> const std::optional<TradeRecord>&;

  void trade_record(std::optional<TradeRecord> trade_record) noexcept;

private:
  double timestamp_;
  double open_;
  double high_;
  double low_;
  double close_;
  double volume_;
  std::optional<TradeRecord> trade_record_;
};

} // namespace pludux::backtest

#endif
