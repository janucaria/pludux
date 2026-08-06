module;

#include <algorithm>
#include <any>
#include <cmath>
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <boost/math/distributions/beta.hpp>
#include <boost/math/distributions/inverse_gamma.hpp>

export module pludux.backtest:position_sizing;

import :strategy_intent;
import :strategy_performance;
import :entry_order_sizing;

export namespace pludux::backtest {

enum class StrategyPerformanceBayesianKellyEstimate {
  PosteriorMean,
  AdverseQuantiles
};

struct StrategyPerformanceBayesianKellyEvaluation {
  StrategyPerformanceBayesianKellyEstimate estimate{
   StrategyPerformanceBayesianKellyEstimate::AdverseQuantiles};
  double central_credible_mass{0.80};
  double win_probability{};
  double winning_payoff{};
  double losing_payoff{};
  double raw_kelly_fraction{};
  double kelly_multiplier{0.50};
  double scaled_kelly_fraction{};
  double maximum_equity_fraction{1.0};
  double allocation_fraction{};

  auto
  operator==(const StrategyPerformanceBayesianKellyEvaluation&) const noexcept
   -> bool = default;
};

struct PositionSizingEvaluation {
  double requested_quantity{};
  EntryOrderSizingConstraint constraint{NearestQuantityConstraint{}};
  std::optional<StrategyPerformanceBayesianKellyEvaluation> bayesian_kelly{};

  auto operator==(const PositionSizingEvaluation&) const noexcept
   -> bool = default;
};

class PositionSizingContext {
public:
  PositionSizingContext(double equity,
                        double entry_price,
                        double direction,
                        const StrategyPerformanceSnapshot& strategy_performance,
                        std::function<double()> risk_distance_resolver)
  : equity_{equity}
  , entry_price_{entry_price}
  , direction_{direction}
  , strategy_performance_{&strategy_performance}
  , risk_distance_resolver_{std::move(risk_distance_resolver)}
  {
    if(direction_ != 1.0 && direction_ != -1.0) {
      throw std::invalid_argument{"Invalid position direction"};
    }
  }

  auto equity(this const PositionSizingContext& self) noexcept -> double
  {
    return self.equity_;
  }

  auto entry_price(this const PositionSizingContext& self) noexcept -> double
  {
    return self.entry_price_;
  }

  auto direction(this const PositionSizingContext& self) noexcept -> double
  {
    return self.direction_;
  }

  auto strategy_performance(this const PositionSizingContext& self) noexcept
   -> const StrategyPerformanceSnapshot&
  {
    return *self.strategy_performance_;
  }

  auto risk_distance(this const PositionSizingContext& self) -> double
  {
    if(!self.risk_distance_resolver_) {
      throw std::runtime_error{"Risk distance is unavailable"};
    }
    return self.risk_distance_resolver_();
  }

private:
  double equity_{};
  double entry_price_{};
  double direction_{};
  const StrategyPerformanceSnapshot* strategy_performance_{};
  std::function<double()> risk_distance_resolver_{};
};

template<typename T>
concept PositionSizingMethodImplementation =
 std::equality_comparable<T> &&
 requires(const T& method, const PositionSizingContext& context) {
   { method.evaluate(context) } -> std::same_as<PositionSizingEvaluation>;
   { method.name() } -> std::convertible_to<std::string_view>;
 };

class PositionSizingMethod {
public:
  template<PositionSizingMethodImplementation TMethod>
    requires(!std::same_as<std::remove_cvref_t<TMethod>, PositionSizingMethod>)
  PositionSizingMethod(TMethod method)
  : impl_{std::make_any<TMethod>(std::move(method))}
  , evaluate_{[](const std::any& impl, const PositionSizingContext& context) {
    return std::any_cast<const TMethod&>(impl).evaluate(context);
  }}
  , name_{[](const std::any& impl) {
    return std::string_view{std::any_cast<const TMethod&>(impl).name()};
  }}
  , equals_{[](const std::any& impl, const PositionSizingMethod& other) {
    const auto* other_method = std::any_cast<TMethod>(&other.impl_);
    return other_method && std::any_cast<const TMethod&>(impl) == *other_method;
  }}
  {
  }

  auto operator==(this const PositionSizingMethod& self,
                  const PositionSizingMethod& other) noexcept -> bool
  {
    return self.equals_(self.impl_, other);
  }

  auto evaluate(this const PositionSizingMethod& self,
                const PositionSizingContext& context)
   -> PositionSizingEvaluation
  {
    return self.evaluate_(self.impl_, context);
  }

  auto name(this const PositionSizingMethod& self) -> std::string_view
  {
    return self.name_(self.impl_);
  }

private:
  std::any impl_;
  std::function<PositionSizingEvaluation(const std::any&,
                                         const PositionSizingContext&)>
   evaluate_;
  std::function<std::string_view(const std::any&)> name_;
  std::function<bool(const std::any&, const PositionSizingMethod&)> equals_;
};

class RiskDistancePositionSizing {
public:
  explicit RiskDistancePositionSizing(double risk_fraction = 0.01)
  : risk_fraction_{risk_fraction}
  {
    validate_positive(risk_fraction_, "risk fraction");
  }

  auto operator==(const RiskDistancePositionSizing&) const noexcept
   -> bool = default;

  auto risk_fraction(this const RiskDistancePositionSizing& self) noexcept
   -> double
  {
    return self.risk_fraction_;
  }

private:
  double risk_fraction_{0.01};

  static void validate_positive(double value, const char* label)
  {
    if(!std::isfinite(value) || value <= 0.0) {
      throw std::invalid_argument{std::string{"Invalid "} + label};
    }
  }
};

class FixedQuantityPositionSizing {
public:
  explicit FixedQuantityPositionSizing(double quantity = 1.0)
  : quantity_{quantity}
  {
    if(!std::isfinite(quantity_) || quantity_ <= 0.0) {
      throw std::invalid_argument{"Invalid fixed quantity"};
    }
  }

  auto operator==(const FixedQuantityPositionSizing&) const noexcept
   -> bool = default;

  auto quantity(this const FixedQuantityPositionSizing& self) noexcept -> double
  {
    return self.quantity_;
  }

private:
  double quantity_{1.0};
};

class FixedBudgetPositionSizing {
public:
  explicit FixedBudgetPositionSizing(double budget = 1000.0)
  : budget_{budget}
  {
    if(!std::isfinite(budget_) || budget_ <= 0.0) {
      throw std::invalid_argument{"Invalid fixed budget"};
    }
  }

  auto operator==(const FixedBudgetPositionSizing&) const noexcept
   -> bool = default;

  auto budget(this const FixedBudgetPositionSizing& self) noexcept -> double
  {
    return self.budget_;
  }

private:
  double budget_{1000.0};
};

class EquityFractionPositionSizing {
public:
  explicit EquityFractionPositionSizing(double equity_fraction = 0.01)
  : equity_fraction_{equity_fraction}
  {
    if(!std::isfinite(equity_fraction_) || equity_fraction_ <= 0.0) {
      throw std::invalid_argument{"Invalid equity fraction"};
    }
  }

  auto operator==(const EquityFractionPositionSizing&) const noexcept
   -> bool = default;

  auto equity_fraction(this const EquityFractionPositionSizing& self) noexcept
   -> double
  {
    return self.equity_fraction_;
  }

private:
  double equity_fraction_{0.01};
};

class StrategyPerformanceBayesianKellySizing {
public:
  StrategyPerformanceBayesianKellySizing(
   StrategyPerformanceBayesianKellyEstimate estimate =
    StrategyPerformanceBayesianKellyEstimate::AdverseQuantiles,
   double central_credible_mass = 0.80,
   double kelly_multiplier = 0.50,
   double maximum_equity_fraction = 1.0)
  : estimate_{estimate}
  , central_credible_mass_{central_credible_mass}
  , kelly_multiplier_{kelly_multiplier}
  , maximum_equity_fraction_{maximum_equity_fraction}
  {
    validate();
  }

  auto operator==(const StrategyPerformanceBayesianKellySizing&) const noexcept
   -> bool = default;

  auto
  estimate(this const StrategyPerformanceBayesianKellySizing& self) noexcept
   -> StrategyPerformanceBayesianKellyEstimate
  {
    return self.estimate_;
  }

  auto central_credible_mass(
   this const StrategyPerformanceBayesianKellySizing& self) noexcept -> double
  {
    return self.central_credible_mass_;
  }

  auto kelly_multiplier(
   this const StrategyPerformanceBayesianKellySizing& self) noexcept -> double
  {
    return self.kelly_multiplier_;
  }

  auto maximum_equity_fraction(
   this const StrategyPerformanceBayesianKellySizing& self) noexcept -> double
  {
    return self.maximum_equity_fraction_;
  }

private:
  StrategyPerformanceBayesianKellyEstimate estimate_{
   StrategyPerformanceBayesianKellyEstimate::AdverseQuantiles};
  double central_credible_mass_{0.80};
  double kelly_multiplier_{0.50};
  double maximum_equity_fraction_{1.0};

  void validate(this const StrategyPerformanceBayesianKellySizing& self)
  {
    if(!std::isfinite(self.central_credible_mass_) ||
       self.central_credible_mass_ <= 0.0 ||
       self.central_credible_mass_ >= 1.0) {
      throw std::invalid_argument{
       "Kelly central credible mass must be in (0, 1)"};
    }
    if(!std::isfinite(self.kelly_multiplier_) || self.kelly_multiplier_ < 0.0 ||
       self.kelly_multiplier_ > 1.0) {
      throw std::invalid_argument{"Kelly multiplier must be in [0, 1]"};
    }
    if(!std::isfinite(self.maximum_equity_fraction_) ||
       self.maximum_equity_fraction_ <= 0.0) {
      throw std::invalid_argument{
       "Kelly maximum equity fraction must be positive"};
    }
  }
};

namespace detail {

inline void validate_context_value(double value, std::string_view label)
{
  if(!std::isfinite(value) || value <= 0.0) {
    throw std::runtime_error{std::string{"Invalid "} + std::string{label}};
  }
}

class RiskDistancePositionSizingMethod {
public:
  explicit RiskDistancePositionSizingMethod(RiskDistancePositionSizing node)
  : node_{std::move(node)}
  {
  }

  auto operator==(const RiskDistancePositionSizingMethod&) const noexcept
   -> bool = default;
  auto name() const noexcept -> std::string_view
  {
    return "RISK_DISTANCE";
  }
  auto evaluate(const PositionSizingContext& context) const
   -> PositionSizingEvaluation
  {
    validate_context_value(context.equity(), "equity");
    const auto risk_distance = context.risk_distance();
    validate_context_value(risk_distance, "risk distance");
    const auto risk_budget = context.equity() * node_.risk_fraction();
    const auto boundary_price =
     context.entry_price() - context.direction() * risk_distance;
    validate_context_value(boundary_price, "risk boundary price");
    return {.requested_quantity = risk_budget / risk_distance,
            .constraint = RiskBudgetConstraint{risk_budget, boundary_price}};
  }

private:
  RiskDistancePositionSizing node_;
};

class FixedQuantityPositionSizingMethod {
public:
  explicit FixedQuantityPositionSizingMethod(FixedQuantityPositionSizing node)
  : node_{std::move(node)}
  {
  }

  auto operator==(const FixedQuantityPositionSizingMethod&) const noexcept
   -> bool = default;
  auto name() const noexcept -> std::string_view
  {
    return "FIXED_QUANTITY";
  }
  auto evaluate(const PositionSizingContext&) const -> PositionSizingEvaluation
  {
    return {.requested_quantity = node_.quantity(),
            .constraint = NearestQuantityConstraint{}};
  }

private:
  FixedQuantityPositionSizing node_;
};

class FixedBudgetPositionSizingMethod {
public:
  explicit FixedBudgetPositionSizingMethod(FixedBudgetPositionSizing node)
  : node_{std::move(node)}
  {
  }

  auto operator==(const FixedBudgetPositionSizingMethod&) const noexcept
   -> bool = default;
  auto name() const noexcept -> std::string_view
  {
    return "FIXED_BUDGET";
  }
  auto evaluate(const PositionSizingContext& context) const
   -> PositionSizingEvaluation
  {
    validate_context_value(context.entry_price(), "entry price");
    return {.requested_quantity = node_.budget() / context.entry_price(),
            .constraint = EntryCostBudgetConstraint{node_.budget()}};
  }

private:
  FixedBudgetPositionSizing node_;
};

class EquityFractionPositionSizingMethod {
public:
  explicit EquityFractionPositionSizingMethod(EquityFractionPositionSizing node)
  : node_{std::move(node)}
  {
  }

  auto operator==(const EquityFractionPositionSizingMethod&) const noexcept
   -> bool = default;
  auto name() const noexcept -> std::string_view
  {
    return "EQUITY_FRACTION";
  }
  auto evaluate(const PositionSizingContext& context) const
   -> PositionSizingEvaluation
  {
    validate_context_value(context.equity(), "equity");
    validate_context_value(context.entry_price(), "entry price");
    const auto budget = context.equity() * node_.equity_fraction();
    return {.requested_quantity = budget / context.entry_price(),
            .constraint = EntryCostBudgetConstraint{budget}};
  }

private:
  EquityFractionPositionSizing node_;
};

class StrategyPerformanceBayesianKellySizingMethod {
public:
  explicit StrategyPerformanceBayesianKellySizingMethod(
   StrategyPerformanceBayesianKellySizing node)
  : node_{std::move(node)}
  {
  }

  auto
  operator==(const StrategyPerformanceBayesianKellySizingMethod&) const noexcept
   -> bool = default;
  auto name() const noexcept -> std::string_view
  {
    return "STRATEGY_PERFORMANCE_BAYESIAN_KELLY";
  }

  auto evaluate(const PositionSizingContext& context) const
   -> PositionSizingEvaluation
  {
    validate_context_value(context.equity(), "equity");
    validate_context_value(context.entry_price(), "entry price");

    const auto& performance = context.strategy_performance();
    const auto& win = performance.win_probability_posterior();
    const auto& winning = performance.winning_payoff_posterior();
    const auto& losing = performance.losing_payoff_posterior();

    auto probability = win.probability;
    auto winning_payoff = winning.mean;
    auto losing_payoff = losing.mean;
    if(node_.estimate() ==
       StrategyPerformanceBayesianKellyEstimate::AdverseQuantiles) {
      const auto tail = (1.0 - node_.central_credible_mass()) / 2.0;
      probability = boost::math::quantile(
       boost::math::beta_distribution<double>{win.posterior_alpha,
                                              win.posterior_beta},
       tail);
      winning_payoff = boost::math::quantile(
       boost::math::inverse_gamma_distribution<double>{winning.posterior_shape,
                                                       winning.posterior_scale},
       tail);
      losing_payoff = boost::math::quantile(
       boost::math::inverse_gamma_distribution<double>{losing.posterior_shape,
                                                       losing.posterior_scale},
       1.0 - tail);
    }

    validate_context_value(winning_payoff, "Bayesian winning payoff");
    validate_context_value(losing_payoff, "Bayesian losing payoff");
    if(!std::isfinite(probability) || probability <= 0.0 ||
       probability >= 1.0) {
      throw std::runtime_error{"Invalid Bayesian win probability"};
    }

    const auto raw_fraction =
     probability / losing_payoff - (1.0 - probability) / winning_payoff;
    if(!std::isfinite(raw_fraction)) {
      throw std::runtime_error{"Invalid Bayesian Kelly fraction"};
    }
    const auto scaled_fraction =
     node_.kelly_multiplier() * std::max(raw_fraction, 0.0);
    const auto allocation_fraction =
     std::min(scaled_fraction, node_.maximum_equity_fraction());

    const auto diagnostic = StrategyPerformanceBayesianKellyEvaluation{
     .estimate = node_.estimate(),
     .central_credible_mass = node_.central_credible_mass(),
     .win_probability = probability,
     .winning_payoff = winning_payoff,
     .losing_payoff = losing_payoff,
     .raw_kelly_fraction = raw_fraction,
     .kelly_multiplier = node_.kelly_multiplier(),
     .scaled_kelly_fraction = scaled_fraction,
     .maximum_equity_fraction = node_.maximum_equity_fraction(),
     .allocation_fraction = allocation_fraction};
    const auto budget = allocation_fraction * context.equity();
    return {.requested_quantity = budget / context.entry_price(),
            .constraint = EntryCostBudgetConstraint{budget},
            .bayesian_kelly = diagnostic};
  }

private:
  StrategyPerformanceBayesianKellySizing node_;
};

} // namespace detail

inline auto make_position_sizing_method(const RiskDistancePositionSizing& node)
 -> PositionSizingMethod
{
  return PositionSizingMethod{detail::RiskDistancePositionSizingMethod{node}};
}

inline auto make_position_sizing_method(const FixedQuantityPositionSizing& node)
 -> PositionSizingMethod
{
  return PositionSizingMethod{detail::FixedQuantityPositionSizingMethod{node}};
}

inline auto make_position_sizing_method(const FixedBudgetPositionSizing& node)
 -> PositionSizingMethod
{
  return PositionSizingMethod{detail::FixedBudgetPositionSizingMethod{node}};
}

inline auto
make_position_sizing_method(const EquityFractionPositionSizing& node)
 -> PositionSizingMethod
{
  return PositionSizingMethod{detail::EquityFractionPositionSizingMethod{node}};
}

inline auto
make_position_sizing_method(const StrategyPerformanceBayesianKellySizing& node)
 -> PositionSizingMethod
{
  return PositionSizingMethod{
   detail::StrategyPerformanceBayesianKellySizingMethod{node}};
}

template<typename T>
concept PositionSizingNodeImplementation =
 std::equality_comparable<T> && requires(const T& node) {
   { make_position_sizing_method(node) } -> std::same_as<PositionSizingMethod>;
 };

class PositionSizingNode {
public:
  PositionSizingNode()
  : PositionSizingNode{RiskDistancePositionSizing{}}
  {
  }

  template<PositionSizingNodeImplementation TNode>
    requires(!std::same_as<std::remove_cvref_t<TNode>, PositionSizingNode>)
  PositionSizingNode(TNode node)
  : impl_{std::make_any<TNode>(std::move(node))}
  , make_method_{[](const std::any& impl) {
    return make_position_sizing_method(std::any_cast<const TNode&>(impl));
  }}
  , equals_{[](const std::any& impl, const PositionSizingNode& other) {
    const auto* other_node = std::any_cast<TNode>(&other.impl_);
    return other_node && std::any_cast<const TNode&>(impl) == *other_node;
  }}
  {
  }

  auto operator==(this const PositionSizingNode& self,
                  const PositionSizingNode& other) noexcept -> bool
  {
    return self.equals_(self.impl_, other);
  }

  auto make_method(this const PositionSizingNode& self) -> PositionSizingMethod
  {
    return self.make_method_(self.impl_);
  }

  template<typename TNode>
  friend auto position_sizing_node_cast(const PositionSizingNode& node) noexcept
   -> const TNode*
  {
    return std::any_cast<TNode>(&node.impl_);
  }

private:
  std::any impl_;
  std::function<PositionSizingMethod(const std::any&)> make_method_;
  std::function<bool(const std::any&, const PositionSizingNode&)> equals_;
};

enum class PositionSizingDecisionOutcome {
  Filtered,
  ShadowOnly,
  NoPositiveSize,
  DrawdownSuppressed,
  SizingLimitTooSmall,
  MaximumOpenTrades,
  MaximumCombinedLayers,
  InsufficientCash,
  Executed
};

struct PositionSizingDecision {
  std::size_t intent_id{};
  std::size_t strategy_trade_id{};
  StrategyDirection direction{StrategyDirection::Long};
  bool pyramiding{};
  std::string method{};
  double entry_price{};
  PositionSizingDecisionOutcome outcome{
   PositionSizingDecisionOutcome::Filtered};
  std::optional<double> requested_quantity{};
  std::optional<double> requested_limit{};
  std::optional<double> drawdown_adjusted_quantity{};
  std::optional<double> drawdown_adjusted_limit{};
  std::optional<double> sizing_normalized_quantity{};
  std::optional<double> entry_cost{};
  std::optional<double> estimated_loss{};
  std::optional<double> cash_required{};
  std::optional<double> cash_available{};
  std::optional<double> final_quantity{};
  std::optional<double> final_entry_cost{};
  bool cash_adjusted{};
  std::optional<StrategyPerformanceBayesianKellyEvaluation> bayesian_kelly{};

  auto operator==(const PositionSizingDecision&) const noexcept
   -> bool = default;
};

} // namespace pludux::backtest
