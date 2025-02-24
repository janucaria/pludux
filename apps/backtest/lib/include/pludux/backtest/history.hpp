#ifndef PLUDUX_PLUDUX_BACKTEST_HISTORY_HPP
#define PLUDUX_PLUDUX_BACKTEST_HISTORY_HPP

#include <cstddef>
#include <optional>
#include <vector>

#include "./snapshot.hpp"
#include "./trade_record.hpp"

namespace pludux::backtest {

class History {
public:
  History();

  auto size() const noexcept -> std::size_t;

  auto operator[](std::size_t index) const -> Snapshot;

  auto front() const -> Snapshot;

  auto back() const -> Snapshot;

  void add_data(Snapshot snapshot);

  auto timestamps() const noexcept -> const std::vector<double>&;

  auto timestamp(std::size_t index) const noexcept -> double;

  auto opens() const noexcept -> const std::vector<double>&;

  auto open(std::size_t index) const noexcept -> double;

  auto highs() const noexcept -> const std::vector<double>&;

  auto high(std::size_t index) const noexcept -> double;

  auto lows() const noexcept -> const std::vector<double>&;

  auto low(std::size_t index) const noexcept -> double;

  auto closes() const noexcept -> const std::vector<double>&;

  auto close(std::size_t index) const noexcept -> double;

  auto volumes() const noexcept -> const std::vector<double>&;

  auto volume(std::size_t index) const noexcept -> double;

  auto trade_records() const noexcept
   -> const std::vector<std::optional<TradeRecord>>&;

  auto trade_record(std::size_t index) const noexcept
   -> const std::optional<TradeRecord>&;

private:
  std::size_t size_;
  std::vector<double> timestamps_;
  std::vector<double> opens_;
  std::vector<double> highs_;
  std::vector<double> lows_;
  std::vector<double> closes_;
  std::vector<double> volumes_;
  std::vector<std::optional<TradeRecord>> trade_records_;
};

} // namespace pludux::backtest

#endif
