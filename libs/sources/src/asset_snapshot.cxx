module;

#include <cstddef>
#include <ctime>
#include <string>
#include <string_view>
#include <unordered_map>

export module pludux:asset_snapshot;

import :asset_series;
import :asset_history;

export namespace pludux {

class AssetSnapshot {
public:
  AssetSnapshot(const AssetHistory& asset_history) noexcept
  : AssetSnapshot{0, asset_history}
  {
  }

  AssetSnapshot(std::size_t offset, const AssetHistory& asset_history) noexcept
  : offset_{offset}
  , asset_history_{asset_history}
  {
  }

  auto operator[](this const auto& self, const std::string& key) noexcept
   -> double
  {
    return self.asset_history_[key][self.offset_];
  }

  auto operator[](this const auto& self, std::size_t index) noexcept
   -> AssetSnapshot
  {
    return AssetSnapshot{self.offset_ + index, self.asset_history_};
  }

  auto offset(this const auto& self) noexcept -> std::size_t
  {
    return self.offset_;
  }

  auto size(this const auto& self) noexcept -> std::size_t
  {
    if(self.offset_ >= self.asset_history_.size()) {
      return 0;
    }

    return self.asset_history_.size() - self.offset_;
  }

  auto contains(this const auto& self, const std::string& key) noexcept -> bool
  {
    return self.asset_history_.contains(key);
  }

  auto datetime(this const auto& self) noexcept -> double
  {
    return self.asset_history_.datetime_series()[self.offset_];
  }

  auto open(this const auto& self) noexcept -> double
  {
    return self.asset_history_.open_series()[self.offset_];
  }

  auto high(this const auto& self) noexcept -> double
  {
    return self.asset_history_.high_series()[self.offset_];
  }

  auto low(this const auto& self) noexcept -> double
  {
    return self.asset_history_.low_series()[self.offset_];
  }

  auto close(this const auto& self) noexcept -> double
  {
    return self.asset_history_.close_series()[self.offset_];
  }

  auto volume(this const auto& self) noexcept -> double
  {
    return self.asset_history_.volume_series()[self.offset_];
  }

private:
  std::size_t offset_;
  const AssetHistory& asset_history_;
};

} // namespace pludux
