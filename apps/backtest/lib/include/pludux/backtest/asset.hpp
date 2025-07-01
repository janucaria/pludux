#ifndef PLUDUX_PLUDUX_BACKTEST_ASSET_HPP
#define PLUDUX_PLUDUX_BACKTEST_ASSET_HPP

#include <string>

#include <pludux/asset_history.hpp>

namespace pludux::backtest {

class Asset {
public:
  Asset(std::string name, AssetHistory asset_history);

  auto name() const noexcept -> const std::string&;

  auto history() const noexcept -> const AssetHistory&;

private:
  std::string name_;
  AssetHistory asset_history_;
};

} // namespace pludux::backtest

#endif