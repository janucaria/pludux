#ifndef PLUDUX_PLUDUX_BACKTEST_ASSET_HPP
#define PLUDUX_PLUDUX_BACKTEST_ASSET_HPP

#include <string>

#include <pludux/asset_history.hpp>
#include <pludux/asset_snapshot.hpp>

namespace pludux::backtest {

class Asset {
public:
  Asset(std::string name, AssetHistory asset_history);

  auto name() const noexcept -> const std::string&;

  auto history() const noexcept -> const AssetHistory&;

  auto get_snapshot(std::size_t index = 0) const noexcept -> AssetSnapshot;

  auto size() const noexcept -> std::size_t;

private:
  std::string name_;
  AssetHistory asset_history_;
};

} // namespace pludux::backtest

#endif