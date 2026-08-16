module;

#include <any>
#include <cmath>
#include <concepts>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <boost/math/distributions/beta.hpp>
#include <boost/math/distributions/inverse_gamma.hpp>

export module pludux.backtest:model_performance_model;

export namespace pludux::backtest {

class BetaBernoulliModelNode {
public:
  BetaBernoulliModelNode(double prior_probability = 0.5,
                         double prior_strength = 2.0) noexcept
  : prior_probability_{prior_probability}
  , prior_strength_{prior_strength}
  {
  }

  auto operator==(const BetaBernoulliModelNode&) const noexcept
   -> bool = default;

  auto prior_probability(this const BetaBernoulliModelNode& self) noexcept
   -> double
  {
    return self.prior_probability_;
  }

  auto prior_strength(this const BetaBernoulliModelNode& self) noexcept
   -> double
  {
    return self.prior_strength_;
  }

private:
  double prior_probability_{0.5};
  double prior_strength_{2.0};
};

class GammaPayoffModelNode {
public:
  GammaPayoffModelNode(double prior_mean_magnitude = 0.01,
                       double prior_strength = 0.01,
                       double coefficient_of_variation = 1.0) noexcept
  : prior_mean_magnitude_{prior_mean_magnitude}
  , prior_strength_{prior_strength}
  , coefficient_of_variation_{coefficient_of_variation}
  {
  }

  auto operator==(const GammaPayoffModelNode&) const noexcept -> bool = default;

  auto prior_mean_magnitude(this const GammaPayoffModelNode& self) noexcept
   -> double
  {
    return self.prior_mean_magnitude_;
  }

  auto prior_strength(this const GammaPayoffModelNode& self) noexcept -> double
  {
    return self.prior_strength_;
  }

  auto coefficient_of_variation(this const GammaPayoffModelNode& self) noexcept
   -> double
  {
    return self.coefficient_of_variation_;
  }

private:
  double prior_mean_magnitude_{0.01};
  double prior_strength_{0.01};
  double coefficient_of_variation_{1.0};
};

struct WeightedBinaryEvidence {
  double win_weight{};
  double loss_weight{};

  auto operator==(const WeightedBinaryEvidence&) const noexcept
   -> bool = default;
};

struct WeightedPayoffEvidence {
  double effective_count{};
  double sum{};

  auto operator==(const WeightedPayoffEvidence&) const noexcept
   -> bool = default;
};

struct BayesianWinSnapshot {
  double probability{};
  double lower_95{};
  double upper_95{};
  double posterior_alpha{};
  double posterior_beta{};

  auto operator==(const BayesianWinSnapshot&) const noexcept -> bool = default;
};

struct BayesianPayoffSnapshot {
  double mean{};
  double lower_95{};
  double upper_95{};
  double posterior_shape{};
  double posterior_scale{};
  double effective_count{};

  auto operator==(const BayesianPayoffSnapshot&) const noexcept
   -> bool = default;
};

class BetaBernoulliModelMethod {
public:
  explicit BetaBernoulliModelMethod(BetaBernoulliModelNode node)
  : node_{std::move(node)}
  {
    validate();
  }

  auto operator==(const BetaBernoulliModelMethod&) const noexcept
   -> bool = default;

  auto evaluate(this const BetaBernoulliModelMethod& self,
                const WeightedBinaryEvidence& evidence) -> BayesianWinSnapshot
  {
    const auto prior_alpha =
     self.node_.prior_probability() * self.node_.prior_strength();
    const auto prior_beta =
     (1.0 - self.node_.prior_probability()) * self.node_.prior_strength();
    const auto alpha = prior_alpha + evidence.win_weight;
    const auto beta = prior_beta + evidence.loss_weight;
    const auto distribution =
     boost::math::beta_distribution<double>{alpha, beta};

    return BayesianWinSnapshot{
     .probability = alpha / (alpha + beta),
     .lower_95 = boost::math::quantile(distribution, 0.025),
     .upper_95 = boost::math::quantile(distribution, 0.975),
     .posterior_alpha = alpha,
     .posterior_beta = beta};
  }

private:
  BetaBernoulliModelNode node_;

  void validate(this const BetaBernoulliModelMethod& self)
  {
    if(!std::isfinite(self.node_.prior_probability()) ||
       self.node_.prior_probability() <= 0.0 ||
       self.node_.prior_probability() >= 1.0 ||
       !std::isfinite(self.node_.prior_strength()) ||
       self.node_.prior_strength() <= 0.0) {
      throw std::invalid_argument{"Invalid Beta-Bernoulli prior"};
    }
  }
};

class GammaPayoffModelMethod {
public:
  explicit GammaPayoffModelMethod(GammaPayoffModelNode node)
  : node_{std::move(node)}
  {
    validate();
  }

  auto operator==(const GammaPayoffModelMethod&) const noexcept
   -> bool = default;

  auto evaluate(this const GammaPayoffModelMethod& self,
                const WeightedPayoffEvidence& evidence)
   -> BayesianPayoffSnapshot
  {
    const auto coefficient_of_variation = self.node_.coefficient_of_variation();
    const auto gamma_shape =
     1.0 / (coefficient_of_variation * coefficient_of_variation);
    const auto posterior_shape =
     1.0 +
     gamma_shape * (self.node_.prior_strength() + evidence.effective_count);
    const auto posterior_scale =
     gamma_shape *
     (self.node_.prior_strength() * self.node_.prior_mean_magnitude() +
      evidence.sum);
    const auto distribution = boost::math::inverse_gamma_distribution<double>{
     posterior_shape, posterior_scale};

    return BayesianPayoffSnapshot{
     .mean = posterior_scale / (posterior_shape - 1.0),
     .lower_95 = boost::math::quantile(distribution, 0.025),
     .upper_95 = boost::math::quantile(distribution, 0.975),
     .posterior_shape = posterior_shape,
     .posterior_scale = posterior_scale,
     .effective_count = evidence.effective_count};
  }

private:
  GammaPayoffModelNode node_;

  void validate(this const GammaPayoffModelMethod& self)
  {
    if(!std::isfinite(self.node_.prior_mean_magnitude()) ||
       self.node_.prior_mean_magnitude() <= 0.0 ||
       !std::isfinite(self.node_.prior_strength()) ||
       self.node_.prior_strength() <= 0.0 ||
       !std::isfinite(self.node_.coefficient_of_variation()) ||
       self.node_.coefficient_of_variation() <= 0.0) {
      throw std::invalid_argument{"Invalid Gamma payoff prior"};
    }
  }
};

auto node_to_model_method(const BetaBernoulliModelNode& node)
 -> BetaBernoulliModelMethod
{
  return BetaBernoulliModelMethod{node};
}

auto node_to_model_method(const GammaPayoffModelNode& node)
 -> GammaPayoffModelMethod
{
  return GammaPayoffModelMethod{node};
}

template<typename TSnapshot, typename TEvidence>
class ErasedBayesianModelMethod {
public:
  template<typename TMethod>
    requires(!std::same_as<std::remove_cvref_t<TMethod>,
                           ErasedBayesianModelMethod>) &&
             std::copy_constructible<std::remove_cvref_t<TMethod>>
  explicit ErasedBayesianModelMethod(TMethod method)
  : impl_{std::make_any<std::remove_cvref_t<TMethod>>(std::move(method))}
  , evaluate_{[](const std::any& impl, const TEvidence& evidence) -> TSnapshot {
    return std::any_cast<const std::remove_cvref_t<TMethod>&>(impl).evaluate(
     evidence);
  }}
  {
  }

  auto evaluate(this const ErasedBayesianModelMethod& self,
                const TEvidence& evidence) -> TSnapshot
  {
    return self.evaluate_(self.impl_, evidence);
  }

private:
  std::any impl_;
  std::function<auto(const std::any&, const TEvidence&)->TSnapshot> evaluate_;
};

template<typename TSnapshot, typename TEvidence>
class ErasedBayesianModelNode {
public:
  template<typename TNode>
    requires(!std::same_as<std::remove_cvref_t<TNode>,
                           ErasedBayesianModelNode>) &&
             std::equality_comparable<std::remove_cvref_t<TNode>> &&
             std::copy_constructible<std::remove_cvref_t<TNode>>
  explicit ErasedBayesianModelNode(TNode node)
  : impl_{std::make_any<std::remove_cvref_t<TNode>>(std::move(node))}
  , convert_{[](const std::any& impl)
              -> ErasedBayesianModelMethod<TSnapshot, TEvidence> {
    const auto& node = std::any_cast<const std::remove_cvref_t<TNode>&>(impl);
    return ErasedBayesianModelMethod<TSnapshot, TEvidence>{
     node_to_model_method(node)};
  }}
  , equals_{
     [](const std::any& impl, const ErasedBayesianModelNode& other) -> bool {
       const auto* other_node =
        std::any_cast<std::remove_cvref_t<TNode>>(&other.impl_);
       return other_node != nullptr &&
              std::any_cast<const std::remove_cvref_t<TNode>&>(impl) ==
               *other_node;
     }}
  {
  }

  auto operator==(this const ErasedBayesianModelNode& self,
                  const ErasedBayesianModelNode& other) noexcept -> bool
  {
    return self.equals_(self.impl_, other);
  }

  friend auto node_to_model_method(const ErasedBayesianModelNode& node)
   -> ErasedBayesianModelMethod<TSnapshot, TEvidence>
  {
    return node.convert_(node.impl_);
  }

  template<typename TNode>
  friend auto
  bayesian_model_node_cast(const ErasedBayesianModelNode& node) noexcept
   -> const TNode*
  {
    return std::any_cast<const TNode>(&node.impl_);
  }

  template<typename TNode>
  friend auto bayesian_model_node_cast(ErasedBayesianModelNode& node) noexcept
   -> TNode*
  {
    return std::any_cast<TNode>(&node.impl_);
  }

private:
  std::any impl_;
  std::function<
   auto(const std::any&)->ErasedBayesianModelMethod<TSnapshot, TEvidence>>
   convert_;
  std::function<auto(const std::any&, const ErasedBayesianModelNode&)->bool>
   equals_;
};

using BayesianWinModelMethod =
 ErasedBayesianModelMethod<BayesianWinSnapshot, WeightedBinaryEvidence>;
using BayesianPayoffModelMethod =
 ErasedBayesianModelMethod<BayesianPayoffSnapshot, WeightedPayoffEvidence>;
using BayesianWinModelNode =
 ErasedBayesianModelNode<BayesianWinSnapshot, WeightedBinaryEvidence>;
using BayesianPayoffModelNode =
 ErasedBayesianModelNode<BayesianPayoffSnapshot, WeightedPayoffEvidence>;

} // namespace pludux::backtest
