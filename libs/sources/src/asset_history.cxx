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
  {
    recalculate_size_();
  }

  auto operator[](this const AssetHistory& self,
                  const std::string& field) noexcept -> AssetSeries
  {
    const auto it = self.field_data_.find(field);
    if(it != self.field_data_.end()) {
      return AssetSeries{it->second.data()};
    }
    return AssetSeries{};
  }

  auto size(this const AssetHistory& self) noexcept -> std::size_t
  {
    return self.size_;
  }

  auto field_data(this const AssetHistory& self) noexcept
   -> const FieldDataType&
  {
    return self.field_data_;
  }

  auto contains(this const AssetHistory& self,
                const std::string& field) noexcept -> bool
  {
    return self.field_data_.contains(field);
  }

  void
  insert(this AssetHistory& self, std::string field, AssetData series) noexcept
  {
    self.field_data_.emplace(std::move(field), std::move(series));
    self.recalculate_size_();
  }

private:
  FieldDataType field_data_;
  std::size_t size_;

  void recalculate_size_(this AssetHistory& self) noexcept
  {
    if(self.field_data_.empty()) {
      self.size_ = 0;
      return;
    }

    auto values_view = std::views::values(self.field_data_);
    auto sizes = std::views::transform(
     values_view, [](const auto& series) { return series.size(); });

    self.size_ = *std::ranges::max_element(sizes);
  }
};

} // namespace pludux
