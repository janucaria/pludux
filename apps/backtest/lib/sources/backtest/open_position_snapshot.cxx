module;

#include <cstddef>
#include <ctime>

export module pludux.backtest:open_position_snapshot;

export namespace pludux::backtest {

class OpenPositionSnapshot {
public:
  OpenPositionSnapshot() = default;

  OpenPositionSnapshot(std::size_t trade_id,
                       std::time_t entry_timestamp,
                       std::time_t market_timestamp,
                       double market_price,
                       double position_size,
                       double investment,
                       double entry_price,
                       double total_entry_fees,
                       double stop_price,
                       double target_price,
                       double stop_loss_price,
                       double take_profit_price)
  : trade_id_{trade_id}
  , entry_timestamp_{entry_timestamp}
  , market_timestamp_{market_timestamp}
  , market_price_{market_price}
  , position_size_{position_size}
  , investment_{investment}
  , entry_price_{entry_price}
  , total_entry_fees_{total_entry_fees}
  , stop_price_{stop_price}
  , target_price_{target_price}
  , stop_loss_price_{stop_loss_price}
  , take_profit_price_{take_profit_price}
  {
  }

  auto operator==(const OpenPositionSnapshot&) const noexcept -> bool = default;

  auto trade_id(this const OpenPositionSnapshot& self) noexcept -> std::size_t
  {
    return self.trade_id_;
  }

  auto entry_timestamp(this const OpenPositionSnapshot& self) noexcept
   -> std::time_t
  {
    return self.entry_timestamp_;
  }

  auto market_timestamp(this const OpenPositionSnapshot& self) noexcept
   -> std::time_t
  {
    return self.market_timestamp_;
  }

  auto market_price(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.market_price_;
  }

  auto position_size(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.position_size_;
  }

  auto investment(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.investment_;
  }

  auto entry_price(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.entry_price_;
  }

  auto total_entry_fees(this const OpenPositionSnapshot& self) noexcept
   -> double
  {
    return self.total_entry_fees_;
  }

  auto stop_price(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.stop_price_;
  }

  auto target_price(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.target_price_;
  }

  auto stop_loss_price(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.stop_loss_price_;
  }

  auto take_profit_price(this const OpenPositionSnapshot& self) noexcept
   -> double
  {
    return self.take_profit_price_;
  }

  auto average_price(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.position_size_ ? self.investment_ / self.position_size_ : 0.0;
  }

  auto unrealized_pnl(this const OpenPositionSnapshot& self) noexcept -> double
  {
    return self.position_size_ * (self.market_price_ - self.average_price());
  }

  auto duration(this const OpenPositionSnapshot& self) noexcept -> std::time_t
  {
    return self.market_timestamp_ - self.entry_timestamp_;
  }

private:
  std::size_t trade_id_{};
  std::time_t entry_timestamp_{};
  std::time_t market_timestamp_{};
  double market_price_{};
  double position_size_{};
  double investment_{};
  double entry_price_{};
  double total_entry_fees_{};
  double stop_price_{};
  double target_price_{};
  double stop_loss_price_{};
  double take_profit_price_{};
};

} // namespace pludux::backtest
