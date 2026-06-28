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
  class Entry {
  public:
    Entry()
    : Entry(FalseNode{})
    {
    }

    Entry(ErasedNode signal)
    : signal_{std::move(signal)}
    {
    }

    auto operator==(const Entry&) const noexcept -> bool = default;

    auto signal(this const Entry& self) noexcept -> const ErasedNode&
    {
      return self.signal_;
    }

    void signal(this Entry& self, ErasedNode signal) noexcept
    {
      self.signal_ = std::move(signal);
    }

  private:
    ErasedNode signal_;
  };

  class Exit {
  public:
    Exit()
    : Exit(FalseNode{})
    {
    }

    Exit(ErasedNode signal)
    : signal_{std::move(signal)}
    {
    }

    auto operator==(const Exit&) const noexcept -> bool = default;

    auto signal(this const Exit& self) noexcept -> const ErasedNode&
    {
      return self.signal_;
    }

    void signal(this Exit& self, ErasedNode signal) noexcept
    {
      self.signal_ = std::move(signal);
    }

  private:
    ErasedNode signal_;
  };

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

  class TakeProfit {
  public:
    TakeProfit(bool enabled = false, ErasedNode target_price = OpenNode{})
    : enabled_{enabled}
    , target_price_{std::move(target_price)}
    {
    }

    auto operator==(const TakeProfit&) const noexcept -> bool = default;

    auto enabled(this const TakeProfit& self) noexcept -> bool
    {
      return self.enabled_;
    }

    void enabled(this TakeProfit& self, bool enabled) noexcept
    {
      self.enabled_ = enabled;
    }

    auto target_price(this const TakeProfit& self) noexcept -> const ErasedNode&
    {
      return self.target_price_;
    }

    void target_price(this TakeProfit& self, ErasedNode target_price) noexcept
    {
      self.target_price_ = std::move(target_price);
    }

  private:
    bool enabled_;
    ErasedNode target_price_;
  };

  class StopLoss {
  public:
    StopLoss(bool enabled = false,
             ErasedNode stop_price = OpenNode{},
             bool trailing = false)
    : enabled_{enabled}
    , stop_price_{std::move(stop_price)}
    , trailing_{trailing}
    {
    }

    auto operator==(const StopLoss&) const noexcept -> bool = default;

    auto enabled(this const StopLoss& self) noexcept -> bool
    {
      return self.enabled_;
    }

    void enabled(this StopLoss& self, bool enabled) noexcept
    {
      self.enabled_ = enabled;
    }

    auto trailing(this const StopLoss& self) noexcept -> bool
    {
      return self.trailing_;
    }

    void trailing(this StopLoss& self, bool trailing) noexcept
    {
      self.trailing_ = trailing;
    }

    auto stop_price(this const StopLoss& self) noexcept -> const ErasedNode&
    {
      return self.stop_price_;
    }

    void stop_price(this StopLoss& self, ErasedNode stop_price) noexcept
    {
      self.stop_price_ = std::move(stop_price);
    }

  private:
    bool enabled_;
    bool trailing_;
    ErasedNode stop_price_;
  };

  class Position {
  public:
    Position() = default;

    auto operator==(const Position&) const noexcept -> bool = default;

    auto entry(this const Position& self) noexcept -> const Entry&
    {
      return self.entry_;
    }

    void entry(this Position& self, Entry entry) noexcept
    {
      self.entry_ = std::move(entry);
    }

    auto exit(this const Position& self) noexcept -> const Exit&
    {
      return self.exit_;
    }

    void exit(this Position& self, Exit exit) noexcept
    {
      self.exit_ = std::move(exit);
    }

    auto pyramiding(this const Position& self) noexcept -> const Pyramiding&
    {
      return self.pyramiding_;
    }

    void pyramiding(this Position& self, Pyramiding pyramiding) noexcept
    {
      self.pyramiding_ = std::move(pyramiding);
    }

    auto take_profit(this const Position& self) noexcept -> const TakeProfit&
    {
      return self.take_profit_;
    }

    void take_profit(this Position& self, TakeProfit take_profit) noexcept
    {
      self.take_profit_ = std::move(take_profit);
    }

    auto stop_loss(this const Position& self) noexcept -> const StopLoss&
    {
      return self.stop_loss_;
    }

    void stop_loss(this Position& self, StopLoss stop_loss) noexcept
    {
      self.stop_loss_ = std::move(stop_loss);
    }

  private:
    Entry entry_;
    Exit exit_;
    Pyramiding pyramiding_;
    TakeProfit take_profit_;
    StopLoss stop_loss_;
  };

  Strategy()
  : Strategy("",
             OrderedNamedRegistry<ErasedNode>{},
             Position{},
             Position{},
             std::vector<PlotGroup>{})
  {
  }

  Strategy(std::string name,
           OrderedNamedRegistry<ErasedNode> series_nodes,
           Position long_position,
           Position short_position,
           std::vector<PlotGroup> plots)
  : name_{std::move(name)}
  , series_nodes_{std::move(series_nodes)}
  , long_position_{std::move(long_position)}
  , short_position_{std::move(short_position)}
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

  void series_nodes(this Strategy& self,
                    OrderedNamedRegistry<ErasedNode> series_nodes) noexcept
  {
    self.series_nodes_ = std::move(series_nodes);
  }

  auto long_position(this const Strategy& self) noexcept -> const Position&
  {
    return self.long_position_;
  }

  void long_position(this Strategy& self, Position long_position) noexcept
  {
    self.long_position_ = std::move(long_position);
  }

  auto short_position(this const Strategy& self) noexcept -> const Position&
  {
    return self.short_position_;
  }

  void short_position(this Strategy& self, Position short_position) noexcept
  {
    self.short_position_ = std::move(short_position);
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
           self.long_position_ == other.long_position_ &&
           self.short_position_ == other.short_position_;
  }

private:
  std::string name_;

  OrderedNamedRegistry<ErasedNode> series_nodes_;

  Position long_position_;
  Position short_position_;

  std::vector<PlotGroup> plots_;
};

} // namespace pludux::backtest
