#include <pludux/backtest/trading_session.hpp>

namespace pludux::backtest {

TradingSession::TradingSession(double order_size,
                               std::size_t entry_index,
                               double entry_price,
                               std::time_t entry_timestamp,
                               screener::ScreenerMethod timestamp_method,
                               screener::ScreenerMethod price_method,
                               screener::ScreenerFilter exit_filter,
                               TradingTakeProfit trading_take_profit,
                               TradingStopLoss trading_stop_loss)
: order_size_{order_size}
, entry_price_{entry_price}
, entry_index_{entry_index}
, entry_timestamp_{entry_timestamp}
, timestamp_method_{std::move(timestamp_method)}
, price_method_{std::move(price_method)}
, exit_filter_{std::move(exit_filter)}
, trading_take_profit_{std::move(trading_take_profit)}
, trading_stop_loss_{std::move(trading_stop_loss)}
{
}

auto TradingSession::done_trade(const AssetSnapshot& asset)
 -> std::optional<TradeRecord>
{
  const auto exit_trade_status_opt =
   [&]() -> std::optional<TradeRecord::Status> {
    if(trading_stop_loss_(asset)) {
      return TradeRecord::Status::closed_stop_loss;
    }

    if(trading_take_profit_(asset)) {
      return TradeRecord::Status::closed_take_profit;
    }

    if(exit_filter_(asset)) {
      return TradeRecord::Status::closed_exit_signal;
    }

    return std::nullopt;
  }();

  if(!exit_trade_status_opt.has_value()) {
    return std::nullopt;
  }

  const auto exit_trade_status = exit_trade_status_opt.value();
  const auto stop_loss_price = trading_stop_loss_.exit_price();
  const auto take_profit_price = trading_take_profit_.exit_price();

  const auto exit_price = [&]() -> double {
    switch(exit_trade_status) {
    case TradeRecord::Status::closed_take_profit:
      return take_profit_price;
    case TradeRecord::Status::closed_stop_loss:
      return stop_loss_price;
    case TradeRecord::Status::closed_exit_signal:
    case TradeRecord::Status::open:
    default:
      return price_method_(asset)[0];
    }
  }();

  const auto exit_timestamp =
   static_cast<std::time_t>(timestamp_method_(asset)[0]);
  const auto exit_index = asset.offset();

  return TradeRecord{exit_trade_status,
                     order_size_,
                     entry_index_,
                     exit_index,
                     entry_price_,
                     exit_price,
                     stop_loss_price,
                     take_profit_price,
                     entry_timestamp_,
                     exit_timestamp};
}

auto TradingSession::ongoing_trade(const AssetSnapshot& asset) const
 -> TradeRecord
{
  const auto exit_timestamp =
   static_cast<std::time_t>(timestamp_method_(asset)[0]);
  const auto exit_index = asset.offset();
  const auto stop_loss_price = trading_stop_loss_.exit_price();
  const auto take_profit_price = trading_take_profit_.exit_price();
  const auto exit_price = price_method_(asset)[0];

  return TradeRecord{TradeRecord::Status::open,
                     order_size_,
                     entry_index_,
                     exit_index,
                     entry_price_,
                     exit_price,
                     stop_loss_price,
                     take_profit_price,
                     entry_timestamp_,
                     exit_timestamp};
}

} // namespace pludux::backtest
