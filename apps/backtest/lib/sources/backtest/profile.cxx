module;

#include <string>
#include <utility>

export module pludux.backtest:profile;

import pludux;

import :execution_filter_method_context;
import :position_sizing;

export namespace pludux::backtest {

enum class InsufficientCashPolicy { Reject, CapToAvailableCash };

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
          DrawdownAdjustment drawdown_adjustment)
  : Profile{std::move(name),
            position_sizing,
            drawdown_adjustment,
            InsufficientCashPolicy::Reject}
  {
  }

  Profile(
   std::string name,
   PositionSizingNode position_sizing,
   DrawdownAdjustment drawdown_adjustment,
   InsufficientCashPolicy insufficient_cash_policy,
   ErasedNode<ExecutionFilterMethodContext> execution_filter = TrueNode{})
  : name_{std::move(name)}
  , position_sizing_{position_sizing}
  , drawdown_adjustment_{drawdown_adjustment}
  , insufficient_cash_policy_{insufficient_cash_policy}
  , execution_filter_{std::move(execution_filter)}
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
                           DrawdownAdjustment drawdown_adjustment) noexcept
  {
    self.drawdown_adjustment_ = drawdown_adjustment;
  }

  auto insufficient_cash_policy(this const Profile& self) noexcept
   -> InsufficientCashPolicy
  {
    return self.insufficient_cash_policy_;
  }

  void insufficient_cash_policy(
   this Profile& self, InsufficientCashPolicy insufficient_cash_policy) noexcept
  {
    self.insufficient_cash_policy_ = insufficient_cash_policy;
  }

  auto execution_filter(this const Profile& self) noexcept
   -> const ErasedNode<ExecutionFilterMethodContext>&
  {
    return self.execution_filter_;
  }

  void execution_filter(
   this Profile& self,
   ErasedNode<ExecutionFilterMethodContext> execution_filter) noexcept
  {
    self.execution_filter_ = std::move(execution_filter);
  }

  auto equivalent_rules(this const Profile& self, const Profile& other) noexcept
   -> bool
  {
    return self.position_sizing_ == other.position_sizing_ &&
           self.drawdown_adjustment_ == other.drawdown_adjustment_ &&
           self.insufficient_cash_policy_ == other.insufficient_cash_policy_ &&
           self.execution_filter_ == other.execution_filter_;
  }

private:
  std::string name_;
  PositionSizingNode position_sizing_;
  DrawdownAdjustment drawdown_adjustment_;
  InsufficientCashPolicy insufficient_cash_policy_;
  ErasedNode<ExecutionFilterMethodContext> execution_filter_;
};

} // namespace pludux::backtest
