#include <cassert>

#include <pludux/backtest/history.hpp>

namespace pludux::backtest {

History::History()
: size_{0}
{
}

auto History::size() const noexcept -> std::size_t
{
  return size_;
}

auto History::operator[](std::size_t index) const -> Snapshot
{
  assert(index < size_);

  return Snapshot{timestamps_[index],
                  opens_[index],
                  highs_[index],
                  lows_[index],
                  closes_[index],
                  volumes_[index],
                  trade_records_[index]};
}

auto History::front() const -> Snapshot
{
  return operator[](0);
}

auto History::back() const -> Snapshot
{
  return operator[](size_ - 1);
}

void History::add_data(Snapshot snapshot)
{
  timestamps_.push_back(snapshot.timestamp());
  opens_.push_back(snapshot.open());
  highs_.push_back(snapshot.high());
  lows_.push_back(snapshot.low());
  closes_.push_back(snapshot.close());
  volumes_.push_back(snapshot.volume());
  trade_records_.push_back(snapshot.trade_record());
  ++size_;

  assert(timestamps_.size() == size_);
  assert(opens_.size() == size_);
  assert(highs_.size() == size_);
  assert(lows_.size() == size_);
  assert(closes_.size() == size_);
  assert(volumes_.size() == size_);
  assert(trade_records_.size() == size_);
}

auto History::timestamps() const noexcept -> const std::vector<double>&
{
  return timestamps_;
}

auto History::timestamp(std::size_t index) const noexcept -> double
{
  return timestamps_[index];
}

auto History::opens() const noexcept -> const std::vector<double>&
{
  return opens_;
}

auto History::open(std::size_t index) const noexcept -> double
{
  return opens_[index];
}

auto History::highs() const noexcept -> const std::vector<double>&
{
  return highs_;
}

auto History::high(std::size_t index) const noexcept -> double
{
  return highs_[index];
}

auto History::lows() const noexcept -> const std::vector<double>&
{
  return lows_;
}

auto History::low(std::size_t index) const noexcept -> double
{
  return lows_[index];
}

auto History::closes() const noexcept -> const std::vector<double>&
{
  return closes_;
}

auto History::close(std::size_t index) const noexcept -> double
{
  return closes_[index];
}

auto History::volumes() const noexcept -> const std::vector<double>&
{
  return volumes_;
}

auto History::volume(std::size_t index) const noexcept -> double
{
  return volumes_[index];
}

auto History::trade_records() const noexcept
 -> const std::vector<std::optional<TradeRecord>>&
{
  return trade_records_;
}

auto History::trade_record(std::size_t index) const noexcept
 -> const std::optional<TradeRecord>&
{
  return trade_records_[index];
}

} // namespace pludux::backtest
