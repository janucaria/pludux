module;

#include <ctime>
#include <initializer_list>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

export module pludux.asset_history;

import pludux.series;

namespace pludux {

export class AssetHistory {
public:
  AssetHistory(
   std::initializer_list<std::pair<std::string, PolySeries<double>>> data)
  : AssetHistory(data.begin(), data.end())
  {
  }

  template<typename TInputIt>
  AssetHistory(TInputIt begin_it, TInputIt end_it)
  : history_data_(begin_it, end_it)
  , size_(0)
  , datetime_key_{"Datetime"}
  , open_key_{"Open"}
  , high_key_{"High"}
  , low_key_{"Low"}
  , close_key_{"Close"}
  , volume_key_{"Volume"}
  {
    recalculate_size_();
  }

  auto operator[](std::string_view key) const
   -> RefSeries<const PolySeries<double>>
  {
    return RefSeries{history_data_.at(std::string(key))};
  }

  auto size() const noexcept -> std::size_t
  {
    return size_;
  }

  auto history_data() const noexcept
   -> const std::unordered_map<std::string, PolySeries<double>>&
  {
    return history_data_;
  }

  auto datetime_key() const noexcept -> const std::string&
  {
    return datetime_key_;
  }

  void datetime_key(std::string key) noexcept
  {
    datetime_key_ = std::move(key);
  }

  auto open_key() const noexcept -> const std::string&
  {
    return open_key_;
  }

  void open_key(std::string key) noexcept
  {
    open_key_ = std::move(key);
  }

  auto high_key() const noexcept -> const std::string&
  {
    return high_key_;
  }

  void high_key(std::string key) noexcept
  {
    high_key_ = std::move(key);
  }

  auto low_key() const noexcept -> const std::string&
  {
    return low_key_;
  }

  void low_key(std::string key) noexcept
  {
    low_key_ = std::move(key);
  }

  auto close_key() const noexcept -> const std::string&
  {
    return close_key_;
  }

  void close_key(std::string key) noexcept
  {
    close_key_ = std::move(key);
  }

  auto volume_key() const noexcept -> const std::string&
  {
    return volume_key_;
  }

  void volume_key(std::string key) noexcept
  {
    volume_key_ = std::move(key);
  }

  auto contains(std::string_view key) const noexcept -> bool
  {
    return history_data_.contains(std::string(key));
  }

  void insert(std::string key, PolySeries<double> series)
  {
    history_data_.emplace(std::move(key), std::move(series));
    recalculate_size_();
  }

  auto get_datetimes() const noexcept -> RefSeries<const PolySeries<double>>
  {
    return (*this)[datetime_key_];
  }

  auto get_opens() const noexcept -> RefSeries<const PolySeries<double>>
  {
    return (*this)[open_key_];
  }

  auto get_highs() const noexcept -> RefSeries<const PolySeries<double>>
  {
    return (*this)[high_key_];
  }

  auto get_lows() const noexcept -> RefSeries<const PolySeries<double>>
  {
    return (*this)[low_key_];
  }

  auto get_closes() const noexcept -> RefSeries<const PolySeries<double>>
  {
    return (*this)[close_key_];
  }

  auto get_volumes() const noexcept -> RefSeries<const PolySeries<double>>
  {
    return (*this)[volume_key_];
  }

private:
  std::unordered_map<std::string, PolySeries<double>> history_data_;
  std::size_t size_;
  std::string datetime_key_;
  std::string open_key_;
  std::string high_key_;
  std::string low_key_;
  std::string close_key_;
  std::string volume_key_;

  void recalculate_size_() noexcept
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
};

} // namespace pludux
