#ifndef PLUDUX_PLUDUX_ASSET_SNAPSHOT_HPP
#define PLUDUX_PLUDUX_ASSET_SNAPSHOT_HPP

#include <cstddef>
#include <ctime>
#include <string>
#include <string_view>
#include <unordered_map>

#include "asset_history.hpp"
#include "series.hpp"

namespace pludux {

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

  auto get_low_values() const -> LookbackSeries<RefSeries<const PolySeries<double>>>;

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

} // namespace pludux

#endif