module;

#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

export module pludux.backtest:profile;

import pludux;

import :position_sizing;

export namespace pludux::backtest {

enum class InsufficientCashPolicy { Reject, CapToAvailableCash };

class DrawdownAdjustment {
public:
  DrawdownAdjustment()
  : DrawdownAdjustment{false, 0.10, 0.20, 0.0}
  {
  }

  DrawdownAdjustment(bool enabled,
                     double drawdown_step,
                     double size_reduction,
                     double notional_equity_reduction)
  : enabled_{enabled}
  , drawdown_step_{drawdown_step}
  , size_reduction_{size_reduction}
  , notional_equity_reduction_{notional_equity_reduction}
  {
    validate_drawdown_step(drawdown_step_);
    validate_reduction(size_reduction_, "size reduction");
    validate_reduction(notional_equity_reduction_, "notional equity reduction");
  }

  auto operator==(const DrawdownAdjustment&) const noexcept -> bool = default;

  auto enabled(this const DrawdownAdjustment& self) noexcept -> bool
  {
    return self.enabled_;
  }

  void enabled(this DrawdownAdjustment& self, bool value) noexcept
  {
    self.enabled_ = value;
  }

  auto drawdown_step(this const DrawdownAdjustment& self) noexcept -> double
  {
    return self.drawdown_step_;
  }

  void drawdown_step(this DrawdownAdjustment& self, double value)
  {
    validate_drawdown_step(value);
    self.drawdown_step_ = value;
  }

  auto size_reduction(this const DrawdownAdjustment& self) noexcept -> double
  {
    return self.size_reduction_;
  }

  void size_reduction(this DrawdownAdjustment& self, double value)
  {
    validate_reduction(value, "size reduction");
    self.size_reduction_ = value;
  }

  auto notional_equity_reduction(this const DrawdownAdjustment& self) noexcept
   -> double
  {
    return self.notional_equity_reduction_;
  }

  void notional_equity_reduction(this DrawdownAdjustment& self, double value)
  {
    validate_reduction(value, "notional equity reduction");
    self.notional_equity_reduction_ = value;
  }

private:
  bool enabled_;
  double drawdown_step_;
  double size_reduction_;
  double notional_equity_reduction_;

  static void validate_drawdown_step(double value)
  {
    if(!std::isfinite(value) || value <= 0.0) {
      throw std::invalid_argument{
       "Drawdown adjustment step must be finite and positive"};
    }
  }

  static void validate_reduction(double value, const char* label)
  {
    if(!std::isfinite(value) || value < 0.0) {
      throw std::invalid_argument{std::string{"Drawdown adjustment "} + label +
                                  " must be finite and non-negative"};
    }
  }
};

class Profile {
public:
  Profile()
  : Profile{""}
  {
  }

  Profile(std::string name)
  : Profile{std::move(name), PositionSizingNode{RiskDistancePositionSizing{}}}
  {
  }

  Profile(std::string name, PositionSizingNode position_sizing)
  : Profile{std::move(name),
            position_sizing,
            DrawdownAdjustment{},
            InsufficientCashPolicy::Reject}
  {
  }

  Profile(std::string name,
          PositionSizingNode position_sizing,
          DrawdownAdjustment drawdown_adjustment,
          InsufficientCashPolicy insufficient_cash_policy)
  : name_{std::move(name)}
  , position_sizing_{position_sizing}
  , drawdown_adjustment_{drawdown_adjustment}
  , insufficient_cash_policy_{insufficient_cash_policy}
  {
    static_cast<void>(position_sizing_.make_method());
  }

  auto operator==(const Profile&) const noexcept -> bool = default;

  auto name(this const Profile& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Profile& self, std::string name) noexcept
  {
    self.name_ = std::move(name);
  }

  auto position_sizing(this const Profile& self) noexcept
   -> const PositionSizingNode&
  {
    return self.position_sizing_;
  }

  void position_sizing(this Profile& self, PositionSizingNode position_sizing)
  {
    static_cast<void>(position_sizing.make_method());
    self.position_sizing_ = std::move(position_sizing);
  }

  auto drawdown_adjustment(this const Profile& self) noexcept
   -> const DrawdownAdjustment&
  {
    return self.drawdown_adjustment_;
  }

  void drawdown_adjustment(this Profile& self,
                           DrawdownAdjustment value) noexcept
  {
    self.drawdown_adjustment_ = value;
  }

  auto insufficient_cash_policy(this const Profile& self) noexcept
   -> InsufficientCashPolicy
  {
    return self.insufficient_cash_policy_;
  }

  void insufficient_cash_policy(this Profile& self,
                                InsufficientCashPolicy value) noexcept
  {
    self.insufficient_cash_policy_ = value;
  }

  auto equivalent_rules(this const Profile& self, const Profile& other) noexcept
   -> bool
  {
    return self.position_sizing_ == other.position_sizing_ &&
           self.drawdown_adjustment_ == other.drawdown_adjustment_ &&
           self.insufficient_cash_policy_ == other.insufficient_cash_policy_;
  }

private:
  std::string name_;
  PositionSizingNode position_sizing_;
  DrawdownAdjustment drawdown_adjustment_;
  InsufficientCashPolicy insufficient_cash_policy_;
};

} // namespace pludux::backtest
