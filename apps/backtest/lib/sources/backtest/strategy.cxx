module;

#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:strategy;

import pludux;

import :trade_entry;
import :trade_exit;
import :execution_model;
import :plot_group;
import :risk_distance_node;
import :stop_target_price_node;

export namespace pludux::backtest {

enum class StopTargetReferencePrice {
  LatestEntryPrice,
  AveragePrice,
  InitialEntryPrice
};

class Strategy {
public:
  class Entry {
  public:
    Entry()
    : Entry(FalseNode{}, SignalTiming::NextOpen)
    {
    }

    Entry(ErasedNode signal, SignalTiming timing = SignalTiming::NextOpen)
    : signal_{std::move(signal)}
    , timing_{timing}
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

    auto timing(this const Entry& self) noexcept -> SignalTiming
    {
      return self.timing_;
    }

    void timing(this Entry& self, SignalTiming timing) noexcept
    {
      self.timing_ = timing;
    }

  private:
    ErasedNode signal_;
    SignalTiming timing_;
  };

  class Exit {
  public:
    Exit()
    : Exit(false, FalseNode{}, SignalTiming::NextOpen)
    {
    }

    Exit(bool enabled,
         ErasedNode signal,
         SignalTiming timing = SignalTiming::NextOpen,
         double reduce = 1.0)
    : enabled_{enabled}
    , signal_{std::move(signal)}
    , timing_{timing}
    , reduce_{reduce}
    {
    }

    auto operator==(const Exit&) const noexcept -> bool = default;

    auto enabled(this const Exit& self) noexcept -> bool
    {
      return self.enabled_;
    }

    void enabled(this Exit& self, bool enabled) noexcept
    {
      self.enabled_ = enabled;
    }

    auto signal(this const Exit& self) noexcept -> const ErasedNode&
    {
      return self.signal_;
    }

    void signal(this Exit& self, ErasedNode signal) noexcept
    {
      self.signal_ = std::move(signal);
    }

    auto timing(this const Exit& self) noexcept -> SignalTiming
    {
      return self.timing_;
    }

    void timing(this Exit& self, SignalTiming timing) noexcept
    {
      self.timing_ = timing;
    }

    auto reduce(this const Exit& self) noexcept -> double
    {
      return self.reduce_;
    }

    void reduce(this Exit& self, double reduce) noexcept
    {
      self.reduce_ = reduce;
    }

  private:
    bool enabled_;
    ErasedNode signal_;
    SignalTiming timing_;
    double reduce_;
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

    auto timing(this const Pyramiding& self) noexcept -> SignalTiming
    {
      return self.timing_;
    }

    void timing(this Pyramiding& self, SignalTiming timing) noexcept
    {
      self.timing_ = timing;
    }

    auto max_layers(this const Pyramiding& self) noexcept -> std::size_t
    {
      return self.max_layers_;
    }

    void max_layers(this Pyramiding& self, std::size_t max_layers) noexcept
    {
      self.max_layers_ = max_layers;
    }

    auto favorable_stop_target_reference(this const Pyramiding& self) noexcept
     -> StopTargetReferencePrice
    {
      return self.favorable_stop_target_reference_;
    }

    void
    favorable_stop_target_reference(this Pyramiding& self,
                                    StopTargetReferencePrice reference) noexcept
    {
      self.favorable_stop_target_reference_ = reference;
    }

    auto unfavorable_stop_target_reference(this const Pyramiding& self) noexcept
     -> StopTargetReferencePrice
    {
      return self.unfavorable_stop_target_reference_;
    }

    void unfavorable_stop_target_reference(
     this Pyramiding& self, StopTargetReferencePrice reference) noexcept
    {
      self.unfavorable_stop_target_reference_ = reference;
    }

  private:
    ErasedNode signal_{FalseNode{}};
    SignalTiming timing_{SignalTiming::NextOpen};
    std::size_t max_layers_{1};
    StopTargetReferencePrice favorable_stop_target_reference_{
     StopTargetReferencePrice::AveragePrice};
    StopTargetReferencePrice unfavorable_stop_target_reference_{
     StopTargetReferencePrice::AveragePrice};
  };

  class TakeProfit {
  public:
    TakeProfit(bool enabled = false,
               ErasedNode target_price = TpRMultipleNode{2.0},
               double reduce = 1.0)
    : enabled_{enabled}
    , target_price_{std::move(target_price)}
    , reduce_{reduce}
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

    auto reduce(this const TakeProfit& self) noexcept -> double
    {
      return self.reduce_;
    }

    void reduce(this TakeProfit& self, double reduce) noexcept
    {
      self.reduce_ = reduce;
    }

  private:
    bool enabled_;
    ErasedNode target_price_;
    double reduce_;
  };

  class StopLoss {
  public:
    StopLoss(bool enabled = true,
             ErasedNode stop_price = Sl1RNode{},
             bool trailing = false,
             double reduce = 1.0)
    : enabled_{enabled}
    , stop_price_{std::move(stop_price)}
    , trailing_{trailing}
    , reduce_{reduce}
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

    auto reduce(this const StopLoss& self) noexcept -> double
    {
      return self.reduce_;
    }

    void reduce(this StopLoss& self, double reduce) noexcept
    {
      self.reduce_ = reduce;
    }

  private:
    bool enabled_;
    bool trailing_;
    ErasedNode stop_price_;
    double reduce_;
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

    auto exits(this const Position& self) noexcept -> const std::vector<Exit>&
    {
      return self.exits_;
    }

    auto exits(this Position& self) noexcept -> std::vector<Exit>&
    {
      return self.exits_;
    }

    void exits(this Position& self, std::vector<Exit> exits) noexcept
    {
      self.exits_ = std::move(exits);
    }

    auto exits_activation(this const Position& self) noexcept -> ExitActivation
    {
      return self.exits_activation_;
    }

    void exits_activation(this Position& self,
                          ExitActivation activation) noexcept
    {
      self.exits_activation_ = activation;
    }

    auto pyramiding(this const Position& self) noexcept -> const Pyramiding&
    {
      return self.pyramiding_;
    }

    void pyramiding(this Position& self, Pyramiding pyramiding) noexcept
    {
      self.pyramiding_ = std::move(pyramiding);
    }

    auto take_profits(this const Position& self) noexcept
     -> const std::vector<TakeProfit>&
    {
      return self.take_profits_;
    }

    auto take_profits(this Position& self) noexcept -> std::vector<TakeProfit>&
    {
      return self.take_profits_;
    }

    void take_profits(this Position& self,
                      std::vector<TakeProfit> take_profits) noexcept
    {
      self.take_profits_ = std::move(take_profits);
    }

    auto take_profits_activation(this const Position& self) noexcept
     -> ExitActivation
    {
      return self.take_profits_activation_;
    }

    void take_profits_activation(this Position& self,
                                 ExitActivation activation) noexcept
    {
      self.take_profits_activation_ = activation;
    }

    auto risk_distance(this const Position& self) noexcept -> const ErasedNode&
    {
      return self.risk_distance_;
    }

    void risk_distance(this Position& self, ErasedNode risk_distance) noexcept
    {
      self.risk_distance_ = std::move(risk_distance);
    }

    auto stop_losses(this const Position& self) noexcept
     -> const std::vector<StopLoss>&
    {
      return self.stop_losses_;
    }

    void stop_losses(this Position& self,
                     std::vector<StopLoss> stop_losses) noexcept
    {
      self.stop_losses_ = std::move(stop_losses);
    }

    auto stop_losses_activation(this const Position& self) noexcept
     -> ExitActivation
    {
      return self.stop_losses_activation_;
    }

    void stop_losses_activation(this Position& self,
                                ExitActivation activation) noexcept
    {
      self.stop_losses_activation_ = activation;
    }

  private:
    Entry entry_;
    std::vector<Exit> exits_;
    ExitActivation exits_activation_{ExitActivation::Simultaneous};
    Pyramiding pyramiding_;
    ErasedNode risk_distance_{RiskDistanceAtrNode{}};
    std::vector<TakeProfit> take_profits_;
    ExitActivation take_profits_activation_{ExitActivation::Simultaneous};
    std::vector<StopLoss> stop_losses_{StopLoss{}};
    ExitActivation stop_losses_activation_{ExitActivation::Simultaneous};
  };

  Strategy()
  : Strategy("",
             OrderedNamedRegistry<ErasedNode>{},
             Position{},
             Position{},
             std::vector<PlotGroup>{},
             IntrabarPath::CandleDirection)
  {
  }

  Strategy(std::string name,
           OrderedNamedRegistry<ErasedNode> series_nodes,
           Position long_position,
           Position short_position,
           std::vector<PlotGroup> plots,
           IntrabarPath intrabar_path = IntrabarPath::CandleDirection)
  : name_{std::move(name)}
  , series_nodes_{std::move(series_nodes)}
  , long_position_{std::move(long_position)}
  , short_position_{std::move(short_position)}
  , plots_{std::move(plots)}
  , intrabar_path_{intrabar_path}
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

  auto intrabar_path(this const Strategy& self) noexcept -> IntrabarPath
  {
    return self.intrabar_path_;
  }

  void intrabar_path(this Strategy& self, IntrabarPath path) noexcept
  {
    self.intrabar_path_ = path;
  }

  auto equivalent_rules(this const Strategy& self,
                        const Strategy& other) noexcept -> bool
  {
    return self.series_nodes_ == other.series_nodes_ &&
           self.long_position_ == other.long_position_ &&
           self.short_position_ == other.short_position_ &&
           self.intrabar_path_ == other.intrabar_path_;
  }

private:
  std::string name_;

  OrderedNamedRegistry<ErasedNode> series_nodes_;

  Position long_position_;
  Position short_position_;

  std::vector<PlotGroup> plots_;
  IntrabarPath intrabar_path_;
};

} // namespace pludux::backtest
