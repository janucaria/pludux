#include <algorithm>

#include <pludux/backtest/stop_loss.hpp>

namespace pludux::backtest {

StopLoss::StopLoss(screener::ScreenerMethod risk_method,
                   bool is_disabled,
                   bool is_trailing)
: is_disabled_{is_disabled}
, is_trailing_{is_trailing}
, risk_method_{std::move(risk_method)}
{
}

auto StopLoss::operator()(AssetSnapshot asset_snapshot) const -> TradingStopLoss
{
  const auto entry_price = asset_snapshot.get_close();
  const auto stop_loss_risk = risk_method_(asset_snapshot)[0];
  const auto stop_loss_price = entry_price - stop_loss_risk;

  return TradingStopLoss{
   is_disabled_, is_trailing_, stop_loss_risk, stop_loss_price};
}

auto StopLoss::is_disabled() const noexcept -> bool
{
  return is_disabled_;
}

auto StopLoss::is_trailing() const noexcept -> bool
{
  return is_trailing_;
}

auto StopLoss::risk_method() const noexcept -> const screener::ScreenerMethod&
{
  return risk_method_;
}

auto StopLoss::get_risk_size(AssetSnapshot asset_snapshot) const -> double
{
  return risk_method_(asset_snapshot)[0];
}

} // namespace pludux::backtest
