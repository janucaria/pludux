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
  : name_{std::move(name)}
  , position_sizing_{position_sizing}
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

  auto equivalent_rules(this const Profile& self, const Profile& other) noexcept
   -> bool
  {
    return self.position_sizing_ == other.position_sizing_;
  }

private:
  std::string name_;
  PositionSizing position_sizing_;
};

} // namespace pludux::backtest
