module;

#include <cstddef>
#include <ctime>
#include <string>
#include <string_view>
#include <unordered_map>

export module pludux.asset_snapshot;

import pludux.series;
import pludux.asset_history;

namespace pludux {

export class AssetSnapshot {
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

  auto operator[](std::string_view key) const -> double
  {
    return get_values(std::move(key))[0];
  }

  auto operator[](std::size_t index) const -> AssetSnapshot
  {
    return AssetSnapshot{offset_ + index, asset_history_};
  }

  auto offset() const noexcept -> std::size_t
  {
    return offset_;
  }

  auto size() const noexcept -> std::size_t
  {
    if(offset_ >= asset_history_.size()) {
      return 0;
    }

    return asset_history_.size() - offset_;
  }

  auto contains(std::string_view key) const noexcept -> bool
  {
    return asset_history_.contains(std::string(key));
  }

  auto get_values(std::string_view key) const
   -> SubSeries<RefSeries<const PolySeries<double>>>
  {
    return SubSeries{asset_history_[key], static_cast<std::ptrdiff_t>(offset_)};
  }

  auto get_datetime_values() const
   -> SubSeries<RefSeries<const PolySeries<double>>>
  {
    return get_values(asset_history_.datetime_key());
  }

  auto get_open_values() const -> SubSeries<RefSeries<const PolySeries<double>>>
  {
    return get_values(asset_history_.open_key());
  }

  auto get_high_values() const -> SubSeries<RefSeries<const PolySeries<double>>>
  {
    return get_values(asset_history_.high_key());
  }

  auto get_low_values() const -> SubSeries<RefSeries<const PolySeries<double>>>
  {
    return get_values(asset_history_.low_key());
  }

  auto get_close_values() const
   -> SubSeries<RefSeries<const PolySeries<double>>>
  {
    return get_values(asset_history_.close_key());
  }

  auto get_volume_values() const
   -> SubSeries<RefSeries<const PolySeries<double>>>
  {
    return get_values(asset_history_.volume_key());
  }

  auto get_datetime() const -> double
  {
    return get_datetime_values()[0];
  }

  auto get_open() const -> double
  {
    return get_open_values()[0];
  }

  auto get_high() const -> double
  {
    return get_high_values()[0];
  }

  auto get_low() const -> double
  {
    return get_low_values()[0];
  }

  auto get_close() const -> double
  {
    return get_close_values()[0];
  }

  auto get_volume() const -> double
  {
    return get_volume_values()[0];
  }

private:
  std::size_t offset_;
  const AssetHistory& asset_history_;
};

} // namespace pludux
