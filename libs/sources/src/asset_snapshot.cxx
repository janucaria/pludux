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

  AssetSnapshot(std::size_t lookback,
                const AssetHistory& asset_history) noexcept
  : lookback_{lookback}
  , asset_history_{asset_history}
  {
  }

  auto operator[](this AssetSnapshot self, const std::string& field) noexcept
   -> double
  {
    return self.data(field);
  }

  auto operator[](this AssetSnapshot self, std::size_t index) noexcept
   -> AssetSnapshot
  {
    return AssetSnapshot{self.lookback_ + index, self.asset_history_};
  }

  auto lookback(this AssetSnapshot self) noexcept -> std::size_t
  {
    return self.lookback_;
  }

  auto size(this AssetSnapshot self) noexcept -> std::size_t
  {
    if(self.lookback_ >= self.asset_history_.size()) {
      return 0;
    }

    return self.asset_history_.size() - self.lookback_;
  }

  auto contains(this AssetSnapshot self, const std::string& field) noexcept
   -> bool
  {
    return self.asset_history_.contains(field);
  }

  auto datetime(this AssetSnapshot self) noexcept -> double
  {
    return self.asset_history_.datetime_series()[self.lookback_];
  }

  auto open(this AssetSnapshot self) noexcept -> double
  {
    return self.asset_history_.open_series()[self.lookback_];
  }

  auto high(this AssetSnapshot self) noexcept -> double
  {
    return self.asset_history_.high_series()[self.lookback_];
  }

  auto low(this AssetSnapshot self) noexcept -> double
  {
    return self.asset_history_.low_series()[self.lookback_];
  }

  auto close(this AssetSnapshot self) noexcept -> double
  {
    return self.asset_history_.close_series()[self.lookback_];
  }

  auto volume(this AssetSnapshot self) noexcept -> double
  {
    return self.asset_history_.volume_series()[self.lookback_];
  }

  auto data(this AssetSnapshot self, const std::string& field) noexcept
   -> double
  {
    return self.asset_history_[field][self.lookback_];
  }

private:
  std::size_t lookback_;
  const AssetHistory& asset_history_;
};

} // namespace pludux
