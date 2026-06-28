module;

#include <string>
#include <utility>

export module pludux.backtest:profile;

import pludux;

export namespace pludux::backtest {

class PositionSizing {
public:
  enum class Mode { RiskDistance, FixedQuantity, FixedNotional, EquityPercent };

  PositionSizing()
  : PositionSizing{Mode::RiskDistance, 0.0}
  {
  }

  PositionSizing(Mode mode, double value)
  : mode_{mode}
  , value_{value}
  {
  }

  auto operator==(const PositionSizing&) const noexcept -> bool = default;

  auto mode(this const PositionSizing& self) noexcept -> Mode
  {
    return self.mode_;
  }

  void mode(this PositionSizing& self, Mode mode) noexcept
  {
    self.mode_ = mode;
  }

  auto value(this const PositionSizing& self) noexcept -> double
  {
    return self.value_;
  }

  void value(this PositionSizing& self, double value) noexcept
  {
    self.value_ = value;
  }

private:
  Mode mode_;
  double value_;
};

class DrawdownAdjustment {
public:
  DrawdownAdjustment()
  : DrawdownAdjustment{false, 0.10, 0.20}
  {
  }

  DrawdownAdjustment(bool enabled, double drawdown_step, double size_reduction)
  : enabled_{enabled}
  , drawdown_step_{drawdown_step}
  , size_reduction_{size_reduction}
  {
  }

  auto operator==(const DrawdownAdjustment&) const noexcept -> bool = default;

  auto enabled(this const DrawdownAdjustment& self) noexcept -> bool
  {
    return self.enabled_;
  }

  void enabled(this DrawdownAdjustment& self, bool enabled) noexcept
  {
    self.enabled_ = enabled;
  }

  auto drawdown_step(this const DrawdownAdjustment& self) noexcept -> double
  {
    return self.drawdown_step_;
  }

  void drawdown_step(this DrawdownAdjustment& self,
                     double drawdown_step) noexcept
  {
    self.drawdown_step_ = drawdown_step;
  }

  auto size_reduction(this const DrawdownAdjustment& self) noexcept -> double
  {
    return self.size_reduction_;
  }

  void size_reduction(this DrawdownAdjustment& self,
                      double size_reduction) noexcept
  {
    self.size_reduction_ = size_reduction;
  }

private:
  bool enabled_;
  double drawdown_step_;
  double size_reduction_;
};

class Profile {
public:
  Profile()
  : Profile{""}
  {
  }

  Profile(std::string name)
  : Profile{std::move(name),
            PositionSizing{PositionSizing::Mode::RiskDistance, 0.0}}
  {
  }

  Profile(std::string name, PositionSizing position_sizing)
  : Profile{std::move(name), position_sizing, DrawdownAdjustment{}}
  {
  }

  Profile(std::string name,
          PositionSizing position_sizing,
          DrawdownAdjustment drawdown_adjustment)
  : name_{std::move(name)}
  , position_sizing_{position_sizing}
  , drawdown_adjustment_{drawdown_adjustment}
  {
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
   -> const PositionSizing&
  {
    return self.position_sizing_;
  }

  void position_sizing(this Profile& self,
                       PositionSizing position_sizing) noexcept
  {
    self.position_sizing_ = position_sizing;
  }

  auto drawdown_adjustment(this const Profile& self) noexcept
   -> const DrawdownAdjustment&
  {
    return self.drawdown_adjustment_;
  }

  void drawdown_adjustment(this Profile& self,
                           DrawdownAdjustment drawdown_adjustment) noexcept
  {
    self.drawdown_adjustment_ = drawdown_adjustment;
  }

  auto equivalent_rules(this const Profile& self, const Profile& other) noexcept
   -> bool
  {
    return self.position_sizing_ == other.position_sizing_ &&
           self.drawdown_adjustment_ == other.drawdown_adjustment_;
  }

private:
  std::string name_;
  PositionSizing position_sizing_;
  DrawdownAdjustment drawdown_adjustment_;
};

} // namespace pludux::backtest
