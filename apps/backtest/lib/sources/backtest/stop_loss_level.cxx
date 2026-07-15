module;

#include <cmath>

export module pludux.backtest:stop_loss_level;

export namespace pludux::backtest {

class StopLossLevel {
public:
  StopLossLevel() = default;

  StopLossLevel(double evaluated_price,
                double effective_price,
                bool enabled,
                bool trailing,
                bool consumed = false,
                double trail_distance = NAN,
                double favorable_anchor = NAN) noexcept
  : evaluated_price_{evaluated_price}
  , effective_price_{effective_price}
  , enabled_{enabled}
  , trailing_{trailing}
  , consumed_{consumed}
  , trail_distance_{trail_distance}
  , favorable_anchor_{favorable_anchor}
  {
  }

  auto operator==(const StopLossLevel&) const noexcept -> bool = default;

  auto evaluated_price(this const StopLossLevel& self) noexcept -> double
  {
    return self.evaluated_price_;
  }

  void evaluated_price(this StopLossLevel& self, double price) noexcept
  {
    self.evaluated_price_ = price;
  }

  auto effective_price(this const StopLossLevel& self) noexcept -> double
  {
    return self.effective_price_;
  }

  void effective_price(this StopLossLevel& self, double price) noexcept
  {
    self.effective_price_ = price;
  }

  auto enabled(this const StopLossLevel& self) noexcept -> bool
  {
    return self.enabled_;
  }

  auto trailing(this const StopLossLevel& self) noexcept -> bool
  {
    return self.trailing_;
  }

  auto consumed(this const StopLossLevel& self) noexcept -> bool
  {
    return self.consumed_;
  }

  void consumed(this StopLossLevel& self, bool consumed) noexcept
  {
    self.consumed_ = consumed;
  }

  auto active(this const StopLossLevel& self) noexcept -> bool
  {
    return self.enabled_ && !self.consumed_ &&
           std::isfinite(self.effective_price_);
  }

  auto trail_distance(this const StopLossLevel& self) noexcept -> double
  {
    return self.trail_distance_;
  }

  void trail_distance(this StopLossLevel& self, double distance) noexcept
  {
    self.trail_distance_ = distance;
  }

  auto favorable_anchor(this const StopLossLevel& self) noexcept -> double
  {
    return self.favorable_anchor_;
  }

  void favorable_anchor(this StopLossLevel& self, double price) noexcept
  {
    self.favorable_anchor_ = price;
  }

private:
  double evaluated_price_{};
  double effective_price_{};
  bool enabled_{};
  bool trailing_{};
  bool consumed_{};
  double trail_distance_{NAN};
  double favorable_anchor_{NAN};
};

} // namespace pludux::backtest
