#include <pludux/backtest/running_state.hpp>

namespace pludux::backtest {

RunningState::RunningState(AssetQuote asset_quote, std::size_t current_index)
: asset_index_{current_index}
, capital_risk_{0}
, asset_quote_{std::move(asset_quote)}
{
}

auto RunningState::asset_quote() const noexcept -> const AssetQuote&
{
  return asset_quote_;
}

auto RunningState::asset_index() const noexcept -> std::size_t
{
  return asset_index_;
}

auto RunningState::capital_risk() const noexcept -> double
{
  return capital_risk_;
}

void RunningState::capital_risk(double capital_risk) noexcept
{
  capital_risk_ = capital_risk;
}

auto RunningState::price() const noexcept -> double
{
  return close();
}

auto RunningState::asset_snapshot() const noexcept -> const AssetSnapshot&
{
  return asset_quote_.asset_snapshot();
}

auto RunningState::timestamp() const noexcept -> double
{
  return asset_quote_.time();
}

auto RunningState::open() const noexcept -> double
{
  return asset_quote_.open();
}

auto RunningState::high() const noexcept -> double
{
  return asset_quote_.high();
}

auto RunningState::low() const noexcept -> double
{
  return asset_quote_.low();
}

auto RunningState::close() const noexcept -> double
{
  return asset_quote_.close();
}

auto RunningState::volume() const noexcept -> double
{
  return asset_quote_.volume();
}

} // namespace pludux::backtest
