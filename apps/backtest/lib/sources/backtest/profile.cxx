module;

#include <string>
#include <utility>

export module pludux.backtest:profile;

import pludux;

import :execution_filter_method_context;
import :position_sizing;

export namespace pludux::backtest {

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
  : Profile{std::move(name), position_sizing, TrueNode{}}
  {
  }

  Profile(std::string name,
          PositionSizingNode position_sizing,
          ErasedNode<ExecutionFilterMethodContext> execution_filter)
  : name_{std::move(name)}
  , position_sizing_{position_sizing}
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
           self.execution_filter_ == other.execution_filter_;
  }

private:
  std::string name_;
  PositionSizingNode position_sizing_;
  ErasedNode<ExecutionFilterMethodContext> execution_filter_;
};

} // namespace pludux::backtest
