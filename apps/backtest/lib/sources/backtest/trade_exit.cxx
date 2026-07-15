module;

#include <cmath>
#include <cstddef>
#include <ctime>
#include <optional>

export module pludux.backtest:trade_exit;

export namespace pludux::backtest {

class TradeExit {
public:
  enum class Reason { signal, stop_loss, take_profit };

  TradeExit(double position_size,
            double price,
            Reason reason,
            std::optional<std::size_t> take_profit_index = std::nullopt,
            std::optional<std::size_t> signal_exit_index = std::nullopt)
  : reason_(reason)
  , position_size_(position_size)
  , price_(price)
  , take_profit_index_{take_profit_index}
  , signal_exit_index_{signal_exit_index}
  {
  }

  auto operator==(const TradeExit&) const noexcept -> bool = default;

  auto position_size(this const TradeExit& self) noexcept -> double
  {
    return self.position_size_;
  }

  auto price(this const TradeExit& self) noexcept -> double
  {
    return self.price_;
  }

  auto reason(this const TradeExit& self) noexcept -> Reason
  {
    return self.reason_;
  }

  auto take_profit_index(this const TradeExit& self) noexcept
   -> std::optional<std::size_t>
  {
    return self.take_profit_index_;
  }

  auto signal_exit_index(this const TradeExit& self) noexcept
   -> std::optional<std::size_t>
  {
    return self.signal_exit_index_;
  }

  auto is_long_direction(this const TradeExit& self) noexcept -> bool
  {
    return self.position_size_ > 0.0;
  }

  auto is_short_direction(this const TradeExit& self) noexcept -> bool
  {
    return self.position_size_ < 0.0;
  }

  auto is_buy(this const TradeExit& self) noexcept -> bool
  {
    return self.is_short_direction();
  }

  auto is_sell(this const TradeExit& self) noexcept -> bool
  {
    return self.is_long_direction();
  }

  auto notional_value(this const TradeExit& self) noexcept -> double
  {
    return std::abs(self.position_size_ * self.price_);
  }

private:
  Reason reason_;

  double position_size_;
  double price_;
  std::optional<std::size_t> take_profit_index_;
  std::optional<std::size_t> signal_exit_index_;
};

} // namespace pludux::backtest
