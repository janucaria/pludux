#include <pludux/asset_snapshot.hpp>

namespace pludux {

AssetSnapshot::AssetSnapshot(const AssetHistory& asset_history)
: AssetSnapshot{0, asset_history}
{
}

AssetSnapshot::AssetSnapshot(std::size_t offset,
                             const AssetHistory& asset_history)
: offset_{offset}
, asset_history_{asset_history}
{
}

auto AssetSnapshot::operator[](std::string_view key) const -> double
{
  return get_values(std::move(key))[0];
}

auto AssetSnapshot::operator[](std::size_t index) const -> AssetSnapshot
{
  return AssetSnapshot{offset_ + index, asset_history_};
}

auto AssetSnapshot::offset() const noexcept -> std::size_t
{
  return offset_;
}

auto AssetSnapshot::size() const noexcept -> std::size_t
{
  if(offset_ >= asset_history_.size()) {
    return 0;
  }

  return asset_history_.size() - offset_;
}

auto AssetSnapshot::contains(std::string_view key) const noexcept -> bool
{
  return asset_history_.contains(std::string(key));
}

auto AssetSnapshot::get_values(std::string_view key) const
 -> SubSeries<RefSeries<const PolySeries<double>>>
{
  return SubSeries{asset_history_[key], static_cast<std::ptrdiff_t>(offset_)};
}

auto AssetSnapshot::get_datetime_values() const
 -> SubSeries<RefSeries<const PolySeries<double>>>
{
  return get_values(asset_history_.datetime_key());
}

auto AssetSnapshot::get_open_values() const
 -> SubSeries<RefSeries<const PolySeries<double>>>
{
  return get_values(asset_history_.open_key());
}

auto AssetSnapshot::get_high_values() const
 -> SubSeries<RefSeries<const PolySeries<double>>>
{
  return get_values(asset_history_.high_key());
}

auto AssetSnapshot::get_low_values() const
 -> SubSeries<RefSeries<const PolySeries<double>>>
{
  return get_values(asset_history_.low_key());
}

auto AssetSnapshot::get_close_values() const
 -> SubSeries<RefSeries<const PolySeries<double>>>
{
  return get_values(asset_history_.close_key());
}

auto AssetSnapshot::get_volume_values() const
 -> SubSeries<RefSeries<const PolySeries<double>>>
{
  return get_values(asset_history_.volume_key());
}

auto AssetSnapshot::get_datetime() const -> double
{
  return get_datetime_values()[0];
}

auto AssetSnapshot::get_open() const -> double
{
  return get_open_values()[0];
}

auto AssetSnapshot::get_high() const -> double
{
  return get_high_values()[0];
}

auto AssetSnapshot::get_low() const -> double
{
  return get_low_values()[0];
}

auto AssetSnapshot::get_close() const -> double
{
  return get_close_values()[0];
}

auto AssetSnapshot::get_volume() const -> double
{
  return get_volume_values()[0];
}

} // namespace pludux
