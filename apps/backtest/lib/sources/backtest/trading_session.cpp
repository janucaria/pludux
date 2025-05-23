#include <pludux/backtest/trading_session.hpp>

namespace pludux::backtest {

TradingSession::TradingSession(double order_size,
                               std::size_t entry_index,
                               double entry_price,
                               std::time_t entry_timestamp,
                               screener::ScreenerFilter exit_filter,
                               TradingTakeProfit trading_take_profit,
                               TradingStopLoss trading_stop_loss)
: order_size_{order_size}
, entry_price_{entry_price}
, entry_index_{entry_index}
, entry_timestamp_{entry_timestamp}
, exit_filter_{std::move(exit_filter)}
, trading_take_profit_{std::move(trading_take_profit)}
, trading_stop_loss_{std::move(trading_stop_loss)}
{
}

auto TradingSession::get_trading_state(const RunningState& running_state)
 -> TradeRecord
{
  const auto& asset = running_state.asset_snapshot();
  const auto current_index = running_state.asset_index();

  const auto trade_status = [&]() {
    if(current_index == entry_index_) {
      return TradeRecord::Status::open;
    }

    if(trading_stop_loss_(running_state)) {
      return TradeRecord::Status::closed_stop_loss;
    }

    if(trading_take_profit_(asset)) {
      return TradeRecord::Status::closed_take_profit;
    }

    if(exit_filter_(asset)) {
      return TradeRecord::Status::closed_exit_signal;
    }

    return TradeRecord::Status::open;
  }();

  const auto stop_loss_price = trading_stop_loss_.exit_price();
  const auto take_profit_price = trading_take_profit_.exit_price();

  const auto exit_price = [&]() -> double {
    switch(trade_status) {
    case TradeRecord::Status::closed_take_profit:
      return take_profit_price;
    case TradeRecord::Status::closed_stop_loss:
      return stop_loss_price;
    case TradeRecord::Status::closed_exit_signal:
    case TradeRecord::Status::open:
      return running_state.price();
    default:
      return 0;
    }
  }();

  const auto exit_timestamp =
   static_cast<std::time_t>(running_state.timestamp());
  const auto exit_index = current_index;

  return TradeRecord{trade_status,
                     order_size_,
                     entry_index_,
                     exit_index,
                     entry_timestamp_,
                     exit_timestamp,
                     entry_price_,
                     exit_price,
                     stop_loss_price,
                     take_profit_price};
}

} // namespace pludux::backtest
