module;

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

export module pludux.backtest:portfolio;

import pludux;

import :requested_order_node;
import :store_handle;

export namespace pludux::backtest {

enum class PortfolioEntryComparatorOrder { HigherFirst, LowerFirst };

class PortfolioEntryComparator {
public:
  PortfolioEntryComparator()
  : PortfolioEntryComparator{ValueNode{0.0},
                             PortfolioEntryComparatorOrder::HigherFirst}
  {
  }

  PortfolioEntryComparator(ErasedNode<ErasedSeriesMethodContext> expression,
                           PortfolioEntryComparatorOrder order)
  : expression_{std::move(expression)}
  , order_{order}
  {
    validate_expression(expression_);
  }

  auto operator==(const PortfolioEntryComparator&) const noexcept
   -> bool = default;

  auto expression(this const PortfolioEntryComparator& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.expression_;
  }

  void expression(this PortfolioEntryComparator& self,
                  ErasedNode<ErasedSeriesMethodContext> value)
  {
    validate_expression(value);
    self.expression_ = std::move(value);
  }

  auto order(this const PortfolioEntryComparator& self) noexcept
   -> PortfolioEntryComparatorOrder
  {
    return self.order_;
  }

  void order(this PortfolioEntryComparator& self,
             PortfolioEntryComparatorOrder value) noexcept
  {
    self.order_ = value;
  }

private:
  ErasedNode<ErasedSeriesMethodContext> expression_;
  PortfolioEntryComparatorOrder order_;

  static void
  validate_expression(const ErasedNode<ErasedSeriesMethodContext>& expression)
  {
    if(!is_portfolio_entry_comparator_expression(expression)) {
      throw std::invalid_argument{
       "Portfolio comparator requires a Requested Order or asset-data "
       "expression"};
    }
  }
};

class Portfolio {
public:
  Portfolio()
  : Portfolio{
     "", 1'000'000.0, MarketStoreHandle{}, BrokerStoreHandle{}, 10, 10, {}, {}}
  {
  }

  Portfolio(std::string name,
            double initial_capital,
            MarketStoreHandle market_handle,
            BrokerStoreHandle broker_handle,
            std::size_t maximum_open_trades,
            std::size_t maximum_combined_layers,
            std::vector<PortfolioEntryComparator> entry_comparators,
             std::vector<SystemStoreHandle> system_handles)
  : name_{std::move(name)}
  , initial_capital_{initial_capital}
  , market_handle_{market_handle}
  , broker_handle_{broker_handle}
  , maximum_open_trades_{maximum_open_trades}
  , maximum_combined_layers_{maximum_combined_layers}
  , entry_comparators_{std::move(entry_comparators)}
    , system_handles_{std::move(system_handles)}
  {
    self_validate();
  }

  auto operator==(const Portfolio&) const noexcept -> bool = default;

  auto name(this const Portfolio& self) noexcept -> const std::string&
  {
    return self.name_;
  }

  void name(this Portfolio& self, std::string value) noexcept
  {
    self.name_ = std::move(value);
  }

  auto initial_capital(this const Portfolio& self) noexcept -> double
  {
    return self.initial_capital_;
  }

  void initial_capital(this Portfolio& self, double value)
  {
    if(!std::isfinite(value) || value < 0.0) {
      throw std::invalid_argument{
       "Portfolio initial capital must be finite and non-negative"};
    }
    self.initial_capital_ = value;
  }

  auto market_handle(this const Portfolio& self) noexcept -> MarketStoreHandle
  {
    return self.market_handle_;
  }

  void market_handle(this Portfolio& self, MarketStoreHandle value) noexcept
  {
    self.market_handle_ = value;
  }

  auto broker_handle(this const Portfolio& self) noexcept -> BrokerStoreHandle
  {
    return self.broker_handle_;
  }

  void broker_handle(this Portfolio& self, BrokerStoreHandle value) noexcept
  {
    self.broker_handle_ = value;
  }

  auto maximum_open_trades(this const Portfolio& self) noexcept -> std::size_t
  {
    return self.maximum_open_trades_;
  }

  void maximum_open_trades(this Portfolio& self, std::size_t value) noexcept
  {
    self.maximum_open_trades_ = value;
  }

  auto maximum_combined_layers(this const Portfolio& self) noexcept
   -> std::size_t
  {
    return self.maximum_combined_layers_;
  }

  void maximum_combined_layers(this Portfolio& self, std::size_t value) noexcept
  {
    self.maximum_combined_layers_ = value;
  }

  auto entry_comparators(this const Portfolio& self) noexcept
   -> const std::vector<PortfolioEntryComparator>&
  {
    return self.entry_comparators_;
  }

  auto entry_comparators(this Portfolio& self) noexcept
   -> std::vector<PortfolioEntryComparator>&
  {
    return self.entry_comparators_;
  }

  void entry_comparators(this Portfolio& self,
                         std::vector<PortfolioEntryComparator> value) noexcept
  {
    self.entry_comparators_ = std::move(value);
  }

  auto system_handles(this const Portfolio& self) noexcept
     -> const std::vector<SystemStoreHandle>&
  {
    return self.system_handles_;
  }

  void system_handles(this Portfolio& self,
                      std::vector<SystemStoreHandle> value)
  {
    if(has_duplicates(value)) {
      throw std::invalid_argument{
       "Portfolio cannot contain duplicate backtests"};
    }
    self.system_handles_ = std::move(value);
  }

  auto equivalent_rules(this const Portfolio& self,
                        const Portfolio& other) noexcept -> bool
  {
    return self.initial_capital_ == other.initial_capital_ &&
           self.market_handle_ == other.market_handle_ &&
           self.broker_handle_ == other.broker_handle_ &&
           self.maximum_open_trades_ == other.maximum_open_trades_ &&
           self.maximum_combined_layers_ == other.maximum_combined_layers_ &&
           self.entry_comparators_ == other.entry_comparators_ &&
            self.system_handles_ == other.system_handles_;
  }

private:
  static auto has_duplicates(const std::vector<SystemStoreHandle>& handles)
   -> bool
  {
    for(auto index = std::size_t{}; index < handles.size(); ++index) {
      if(std::find(handles.begin() + static_cast<std::ptrdiff_t>(index + 1),
                   handles.end(),
                   handles[index]) != handles.end()) {
        return true;
      }
    }
    return false;
  }

  void self_validate() const
  {
    if(!std::isfinite(initial_capital_) || initial_capital_ < 0.0) {
      throw std::invalid_argument{
       "Portfolio initial capital must be finite and non-negative"};
    }
    if(has_duplicates(system_handles_)) {
      throw std::invalid_argument{
       "Portfolio cannot contain duplicate backtests"};
    }
  }

  std::string name_;
  double initial_capital_;
  MarketStoreHandle market_handle_;
  BrokerStoreHandle broker_handle_;
  std::size_t maximum_open_trades_;
  std::size_t maximum_combined_layers_;
  std::vector<PortfolioEntryComparator> entry_comparators_;
  std::vector<SystemStoreHandle> system_handles_;
};

} // namespace pludux::backtest
