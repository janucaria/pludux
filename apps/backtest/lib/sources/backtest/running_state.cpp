#include <pludux/backtest/running_state.hpp>

namespace pludux::backtest {

RunningState::RunningState(AssetSnapshot asset_snapshot)
: capital_risk_{0}
, asset_snapshot_{std::move(asset_snapshot)}
{
}

auto RunningState::asset_snapshot() const noexcept -> const AssetSnapshot&
{
  return asset_snapshot_;
}

auto RunningState::asset_index() const noexcept -> std::size_t
{
  return asset_snapshot().offset();
}

auto RunningState::capital_risk() const noexcept -> double
{
  return capital_risk_;
}

void RunningState::capital_risk(double capital_risk) noexcept
{
  capital_risk_ = capital_risk;
}

auto RunningState::price() const -> double
{
  return close();
}

auto RunningState::timestamp() const -> double
{
  return asset_snapshot_.get_datetime();
}

auto RunningState::open() const -> double
{
  return asset_snapshot_.get_open();
}

auto RunningState::high() const -> double
{
  return asset_snapshot_.get_high();
}

auto RunningState::low() const -> double
{
  return asset_snapshot_.get_low();
}

auto RunningState::close() const -> double
{
  return asset_snapshot_.get_close();
}

auto RunningState::volume() const -> double
{
  return asset_snapshot_.get_volume();
}

} // namespace pludux::backtest
