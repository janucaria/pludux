#include <algorithm>
#include <cassert>
#include <cstddef>
#include <ctime>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

#include <pludux/asset_history.hpp>

namespace pludux {

AssetHistory::AssetHistory(
 std::initializer_list<std::pair<std::string, PolySeries<double>>> data)
: AssetHistory(data.begin(), data.end())
{
}

AssetHistory::operator AssetSnapshot() const noexcept
{
  return this->operator[](0);
}

auto AssetHistory::operator[](std::size_t index) const noexcept -> AssetSnapshot
{
  return AssetSnapshot{index, history_data_};
}

auto AssetHistory::operator[](const std::string& key) const
 -> RefSeries<const PolySeries<double>>
{
  return RefSeries{history_data_.at(key)};
}

auto AssetHistory::size() const noexcept -> std::size_t
{
  return size_;
}

auto AssetHistory::contains(const std::string& key) const noexcept -> bool
{
  return history_data_.contains(key);
}

auto AssetHistory::insert(std::string key, PolySeries<double> series) -> void
{
  history_data_.emplace(std::move(key), std::move(series));
}

auto AssetHistory::begin() const -> Iterator
{
  return Iterator{*this, 0};
}

auto AssetHistory::end() const -> Iterator
{
  return Iterator{*this, size_};
}

auto AssetHistory::rbegin() const -> ReverseIterator
{
  return std::make_reverse_iterator(end());
}

auto AssetHistory::rend() const -> ReverseIterator
{
  return std::make_reverse_iterator(begin());
}

// -------------------------------------------------------------------------- //

AssetHistory::ConstIterator::ConstIterator(const AssetHistory& asset_history,
                                           std::size_t index)
: asset_history_{asset_history}
, index_{index}
{
}

auto AssetHistory::ConstIterator::operator++() -> ConstIterator&
{
  ++index_;
  return *this;
}

auto AssetHistory::ConstIterator::operator++(int) -> ConstIterator
{
  auto copy = *this;
  ++index_;
  return copy;
}

auto AssetHistory::ConstIterator::operator--() -> ConstIterator&
{
  --index_;
  return *this;
}

auto AssetHistory::ConstIterator::operator--(int) -> ConstIterator
{
  auto copy = *this;
  --index_;
  return copy;
}

auto AssetHistory::ConstIterator::operator*() const -> value_type
{
  return asset_history_[index_];
}

auto AssetHistory::ConstIterator::operator==(const ConstIterator& other) const
 -> bool
{
  return index_ == other.index_;
}

auto AssetHistory::ConstIterator::operator!=(const ConstIterator& other) const
 -> bool
{
  return !(*this == other);
}

} // namespace pludux
