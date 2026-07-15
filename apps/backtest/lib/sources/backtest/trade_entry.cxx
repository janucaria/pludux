module;

#include <cmath>
#include <cstddef>
#include <ctime>

export module pludux.backtest:trade_entry;

export namespace pludux::backtest {

class TradeEntry {
public:
  TradeEntry(double position_size, double price)
  : TradeEntry{position_size, price, NAN, NAN, false}
  {
  }

  TradeEntry(double position_size,
             double price,
             double stop_price,
             double stop_loss_price,
             bool stop_loss_trailing_enabled)
  : position_size_(position_size)
  , price_(price)
  , stop_price_(stop_price)
  , stop_loss_price_(stop_loss_price)
  , stop_loss_trailing_enabled_(stop_loss_trailing_enabled)
  {
  }

  auto operator==(const TradeEntry&) const noexcept -> bool = default;

  auto position_size(this const TradeEntry& self) noexcept -> double
  {
    return self.position_size_;
  }

  void position_size(this TradeEntry& self, double new_position_size) noexcept
  {
    self.position_size_ = new_position_size;
  }

  auto price(this const TradeEntry& self) noexcept -> double
  {
    return self.price_;
  }

  auto stop_price(this const TradeEntry& self) noexcept -> double
  {
    return self.stop_price_;
  }

  auto stop_loss_price(this const TradeEntry& self) noexcept -> double
  {
    return self.stop_loss_price_;
  }

  auto stop_loss_trailing_enabled(this const TradeEntry& self) noexcept -> bool
  {
    return self.stop_loss_trailing_enabled_;
  }

  auto is_long_direction(this const TradeEntry& self) noexcept -> bool
  {
    return self.position_size_ > 0.0;
  }

  auto is_short_direction(this const TradeEntry& self) noexcept -> bool
  {
    return self.position_size_ < 0.0;
  }

  auto is_buy(this const TradeEntry& self) noexcept -> bool
  {
    return self.is_long_direction();
  }

  auto is_sell(this const TradeEntry& self) noexcept -> bool
  {
    return self.is_short_direction();
  }

  auto notional_value(this const TradeEntry& self) noexcept -> double
  {
    return std::abs(self.position_size_ * self.price_);
  }

private:
  double position_size_;
  double price_;

  double stop_price_;
  double stop_loss_price_;

  bool stop_loss_trailing_enabled_;
};

} // namespace pludux::backtest
