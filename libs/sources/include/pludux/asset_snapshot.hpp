#ifndef PLUDUX_PLUDUX_ASSET_SNAPSHOT_HPP
#define PLUDUX_PLUDUX_ASSET_SNAPSHOT_HPP

#include <cstdint>
#include <ctime>
#include <string>
#include <unordered_map>

#include "series.hpp"

namespace pludux {

class AssetSnapshot {
public:
  using HistoryData = std::unordered_map<std::string, PolySeries<double>>;

  AssetSnapshot(std::size_t offset, const HistoryData& history_data);

  auto operator[](std::string key) const -> PolySeries<double>;

  auto offset() const noexcept -> std::size_t;

  auto size() const noexcept -> std::size_t;

  auto contains(const std::string& key) const noexcept -> bool;

private:
  std::size_t offset_;
  const HistoryData& history_data_;
};

} // namespace pludux

#endif