module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <ctime>
#include <initializer_list>
#include <iterator>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

export module pludux:asset_history;

import :series;

export namespace pludux {

class AssetHistory {
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

  auto operator[](this const auto& self, const std::string& key)
   -> RefSeries<const PolySeries<double>>
  {
    return RefSeries{self.history_data_.at(key)};
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.size_;
  }

  auto history_data(this const auto& self) noexcept
   -> const std::unordered_map<std::string, PolySeries<double>>&
  {
    return self.history_data_;
  }

  auto datetime_key(this const auto& self) noexcept -> const std::string&
  {
    return self.datetime_key_;
  }

  void datetime_key(this auto& self, std::string key) noexcept
  {
    self.datetime_key_ = std::move(key);
  }

  auto open_key(this const auto& self) noexcept -> const std::string&
  {
    return self.open_key_;
  }

  void open_key(this auto& self, std::string key) noexcept
  {
    self.open_key_ = std::move(key);
  }

  auto high_key(this const auto& self) noexcept -> const std::string&
  {
    return self.high_key_;
  }

  void high_key(this auto& self, std::string key) noexcept
  {
    self.high_key_ = std::move(key);
  }

  auto low_key(this const auto& self) noexcept -> const std::string&
  {
    return self.low_key_;
  }

  void low_key(this auto& self, std::string key) noexcept
  {
    self.low_key_ = std::move(key);
  }

  auto close_key(this const auto& self) noexcept -> const std::string&
  {
    return self.close_key_;
  }

  void close_key(this auto& self, std::string key) noexcept
  {
    self.close_key_ = std::move(key);
  }

  auto volume_key(this const auto& self) noexcept -> const std::string&
  {
    return self.volume_key_;
  }

  void volume_key(this auto& self, std::string key) noexcept
  {
    self.volume_key_ = std::move(key);
  }

  auto contains(this const auto& self, const std::string& key) noexcept -> bool
  {
    return self.history_data_.contains(key);
  }

  void insert(this auto& self, std::string key, PolySeries<double> series)
  {
    self.history_data_.emplace(std::move(key), std::move(series));
    self.recalculate_size_();
  }

  auto get_datetimes(this const auto& self) noexcept
   -> RefSeries<const PolySeries<double>>
  {
    return self[self.datetime_key_];
  }

  auto get_opens(this const auto& self) noexcept
   -> RefSeries<const PolySeries<double>>
  {
    return self[self.open_key_];
  }

  auto get_highs(this const auto& self) noexcept
   -> RefSeries<const PolySeries<double>>
  {
    return self[self.high_key_];
  }

  auto get_lows(this const auto& self) noexcept
   -> RefSeries<const PolySeries<double>>
  {
    return self[self.low_key_];
  }

  auto get_closes(this const auto& self) noexcept
   -> RefSeries<const PolySeries<double>>
  {
    return self[self.close_key_];
  }

  auto get_volumes(this const auto& self) noexcept
   -> RefSeries<const PolySeries<double>>
  {
    return self[self.volume_key_];
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

  void recalculate_size_(this auto& self) noexcept
  {
    self.size_ =
     self.history_data_.empty()
      ? 0
      : std::max_element(self.history_data_.begin(),
                         self.history_data_.end(),
                         [](const auto& lhs, const auto& rhs) {
                           return lhs.second.size() < rhs.second.size();
                         })
         ->second.size();
  }
};

} // namespace pludux
