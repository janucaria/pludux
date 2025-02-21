#include <algorithm>

#include <pludux/ta.hpp>

#include <pludux/backtest/stop_loss.hpp>

namespace pludux::backtest {

StopLoss::StopLoss()
: StopLoss{screener::ValueMethod{0}, screener::ValueMethod{0}}
{
}

StopLoss::StopLoss(screener::ScreenerMethod risk_method,
                   screener::ScreenerMethod trading_price_method)
: risk_method_{std::move(risk_method)}
, trading_price_method_{std::move(trading_price_method)}
{
}

auto StopLoss::operator()(const RunningState& running_state) const
 -> TradingStopLoss
{
  const auto asset = running_state.asset_snapshot();
  const auto entry_price = running_state.price();
  const auto stop_loss = risk_method_(asset)[0];
  const auto stop_loss_price = entry_price - stop_loss;

  return TradingStopLoss{stop_loss_price, trading_price_method_};
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
