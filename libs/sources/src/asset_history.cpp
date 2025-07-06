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

auto AssetHistory::operator[](std::string_view key) const
 -> RefSeries<const PolySeries<double>>
{
  return RefSeries{history_data_.at(std::string(key))};
}

auto AssetHistory::size() const noexcept -> std::size_t
{
  return size_;
}

auto AssetHistory::history_data() const noexcept
 -> const std::unordered_map<std::string, PolySeries<double>>&
{
  return history_data_;
}

auto AssetHistory::datetime_key() const noexcept -> const std::string&
{
  return datetime_key_;
}

void AssetHistory::datetime_key(std::string key) noexcept
{
  datetime_key_ = std::move(key);
}

auto AssetHistory::open_key() const noexcept -> const std::string&
{
  return open_key_;
}

void AssetHistory::open_key(std::string key) noexcept
{
  open_key_ = std::move(key);
}

auto AssetHistory::high_key() const noexcept -> const std::string&
{
  return high_key_;
}

void AssetHistory::high_key(std::string key) noexcept
{
  high_key_ = std::move(key);
}

auto AssetHistory::low_key() const noexcept -> const std::string&
{
  return low_key_;
}

void AssetHistory::low_key(std::string key) noexcept
{
  low_key_ = std::move(key);
}

auto AssetHistory::close_key() const noexcept -> const std::string&
{
  return close_key_;
}

void AssetHistory::close_key(std::string key) noexcept
{
  close_key_ = std::move(key);
}

auto AssetHistory::volume_key() const noexcept -> const std::string&
{
  return volume_key_;
}

void AssetHistory::volume_key(std::string key) noexcept
{
  volume_key_ = std::move(key);
}

auto AssetHistory::contains(std::string_view key) const noexcept -> bool
{
  return history_data_.contains(std::string(key));
}

void AssetHistory::insert(std::string key, PolySeries<double> series)
{
  history_data_.emplace(std::move(key), std::move(series));
  recalculate_size_();
}

auto AssetHistory::get_datetimes() const noexcept
 -> RefSeries<const PolySeries<double>>
{
  return (*this)[datetime_key_];
}

auto AssetHistory::get_opens() const noexcept
 -> RefSeries<const PolySeries<double>>
{
  return (*this)[open_key_];
}

auto AssetHistory::get_highs() const noexcept
 -> RefSeries<const PolySeries<double>>
{
  return (*this)[high_key_];
}

auto AssetHistory::get_lows() const noexcept
 -> RefSeries<const PolySeries<double>>
{
  return (*this)[low_key_];
}

auto AssetHistory::get_closes() const noexcept
 -> RefSeries<const PolySeries<double>>
{
  return (*this)[close_key_];
}

auto AssetHistory::get_volumes() const noexcept
 -> RefSeries<const PolySeries<double>>
{
  return (*this)[volume_key_];
}

void AssetHistory::recalculate_size_() noexcept
{
  size_ = history_data_.empty()
           ? 0
           : std::max_element(history_data_.begin(),
                              history_data_.end(),
                              [](const auto& lhs, const auto& rhs) {
                                return lhs.second.size() < rhs.second.size();
                              })
              ->second.size();
}

} // namespace pludux
