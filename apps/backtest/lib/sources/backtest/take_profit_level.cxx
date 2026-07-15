module;

#include <cmath>

export module pludux.backtest:take_profit_level;

export namespace pludux::backtest {

class TakeProfitLevel {
public:
  TakeProfitLevel() = default;

  TakeProfitLevel(double price, bool enabled, bool consumed = false) noexcept
  : price_{price}
  , enabled_{enabled}
  , consumed_{consumed}
  {
  }

  auto operator==(const TakeProfitLevel&) const noexcept -> bool = default;

  auto price(this const TakeProfitLevel& self) noexcept -> double
  {
    return self.price_;
  }

  void price(this TakeProfitLevel& self, double price) noexcept
  {
    self.price_ = price;
  }

  auto enabled(this const TakeProfitLevel& self) noexcept -> bool
  {
    return self.enabled_;
  }

  auto consumed(this const TakeProfitLevel& self) noexcept -> bool
  {
    return self.consumed_;
  }

  void consumed(this TakeProfitLevel& self, bool consumed) noexcept
  {
    self.consumed_ = consumed;
  }

  auto active(this const TakeProfitLevel& self) noexcept -> bool
  {
    return self.enabled_ && !self.consumed_ && std::isfinite(self.price_);
  }

private:
  double price_{};
  bool enabled_{};
  bool consumed_{};
};

} // namespace pludux::backtest
