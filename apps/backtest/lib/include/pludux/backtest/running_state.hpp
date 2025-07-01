#ifndef PLUDUX_PLUDUX_BACKTEST_RUNNING_STATE_HPP
#define PLUDUX_PLUDUX_BACKTEST_RUNNING_STATE_HPP

#include <pludux/asset_snapshot.hpp>

namespace pludux::backtest {

class RunningState {
public:
  RunningState(AssetSnapshot asset_snapshot);

  auto asset_index() const noexcept -> std::size_t;

  auto capital_risk() const noexcept -> double;

  void capital_risk(double capital_risk) noexcept;

  auto asset_snapshot() const noexcept -> const AssetSnapshot&;

  auto price() const -> double;

  auto timestamp() const -> double;

  auto open() const -> double;

  auto high() const -> double;

  auto low() const -> double;

  auto close() const -> double;

  auto volume() const -> double;

private:
  std::size_t asset_index_;
  double capital_risk_;
  AssetSnapshot asset_snapshot_;
};

} // namespace pludux::backtest

#endif
