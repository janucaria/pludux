module;

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

export module pludux.backtest:backtest_method_context;

import pludux;

export namespace pludux::backtest {

class RequestedOrderPriceNode;
class RequestedOrderDirectionNode;
class IsPyramidingOrderNode;
class RawRequestedQuantityNode;
class RawRequestedQuantityLimitNode;
class DrawdownAdjustedQuantityNode;
class DrawdownAdjustedQuantityLimitNode;
class RequestedQuantityNode;
class RequestedNotionalNode;
class RequestedCostNode;
class EstimatedEntryFeeNode;
class EstimatedOneRExitFeeNode;
class RequestedOrderRiskDistanceNode;
class RequestedPriceRiskNode;
class RequestedRiskWithFeesNode;
class FrozenUnitQuantityNode;

class BacktestAccountState {
public:
  BacktestAccountState() = default;

  BacktestAccountState(double capital,
                       double unrealized_pnl,
                       double peak_equity,
                       double initial_capital) noexcept
  : capital_{capital}
  , unrealized_pnl_{unrealized_pnl}
  , peak_equity_{peak_equity}
  , initial_capital_{initial_capital}
  {
  }

  auto capital(this const BacktestAccountState& self) noexcept -> double
  {
    return self.capital_;
  }

  auto unrealized_pnl(this const BacktestAccountState& self) noexcept -> double
  {
    return self.unrealized_pnl_;
  }

  auto equity(this const BacktestAccountState& self) noexcept -> double
  {
    return self.capital_ + self.unrealized_pnl_;
  }

  auto peak_equity(this const BacktestAccountState& self) noexcept -> double
  {
    return self.peak_equity_;
  }

  auto effective_peak_equity(this const BacktestAccountState& self) noexcept
   -> double
  {
    return std::max(self.peak_equity_, self.equity());
  }

  auto initial_capital(this const BacktestAccountState& self) noexcept -> double
  {
    return self.initial_capital_;
  }

  void capital(this BacktestAccountState& self, double capital) noexcept
  {
    self.capital_ = capital;
  }

  void unrealized_pnl(this BacktestAccountState& self,
                      double unrealized_pnl) noexcept
  {
    self.unrealized_pnl_ = unrealized_pnl;
  }

  void peak_equity(this BacktestAccountState& self, double peak_equity) noexcept
  {
    self.peak_equity_ = peak_equity;
  }

  void update_peak_to_current_equity(this BacktestAccountState& self) noexcept
  {
    self.peak_equity(self.effective_peak_equity());
  }

  auto equity_percent(this const BacktestAccountState& self) noexcept -> double
  {
    if(self.initial_capital_ == 0.0) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    return self.equity() / self.initial_capital_ * 100.0;
  }

  auto drawdown(this const BacktestAccountState& self) noexcept -> double
  {
    const auto peak_equity = self.effective_peak_equity();
    return peak_equity ? (peak_equity - self.equity()) / peak_equity * 100.0
                       : 0.0;
  }

  auto drawdown_ratio(this const BacktestAccountState& self) noexcept -> double
  {
    return self.drawdown() / 100.0;
  }

  auto operator==(const BacktestAccountState& other) const noexcept
   -> bool = default;

private:
  double capital_{std::numeric_limits<double>::quiet_NaN()};
  double unrealized_pnl_{std::numeric_limits<double>::quiet_NaN()};
  double peak_equity_{std::numeric_limits<double>::quiet_NaN()};
  double initial_capital_{std::numeric_limits<double>::quiet_NaN()};
};

class BacktestMethodContext;
using ModelNode = ErasedNode<BacktestMethodContext>;
using ModelMethod = ErasedSeriesMethod<BacktestMethodContext>;
using ModelMethodRegistry = OrderedNamedRegistry<ModelMethod>;

class BacktestMethodContext {
public:
  template<typename TNode>
  static consteval auto node_admissible() -> bool
  {
    using Node = std::remove_cvref_t<TNode>;
    return !(std::same_as<Node, RequestedOrderPriceNode> ||
             std::same_as<Node, RequestedOrderDirectionNode> ||
             std::same_as<Node, IsPyramidingOrderNode> ||
             std::same_as<Node, RawRequestedQuantityNode> ||
             std::same_as<Node, RawRequestedQuantityLimitNode> ||
             std::same_as<Node, DrawdownAdjustedQuantityNode> ||
             std::same_as<Node, DrawdownAdjustedQuantityLimitNode> ||
             std::same_as<Node, RequestedQuantityNode> ||
             std::same_as<Node, RequestedNotionalNode> ||
             std::same_as<Node, RequestedCostNode> ||
             std::same_as<Node, EstimatedEntryFeeNode> ||
             std::same_as<Node, EstimatedOneRExitFeeNode> ||
             std::same_as<Node, RequestedOrderRiskDistanceNode> ||
             std::same_as<Node, RequestedPriceRiskNode> ||
             std::same_as<Node, RequestedRiskWithFeesNode> ||
             std::same_as<Node, FrozenUnitQuantityNode>);
  }

  BacktestMethodContext(const ModelMethodRegistry& series_methods,
   SeriesEvaluationResults& series_evaluation_results,
   std::size_t current_index,
   const BacktestAccountState& account_state,
   std::size_t pyramiding_layer) noexcept
  : series_methods_{series_methods}
  , series_evaluation_results_{series_evaluation_results}
  , current_index_{current_index}
  , account_state_{account_state}
  , pyramiding_layer_{pyramiding_layer}
  {
  }

  auto call_series_method(this const BacktestMethodContext& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot) noexcept -> double
  {
    if(const auto method_opt = self.series_methods_.get(name);
       method_opt.has_value()) {
      return evaluate_series_method(method_opt.value(), asset_snapshot, self);
    }
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto call_series_method(this const BacktestMethodContext& self,
                          const std::string& name,
                          AssetSnapshot asset_snapshot,
                          MethodOutput output) noexcept -> double
  {
    if(const auto method_opt = self.series_methods_.get(name);
       method_opt.has_value()) {
      return evaluate_series_method(
       output, method_opt.value(), asset_snapshot, self);
    }
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto get_series_result(this const BacktestMethodContext& self,
                         const std::string& name,
                         std::size_t result_index) noexcept -> double
  {
    const auto& method_opt = self.series_methods_.get(name);
    if(!method_opt.has_value()) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    if(const auto results_opt = self.series_evaluation_results_.results(name);
       results_opt.has_value()) {
      const auto& results = results_opt.value().get();
      if(result_index < results.size()) {
        return results[result_index];
      }
    }
    if(const auto results_opt =
        self.series_evaluation_results_.results(method_opt.value());
       results_opt.has_value()) {
      const auto& results = results_opt.value().get();
      if(result_index < results.size()) {
        return results[result_index];
      }
    }
    return std::numeric_limits<double>::quiet_NaN();
  }

  auto get_series_results(this BacktestMethodContext& self,
                          const auto& method_key) noexcept
   -> std::vector<double>&
  {
    const auto results_opt =
     self.series_evaluation_results_.results(method_key);
    if(!results_opt.has_value()) {
      self.series_evaluation_results_.results(method_key, {});
    }
    return self.series_evaluation_results_.results(method_key).value();
  }

  auto index(this const BacktestMethodContext& self) noexcept -> std::size_t
  {
    return self.current_index_;
  }

  auto equity(this const BacktestMethodContext& self) noexcept -> double
  {
    return self.account_state_.equity();
  }

  auto equity_percent(this const BacktestMethodContext& self) noexcept -> double
  {
    return self.account_state_.equity_percent();
  }

  auto drawdown(this const BacktestMethodContext& self) noexcept -> double
  {
    return self.account_state_.drawdown();
  }

  auto pyramiding_layer(this const BacktestMethodContext& self) noexcept
   -> std::size_t
  {
    return self.pyramiding_layer_;
  }

  auto with_position_reference(this const BacktestMethodContext& self,
                               double reference_price,
                               double direction) noexcept
   -> BacktestMethodContext
  {
    return self.with_position_prices(reference_price,
                                     reference_price,
                                     reference_price,
                                     reference_price,
                                     direction);
  }

  auto with_position_prices(this const BacktestMethodContext& self,
                            double initial_entry_price,
                            double latest_entry_price,
                            double average_price,
                            double reference_price,
                            double direction) noexcept -> BacktestMethodContext
  {
    auto context = self;
    context.position_initial_entry_price_ = initial_entry_price;
    context.position_latest_entry_price_ = latest_entry_price;
    context.position_average_price_ = average_price;
    context.position_reference_price_ = reference_price;
    context.position_direction_ = direction;
    context.position_risk_distance_ = std::numeric_limits<double>::quiet_NaN();
    return context;
  }

  auto with_position_risk_distance(this const BacktestMethodContext& self,
                                   double risk_distance) noexcept
   -> BacktestMethodContext
  {
    auto context = self;
    context.position_risk_distance_ = risk_distance;
    return context;
  }

  auto position_reference_price(this const BacktestMethodContext& self) noexcept
   -> double
  {
    return self.position_reference_price_;
  }

  auto
  position_initial_entry_price(this const BacktestMethodContext& self) noexcept
   -> double
  {
    return self.position_initial_entry_price_;
  }

  auto
  position_latest_entry_price(this const BacktestMethodContext& self) noexcept
   -> double
  {
    return self.position_latest_entry_price_;
  }

  auto position_average_price(this const BacktestMethodContext& self) noexcept
   -> double
  {
    return self.position_average_price_;
  }

  auto position_direction(this const BacktestMethodContext& self) noexcept
   -> double
  {
    return self.position_direction_;
  }

  auto position_risk_distance(this const BacktestMethodContext& self) noexcept
   -> double
  {
    return self.position_risk_distance_;
  }

private:
  const ModelMethodRegistry& series_methods_;
  SeriesEvaluationResults& series_evaluation_results_;
  std::size_t current_index_;
  const BacktestAccountState& account_state_;
  std::size_t pyramiding_layer_;
  double position_initial_entry_price_{
   std::numeric_limits<double>::quiet_NaN()};
  double position_latest_entry_price_{std::numeric_limits<double>::quiet_NaN()};
  double position_average_price_{std::numeric_limits<double>::quiet_NaN()};
  double position_reference_price_{std::numeric_limits<double>::quiet_NaN()};
  double position_direction_{std::numeric_limits<double>::quiet_NaN()};
  double position_risk_distance_{std::numeric_limits<double>::quiet_NaN()};
};

} // namespace pludux::backtest
