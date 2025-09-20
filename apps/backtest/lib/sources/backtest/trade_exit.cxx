module;

#include <cstddef>
#include <ctime>

export module pludux.backtest:trade_exit;

export namespace pludux::backtest {

class TradeExit {
public:
  enum class Reason { signal, stop_loss, take_profit };

  TradeExit(double position_size, double price, Reason reason)
  : reason_(reason)
  , position_size_(position_size)
  , price_(price)
  {
  }

  auto position_size(this const auto& self) noexcept -> double
  {
    return self.position_size_;
  }

  auto price(this const auto& self) noexcept -> double
  {
    return self.price_;
  }

  auto reason(this const auto& self) noexcept -> Reason
  {
    return self.reason_;
  }

  auto is_long_direction(this const auto& self) noexcept -> bool
  {
    return self.position_size_ > 0.0;
  }

  auto is_short_direction(this const auto& self) noexcept -> bool
  {
    return self.position_size_ < 0.0;
  }

private:
  Reason reason_;

  double position_size_;
  double price_;
};

} // namespace pludux::backtest
