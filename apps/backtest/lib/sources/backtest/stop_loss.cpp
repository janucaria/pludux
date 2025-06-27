#include <algorithm>

#include <pludux/ta.hpp>

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

auto StopLoss::operator()(const RunningState& running_state) const
 -> TradingStopLoss
{
  const auto asset = running_state.asset_snapshot();
  const auto entry_price = running_state.price();
  const auto stop_loss_risk = risk_method_(asset)[0];
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

auto StopLoss::get_order_size(const RunningState& running_state) const -> double
{
  const auto capital_risk = running_state.capital_risk();
  const auto& asset = running_state.asset_snapshot();
  const auto stop_loss = risk_method_(asset)[0];
  const auto order_size = capital_risk / stop_loss;

  return order_size;
}

} // namespace pludux::backtest
