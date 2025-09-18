module;

#include <cstddef>
#include <ctime>
#include <string>
#include <string_view>
#include <unordered_map>

#include <pludux/series.hpp>

export module pludux.asset_snapshot;

export import pludux.series;

export import pludux.asset_history;

export namespace pludux {

class AssetSnapshot {
public:
  AssetSnapshot(const AssetHistory& asset_history);

  AssetSnapshot(std::size_t offset, const AssetHistory& asset_history);

  auto operator[](std::string_view key) const -> double;

  auto operator[](std::size_t index) const -> AssetSnapshot;

  auto offset() const noexcept -> std::size_t;

  auto size() const noexcept -> std::size_t;

  auto contains(std::string_view key) const noexcept -> bool;

  auto get_values(std::string_view key) const
   -> LookbackSeries<RefSeries<const PolySeries<double>>>;

  auto get_datetime_values() const
   -> LookbackSeries<RefSeries<const PolySeries<double>>>;

  auto get_open_values() const
   -> LookbackSeries<RefSeries<const PolySeries<double>>>;

  auto get_high_values() const
   -> LookbackSeries<RefSeries<const PolySeries<double>>>;

  auto get_low_values() const
   -> LookbackSeries<RefSeries<const PolySeries<double>>>;

  auto get_close_values() const
   -> LookbackSeries<RefSeries<const PolySeries<double>>>;

  auto get_volume_values() const
   -> LookbackSeries<RefSeries<const PolySeries<double>>>;

  auto get_datetime() const -> double;

  auto get_open() const -> double;

  auto get_high() const -> double;

  auto get_low() const -> double;

  auto get_close() const -> double;

  auto get_volume() const -> double;

private:
  std::size_t offset_;
  const AssetHistory& asset_history_;
};

// ------------------------------------------------------------------------

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
 -> LookbackSeries<RefSeries<const PolySeries<double>>>
{
  return LookbackSeries{asset_history_[key], offset_};
}

auto AssetSnapshot::get_datetime_values() const
 -> LookbackSeries<RefSeries<const PolySeries<double>>>
{
  return get_values(asset_history_.datetime_key());
}

auto AssetSnapshot::get_open_values() const
 -> LookbackSeries<RefSeries<const PolySeries<double>>>
{
  return get_values(asset_history_.open_key());
}

auto AssetSnapshot::get_high_values() const
 -> LookbackSeries<RefSeries<const PolySeries<double>>>
{
  return get_values(asset_history_.high_key());
}

auto AssetSnapshot::get_low_values() const
 -> LookbackSeries<RefSeries<const PolySeries<double>>>
{
  return get_values(asset_history_.low_key());
}

auto AssetSnapshot::get_close_values() const
 -> LookbackSeries<RefSeries<const PolySeries<double>>>
{
  return get_values(asset_history_.close_key());
}

auto AssetSnapshot::get_volume_values() const
 -> LookbackSeries<RefSeries<const PolySeries<double>>>
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
