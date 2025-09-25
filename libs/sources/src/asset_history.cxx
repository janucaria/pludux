module;

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <ctime>
#include <initializer_list>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

export module pludux:asset_history;

import :asset_data;
import :asset_series;

export namespace pludux {

class AssetHistory {
public:
  using FieldDataType = std::unordered_map<std::string, AssetData>;

  AssetHistory(
   std::initializer_list<std::pair<std::string, std::initializer_list<double>>>
    data)
  : AssetHistory(data.begin(), data.end())
  {
  }

  template<typename TInputIt>
  AssetHistory(TInputIt begin_it, TInputIt end_it)
  : field_data_(begin_it, end_it)
  , size_{0}
  , datetime_field_{"Datetime"}
  , open_field_{"Open"}
  , high_field_{"High"}
  , low_field_{"Low"}
  , close_field_{"Close"}
  , volume_field_{"Volume"}
  {
    recalculate_size_();
  }

  auto operator[](this const auto& self, const std::string& field) noexcept
   -> AssetSeries
  {
    const auto it = self.field_data_.find(field);
    if(it != self.field_data_.end()) {
      return AssetSeries{it->second.data()};
    }
    return AssetSeries{};
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    return self.size_;
  }

  auto field_data(this const auto& self) noexcept -> const FieldDataType&
  {
    return self.field_data_;
  }

  auto datetime_field(this const auto& self) noexcept -> const std::string&
  {
    return self.datetime_field_;
  }

  void datetime_field(this auto& self, std::string field) noexcept
  {
    self.datetime_field_ = std::move(field);
  }

  auto open_field(this const auto& self) noexcept -> const std::string&
  {
    return self.open_field_;
  }

  void open_field(this auto& self, std::string field) noexcept
  {
    self.open_field_ = std::move(field);
  }

  auto high_field(this const auto& self) noexcept -> const std::string&
  {
    return self.high_field_;
  }

  void high_field(this auto& self, std::string field) noexcept
  {
    self.high_field_ = std::move(field);
  }

  auto low_field(this const auto& self) noexcept -> const std::string&
  {
    return self.low_field_;
  }

  void low_field(this auto& self, std::string field) noexcept
  {
    self.low_field_ = std::move(field);
  }

  auto close_field(this const auto& self) noexcept -> const std::string&
  {
    return self.close_field_;
  }

  void close_field(this auto& self, std::string field) noexcept
  {
    self.close_field_ = std::move(field);
  }

  auto volume_field(this const auto& self) noexcept -> const std::string&
  {
    return self.volume_field_;
  }

  void volume_field(this auto& self, std::string field) noexcept
  {
    self.volume_field_ = std::move(field);
  }

  auto contains(this const auto& self, const std::string& field) noexcept
   -> bool
  {
    return self.field_data_.contains(field);
  }

  void insert(this auto& self, std::string field, AssetData series)
  {
    self.field_data_.emplace(std::move(field), std::move(series));
    self.recalculate_size_();
  }

  auto datetime_series(this const auto& self) noexcept -> AssetSeries
  {
    return self[self.datetime_field_];
  }

  auto open_series(this const auto& self) noexcept -> AssetSeries
  {
    return self[self.open_field_];
  }

  auto high_series(this const auto& self) noexcept -> AssetSeries
  {
    return self[self.high_field_];
  }

  auto low_series(this const auto& self) noexcept -> AssetSeries
  {
    return self[self.low_field_];
  }

  auto close_series(this const auto& self) noexcept -> AssetSeries
  {
    return self[self.close_field_];
  }

  auto volume_series(this const auto& self) noexcept -> AssetSeries
  {
    return self[self.volume_field_];
  }

private:
  FieldDataType field_data_;
  std::size_t size_;
  std::string datetime_field_;
  std::string open_field_;
  std::string high_field_;
  std::string low_field_;
  std::string close_field_;
  std::string volume_field_;

  void recalculate_size_(this auto& self) noexcept
  {
    self.size_ =
     std::ranges::fold_left_first(
      self.field_data_ | std::views::values |
       std::views::transform([](const auto& series) { return series.size(); }),
      [](auto a, auto b) { return std::max(a, b); })
      .value_or(0);
  }
};

} // namespace pludux
