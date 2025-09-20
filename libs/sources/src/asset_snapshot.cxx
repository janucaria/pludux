module;

#include <cstddef>
#include <ctime>
#include <string>
#include <string_view>
#include <unordered_map>

export module pludux:asset_snapshot;

export import :series;
export import :asset_history;

export namespace pludux {

class AssetSnapshot {
public:
  AssetSnapshot(const AssetHistory& asset_history)
  : AssetSnapshot{0, asset_history}
  {
  }

  AssetSnapshot(std::size_t offset, const AssetHistory& asset_history)
  : offset_{offset}
  , asset_history_{asset_history}
  {
  }

  auto operator[](this const auto& self, const std::string& key) -> double
  {
    return self.get_values(key)[0];
  }

  auto operator[](this const auto& self, std::size_t index) -> AssetSnapshot
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

  auto get_values(this const auto& self, const std::string& key)
   -> LookbackSeries<RefSeries<const PolySeries<double>>>
  {
    return LookbackSeries{self.asset_history_[key], self.offset_};
  }

  auto get_datetime_values(this const auto& self)
   -> LookbackSeries<RefSeries<const PolySeries<double>>>
  {
    return self.get_values(self.asset_history_.datetime_key());
  }

  auto get_open_values() const
   -> LookbackSeries<RefSeries<const PolySeries<double>>>
  {
    return get_values(asset_history_.open_key());
  }

  auto get_high_values() const
   -> LookbackSeries<RefSeries<const PolySeries<double>>>
  {
    return get_values(asset_history_.high_key());
  }

  auto get_low_values() const
   -> LookbackSeries<RefSeries<const PolySeries<double>>>
  {
    return get_values(asset_history_.low_key());
  }

  auto get_close_values() const
   -> LookbackSeries<RefSeries<const PolySeries<double>>>
  {
    return get_values(asset_history_.close_key());
  }

  auto get_volume_values() const
   -> LookbackSeries<RefSeries<const PolySeries<double>>>
  {
    return get_values(asset_history_.volume_key());
  }

  auto get_datetime(this const auto& self) -> double
  {
    return self.get_datetime_values()[0];
  }

  auto get_open(this const auto& self) -> double
  {
    return self.get_open_values()[0];
  }

  auto get_high(this const auto& self) -> double
  {
    return self.get_high_values()[0];
  }

  auto get_low(this const auto& self) -> double
  {
    return self.get_low_values()[0];
  }

  auto get_close(this const auto& self) -> double
  {
    return self.get_close_values()[0];
  }

  auto get_volume(this const auto& self) -> double
  {
    return self.get_volume_values()[0];
  }

private:
  std::size_t offset_;
  const AssetHistory& asset_history_;
};

} // namespace pludux
