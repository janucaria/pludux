module;

#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:strategy;

import pludux;

import :trade_entry;
import :trade_exit;
import :plot_group;

export namespace pludux::backtest {

class Strategy {
public:
  class Pyramiding {
  public:
    Pyramiding() = default;

    auto operator==(const Pyramiding&) const noexcept -> bool = default;

    auto signal(this const Pyramiding& self) noexcept -> const ErasedNode&
    {
      return self.signal_;
    }

    void signal(this Pyramiding& self, ErasedNode signal) noexcept
    {
      self.signal_ = std::move(signal);
    }

    auto max_layers(this const Pyramiding& self) noexcept -> std::size_t
    {
      return self.max_layers_;
    }

    void max_layers(this Pyramiding& self, std::size_t max_layers) noexcept
    {
      self.max_layers_ = max_layers;
    }

  private:
    ErasedNode signal_{FalseNode{}};
    std::size_t max_layers_{1};
  };

  class PositionSide {
  public:
    PositionSide() = default;

    auto operator==(const PositionSide&) const noexcept -> bool = default;

    auto pyramiding(this const PositionSide& self) noexcept -> const Pyramiding&
    {
      return self.pyramiding_;
    }

    void pyramiding(this PositionSide& self, Pyramiding pyramiding) noexcept
    {
      self.pyramiding_ = std::move(pyramiding);
    }

  private:
    Pyramiding pyramiding_;
  };

  class Positions {
  public:
    Positions() = default;

    auto operator==(const Positions&) const noexcept -> bool = default;

    auto long_side(this const Positions& self) noexcept -> const PositionSide&
    {
      return self.long_side_;
    }

    void long_side(this Positions& self,
                   PositionSide long_position_side) noexcept
    {
      self.long_side_ = std::move(long_position_side);
    }

    auto short_side(this const Positions& self) noexcept -> const PositionSide&
    {
      return self.short_side_;
    }

    void short_side(this Positions& self,
                    PositionSide short_position_side) noexcept
    {
      self.short_side_ = std::move(short_position_side);
    }

  private:
    PositionSide long_side_;
    PositionSide short_side_;
  };

  Strategy()
  : Strategy("",
             OrderedNamedRegistry<ErasedNode>{},
             FalseNode{},
             FalseNode{},
             FalseNode{},
             FalseNode{},
             Positions{},
             false,
             false,
             false,
             1.0,
             {})
  {
  }

  Strategy(std::string name,
           OrderedNamedRegistry<ErasedNode> series_nodes,
           ErasedNode long_entry_node,
           ErasedNode long_exit_node,
           ErasedNode short_entry_node,
           ErasedNode short_exit_node,
           Positions position,
           bool stop_loss_enabled,
           bool stop_loss_trailing_enabled,
           bool take_profit_enabled,
           double take_profit_r_multiple,
           std::vector<PlotGroup> plots)
  : name_{std::move(name)}
  , series_nodes_{series_nodes}
  , long_entry_node_{std::move(long_entry_node)}
  , long_exit_node_{std::move(long_exit_node)}
  , short_entry_node_{std::move(short_entry_node)}
  , short_exit_node_{std::move(short_exit_node)}
  , positions_{std::move(position)}
  , stop_loss_enabled_{stop_loss_enabled}
  , stop_loss_trailing_enabled_{stop_loss_trailing_enabled}
  , take_profit_enabled_{take_profit_enabled}
  , take_profit_r_multiple_{take_profit_r_multiple}
  , plots_{std::move(plots)}
  {
  }

  auto operator==(const Strategy&) const noexcept -> bool = default;

  auto name(this const Strategy& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Strategy& self, std::string name) noexcept
  {
    self.name_ = std::move(name);
  }

  auto series_nodes(this const Strategy& self) noexcept
   -> const OrderedNamedRegistry<ErasedNode>&
  {
    return self.series_nodes_;
  }

  auto series_nodes(this Strategy& self) noexcept
   -> OrderedNamedRegistry<ErasedNode>&
  {
    return self.series_nodes_;
  }

  auto long_entry_node(this const Strategy& self) noexcept
   -> const ErasedNode&
  {
    return self.long_entry_node_;
  }

  void long_entry_node(this Strategy& self,
                         ErasedNode long_entry_node) noexcept
  {
    self.long_entry_node_ = std::move(long_entry_node);
  }

  auto long_exit_node(this const Strategy& self) noexcept -> const ErasedNode&
  {
    return self.long_exit_node_;
  }

  void long_exit_node(this Strategy& self,
                        ErasedNode long_exit_node) noexcept
  {
    self.long_exit_node_ = std::move(long_exit_node);
  }

  auto short_entry_node(this const Strategy& self) noexcept
   -> const ErasedNode&
  {
    return self.short_entry_node_;
  }

  void short_entry_node(this Strategy& self,
                          ErasedNode short_entry_node) noexcept
  {
    self.short_entry_node_ = std::move(short_entry_node);
  }

  auto short_exit_node(this const Strategy& self) noexcept
   -> const ErasedNode&
  {
    return self.short_exit_node_;
  }

  void short_exit_node(this Strategy& self,
                         ErasedNode short_exit_node) noexcept
  {
    self.short_exit_node_ = std::move(short_exit_node);
  }

  auto positions(this const Strategy& self) noexcept -> const Positions&
  {
    return self.positions_;
  }

  void positions(this Strategy& self, Positions positions) noexcept
  {
    self.positions_ = std::move(positions);
  }

  auto stop_loss_enabled(this const Strategy& self) noexcept -> bool
  {
    return self.stop_loss_enabled_;
  }

  void stop_loss_enabled(this Strategy& self, bool stop_loss_enabled) noexcept
  {
    self.stop_loss_enabled_ = stop_loss_enabled;
  }

  auto stop_loss_trailing_enabled(this const Strategy& self) noexcept -> bool
  {
    return self.stop_loss_trailing_enabled_;
  }

  void stop_loss_trailing_enabled(this Strategy& self,
                                  bool stop_loss_trailing_enabled) noexcept
  {
    self.stop_loss_trailing_enabled_ = stop_loss_trailing_enabled;
  }

  auto take_profit_enabled(this const Strategy& self) noexcept -> bool
  {
    return self.take_profit_enabled_;
  }

  void take_profit_enabled(this Strategy& self,
                           bool take_profit_enabled) noexcept
  {
    self.take_profit_enabled_ = take_profit_enabled;
  }

  auto take_profit_r_multiple(this const Strategy& self) noexcept -> double
  {
    return self.take_profit_r_multiple_;
  }

  void take_profit_r_multiple(this Strategy& self,
                              double take_profit_r_multiple) noexcept
  {
    self.take_profit_r_multiple_ = take_profit_r_multiple;
  }

  auto plots(this const Strategy& self) noexcept
   -> const std::vector<PlotGroup>&
  {
    return self.plots_;
  }

  void plots(this Strategy& self, std::vector<PlotGroup> plots) noexcept
  {
    self.plots_ = std::move(plots);
  }

  auto equivalent_rules(this const Strategy& self,
                        const Strategy& other) noexcept -> bool
  {
    return self.series_nodes_ == other.series_nodes_ &&
           self.long_entry_node_ == other.long_entry_node_ &&
           self.long_exit_node_ == other.long_exit_node_ &&
           self.short_entry_node_ == other.short_entry_node_ &&
           self.short_exit_node_ == other.short_exit_node_ &&
           self.positions_ == other.positions_ &&
           self.stop_loss_enabled_ == other.stop_loss_enabled_ &&
           self.stop_loss_trailing_enabled_ ==
            other.stop_loss_trailing_enabled_ &&
           self.take_profit_enabled_ == other.take_profit_enabled_ &&
           self.take_profit_r_multiple_ == other.take_profit_r_multiple_;
  }

private:
  std::string name_;

  OrderedNamedRegistry<ErasedNode> series_nodes_;

  ErasedNode long_entry_node_{FalseNode{}};
  ErasedNode long_exit_node_{FalseNode{}};

  ErasedNode short_entry_node_{FalseNode{}};
  ErasedNode short_exit_node_{FalseNode{}};

  Positions positions_;

  bool stop_loss_enabled_{false};
  bool stop_loss_trailing_enabled_{false};

  bool take_profit_enabled_{false};
  double take_profit_r_multiple_{1.0};

  std::vector<PlotGroup> plots_;
};

} // namespace pludux::backtest
