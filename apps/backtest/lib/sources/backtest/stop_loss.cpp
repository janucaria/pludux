#include <algorithm>

#include <pludux/ta.hpp>

#include <pludux/backtest/stop_loss.hpp>

namespace pludux::backtest {

StopLoss::StopLoss()
: StopLoss{
   screener::ValueMethod{0}, screener::ValueMethod{0}, screener::ValueMethod{0}}
{
}

StopLoss::StopLoss(screener::ScreenerMethod entry_price_method,
                   screener::ScreenerMethod risk_method,
                   screener::ScreenerMethod trading_price_method)
: entry_price_method_{entry_price_method}
, risk_method_{std::move(risk_method)}
, trading_price_method_{std::move(trading_price_method)}
{
}

auto StopLoss::operator()(const AssetSnapshot& asset) const -> TradingStopLoss
{
  const auto entry_price = entry_price_method_(asset)[0];
  const auto stop_loss = risk_method_(asset)[0];
  const auto stop_loss_price = entry_price - stop_loss;

  return TradingStopLoss{stop_loss_price, trading_price_method_};
}

auto StopLoss::get_order_size(double capital_risk,
                              const AssetSnapshot& asset) const -> double
{
  const auto stop_loss = risk_method_(asset)[0];
  const auto order_size = capital_risk / stop_loss;

  return order_size;
}

} // namespace pludux::backtest
