#include <cassert>
#include <cstdint>
#include <ctime>
#include <string>
#include <utility>
#include <vector>

#include <pludux/quote_series.hpp>

namespace pludux {

QuoteSeries::QuoteSeries()
: QuoteSeries{0, {}, {}, {}, {}, {}, {}}
{
}

QuoteSeries::QuoteSeries(std::size_t size,
                         std::vector<std::time_t> timestamps,
                         std::vector<double> opens,
                         std::vector<double> highs,
                         std::vector<double> lows,
                         std::vector<double> closes,
                         std::vector<std::uint32_t> volumes)
: size_{size}
, timestamps_{std::move(timestamps)}
, opens_{std::move(opens)}
, highs_{std::move(highs)}
, lows_{std::move(lows)}
, closes_{std::move(closes)}
, volumes_{std::move(volumes)}
{
  assert(timestamps_.size() == size_);
  assert(opens_.size() == size_);
  assert(highs_.size() == size_);
  assert(lows_.size() == size_);
  assert(closes_.size() == size_);
  assert(volumes_.size() == size_);
}

auto QuoteSeries::operator[](std::size_t index) const -> Quote
{
  return Quote{static_cast<std::time_t>(timestamps_[index]),
               opens_[index],
               highs_[index],
               lows_[index],
               closes_[index],
               static_cast<std::uint32_t>(volumes_[index])};
}

auto QuoteSeries::timestamps() const noexcept
 -> RefSeries<const DataSeries<std::time_t>>
{
  return RefSeries{timestamps_};
}

auto QuoteSeries::opens() const noexcept -> RefSeries<const DataSeries<double>>
{
  return RefSeries{opens_};
}

auto QuoteSeries::highs() const noexcept -> RefSeries<const DataSeries<double>>
{
  return RefSeries{highs_};
}

auto QuoteSeries::lows() const noexcept -> RefSeries<const DataSeries<double>>
{
  return RefSeries{lows_};
}

auto QuoteSeries::closes() const noexcept
 -> RefSeries<const DataSeries<double>>
{
  return RefSeries{closes_};
}

auto QuoteSeries::volumes() const noexcept
 -> RefSeries<const DataSeries<double>>
{
  return RefSeries{volumes_};
}

void QuoteSeries::add_quote(const Quote& quote)
{
  ++size_;
  timestamps_.data().emplace_back(quote.timestamp());
  opens_.data().emplace_back(quote.open());
  highs_.data().emplace_back(quote.high());
  lows_.data().emplace_back(quote.low());
  closes_.data().emplace_back(quote.close());
  volumes_.data().emplace_back(quote.volume());
}

auto QuoteSeries::size() const noexcept -> std::size_t
{
  return size_;
}

auto QuoteSeries::begin() const -> Iterator
{
  return Iterator{*this, 0};
}

auto QuoteSeries::end() const -> Iterator
{
  return Iterator{*this, size_};
}

auto QuoteSeries::cbegin() const -> ConstIterator
{
  return begin();
}

auto QuoteSeries::cend() const -> ConstIterator
{
  return end();
}

auto QuoteSeries::rbegin() const -> ReverseIterator
{
  return std::make_reverse_iterator(end());
}

auto QuoteSeries::rend() const -> ReverseIterator
{
  return std::make_reverse_iterator(begin());
}

auto QuoteSeries::crbegin() const -> ConstReverseIterator
{
  return rbegin();
}

auto QuoteSeries::crend() const -> ConstReverseIterator
{
  return rend();
}

// -------------------------------------------------------------------------- //

QuoteSeries::ConstIterator::ConstIterator(const QuoteSeries& quote_history,
                                          std::size_t index)
: quote_series_{quote_history}
, index_{index}
{
}

auto QuoteSeries::ConstIterator::operator++() -> ConstIterator&
{
  ++index_;
  return *this;
}

auto QuoteSeries::ConstIterator::operator++(int) -> ConstIterator
{
  auto copy = *this;
  ++index_;
  return copy;
}

auto QuoteSeries::ConstIterator::operator--() -> ConstIterator&
{
  --index_;
  return *this;
}

auto QuoteSeries::ConstIterator::operator--(int) -> ConstIterator
{
  auto copy = *this;
  --index_;
  return copy;
}

auto QuoteSeries::ConstIterator::operator*() const -> value_type
{
  return quote_series_[index_];
}

auto QuoteSeries::ConstIterator::operator==(const ConstIterator& other) const
 -> bool
{
  return index_ == other.index_;
}

auto QuoteSeries::ConstIterator::operator!=(const ConstIterator& other) const
 -> bool
{
  return !(*this == other);
}

} // namespace pludux
