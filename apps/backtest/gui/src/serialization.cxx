module;

#include <cstddef>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <jsoncons/json.hpp>
#include <jsoncons/reflect/json_conv_traits.hpp>

export module pludux.apps.backtest:serialization;

import pludux.backtest;

import :ui_state;
import :application_state;

namespace {

template<typename T>
auto conversion_failed() -> jsoncons::conversion_result<T>
{
  return jsoncons::conversion_result<T>{jsoncons::unexpect,
                                        jsoncons::conv_errc::conversion_failed};
}

template<typename T, typename Json>
auto required_as(const Json& json, const std::string& key) -> T
{
  return json.at(key).template as<T>();
}

template<typename Json, typename TValue>
void set_json(Json& json,
              const auto& aset,
              const std::string& key,
              const TValue& value)
{
  json[key] =
   jsoncons::reflect::json_conv_traits<Json, TValue>::to_json(aset, value);
}

template<typename Json, typename TValue>
auto vector_to_json(const auto& aset, const std::vector<TValue>& values) -> Json
{
  auto json = typename Json::array();
  for(const auto& value : values) {
    json.push_back(
     jsoncons::reflect::json_conv_traits<Json, TValue>::to_json(aset, value));
  }
  return json;
}

template<typename TValue, typename Json>
auto vector_from_json(const Json& json) -> std::vector<TValue>
{
  auto values = std::vector<TValue>{};
  values.reserve(json.size());
  for(const auto& item : json.array_range()) {
    values.push_back(item.template as<TValue>());
  }
  return values;
}

template<typename Json>
auto strategy_entry_to_json(const auto& aset,
                            const pludux::backtest::Strategy& strategy) -> Json
{
  auto json = Json{};
  json["name"] = strategy.name();
  json["config"] = jsoncons::reflect::
   json_conv_traits<Json, pludux::backtest::Strategy>::to_json(aset, strategy);
  return json;
}

template<typename TStrategy, typename Json>
auto strategy_entry_from_json(const Json& json) -> TStrategy
{
  auto strategy = json.at("config").template as<TStrategy>();
  strategy.name(required_as<std::string>(json, "name"));
  return strategy;
}

template<typename Json>
auto strategies_to_json(const auto& aset,
                        const std::vector<pludux::backtest::Strategy>& values)
 -> Json
{
  auto json = typename Json::array();
  for(const auto& value : values) {
    json.push_back(strategy_entry_to_json<Json>(aset, value));
  }
  return json;
}

template<typename Json>
auto strategies_from_json(const Json& json)
 -> std::vector<pludux::backtest::Strategy>
{
  auto values = std::vector<pludux::backtest::Strategy>{};
  values.reserve(json.size());
  for(const auto& item : json.array_range()) {
    values.push_back(
     strategy_entry_from_json<pludux::backtest::Strategy>(item));
  }
  return values;
}

template<typename Json>
auto strategy_performance_from_json(const Json& json)
 -> pludux::backtest::StrategyPerformanceConfig
{
  using namespace pludux::backtest;
  const auto& history = json.at("history");
  const auto mode_value = required_as<int>(history, "mode");
  if(mode_value < static_cast<int>(StrategyPerformanceHistoryMode::All) ||
     mode_value >
      static_cast<int>(StrategyPerformanceHistoryMode::ExponentialDecay)) {
    throw std::invalid_argument{"Invalid strategy-performance history mode"};
  }
  const auto& bayesian = json.at("bayesian");
  const auto& win = bayesian.at("winProbability");
  if(required_as<std::string>(win, "method") != "BETA_BERNOULLI") {
    throw std::invalid_argument{"Unsupported Bayesian win model"};
  }
  const auto& win_params = win.at("params");
  const auto parse_payoff = [](const auto& payoff) {
    if(required_as<std::string>(payoff, "method") != "GAMMA_INVERSE_GAMMA") {
      throw std::invalid_argument{"Unsupported Bayesian payoff model"};
    }
    const auto& params = payoff.at("params");
    return BayesianPayoffModelNode{GammaPayoffModelNode{
     required_as<double>(params, "priorMeanMagnitude"),
     required_as<double>(params, "priorStrength"),
     required_as<double>(params, "coefficientOfVariation")}};
  };
  return StrategyPerformanceConfig{
   StrategyPerformanceHistoryPolicy{
    static_cast<StrategyPerformanceHistoryMode>(mode_value),
    required_as<std::size_t>(history, "rollingWindow"),
    required_as<double>(history, "exponentialDecay")},
   StrategyPerformanceBayesianConfig{
    BayesianWinModelNode{BetaBernoulliModelNode{
     required_as<double>(win_params, "priorWinProbability"),
     required_as<double>(win_params, "priorStrength")}},
    parse_payoff(bayesian.at("winningPayoff")),
    parse_payoff(bayesian.at("losingPayoff"))}};
}

template<typename Json>
auto strategy_performance_to_json(
 const pludux::backtest::StrategyPerformanceConfig& config) -> Json
{
  auto json = Json{};
  json["history"]["mode"] = static_cast<int>(config.history().mode());
  json["history"]["rollingWindow"] = config.history().rolling_window();
  json["history"]["exponentialDecay"] = config.history().exponential_decay();
  const auto* win_model =
   bayesian_model_node_cast<pludux::backtest::BetaBernoulliModelNode>(
    config.bayesian().win_probability_model());
  if(win_model == nullptr) {
    throw std::invalid_argument{"Unsupported Bayesian win model"};
  }
  json["bayesian"]["winProbability"]["method"] = "BETA_BERNOULLI";
  json["bayesian"]["winProbability"]["params"]["priorWinProbability"] =
   win_model->prior_probability();
  json["bayesian"]["winProbability"]["params"]["priorStrength"] =
   win_model->prior_strength();

  const auto serialize_payoff = [&json](const char* name,
                                        const auto& model_node) {
    const auto* model =
     bayesian_model_node_cast<pludux::backtest::GammaPayoffModelNode>(
      model_node);
    if(model == nullptr) {
      throw std::invalid_argument{"Unsupported Bayesian payoff model"};
    }
    json["bayesian"][name]["method"] = "GAMMA_INVERSE_GAMMA";
    json["bayesian"][name]["params"]["priorMeanMagnitude"] =
     model->prior_mean_magnitude();
    json["bayesian"][name]["params"]["priorStrength"] = model->prior_strength();
    json["bayesian"][name]["params"]["coefficientOfVariation"] =
     model->coefficient_of_variation();
  };
  serialize_payoff("winningPayoff", config.bayesian().winning_payoff_model());
  serialize_payoff("losingPayoff", config.bayesian().losing_payoff_model());
  return json;
}

} // namespace

export namespace jsoncons::reflect {

template<typename Json>
struct json_conv_traits<Json, pludux::NumericInputNode> {
  using value_type = pludux::NumericInputNode;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{
       value_type{required_as<std::string>(json, "label"),
                  static_cast<value_type::ValueRepresentation>(
                   required_as<int>(json, "representation")),
                  required_as<double>(json, "value")}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                      const value_type& input)
  {
    auto json = Json{};
    json["label"] = input.label();
    json["representation"] = static_cast<int>(input.representation());
    json["value"] = input.value();
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::PositionSizingNode> {
  using value_type = pludux::backtest::PositionSizingNode;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      using namespace pludux::backtest;
      const auto method = required_as<std::string>(json, "method");
      const auto& params = json.at("params");
      if(method == "RISK_DISTANCE") {
        return result_type{value_type{RiskDistancePositionSizing{
         required_as<double>(params, "riskFraction")}}};
      }
      if(method == "FIXED_QUANTITY") {
        return result_type{value_type{
         FixedQuantityPositionSizing{required_as<double>(params, "quantity")}}};
      }
      if(method == "FIXED_NOTIONAL") {
        return result_type{value_type{
         FixedNotionalPositionSizing{required_as<double>(params, "notional")}}};
      }
      if(method == "EQUITY_FRACTION") {
        return result_type{value_type{EquityFractionPositionSizing{
         required_as<double>(params, "equityFraction")}}};
      }
      if(method == "STRATEGY_PERFORMANCE_BAYESIAN_KELLY") {
        const auto estimate_name = required_as<std::string>(params, "estimate");
        const auto estimate =
         estimate_name == "POSTERIOR_MEAN"
          ? StrategyPerformanceBayesianKellyEstimate::PosteriorMean
         : estimate_name == "ADVERSE_QUANTILES"
           ? StrategyPerformanceBayesianKellyEstimate::AdverseQuantiles
           : throw std::invalid_argument{"Invalid Bayesian Kelly estimate"};
        return result_type{value_type{StrategyPerformanceBayesianKellySizing{
         estimate,
         required_as<double>(params, "centralCredibleMass"),
         required_as<double>(params, "kellyMultiplier"),
         required_as<double>(params, "maxEquityFraction")}}};
      }
      throw std::invalid_argument{"Unsupported position sizing method"};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                      const value_type& node)
  {
    using namespace pludux::backtest;
    auto json = Json{};
    if(const auto* value =
        position_sizing_node_cast<RiskDistancePositionSizing>(node)) {
      json["method"] = "RISK_DISTANCE";
      json["params"]["riskFraction"] = value->risk_fraction();
    } else if(const auto* value =
               position_sizing_node_cast<FixedQuantityPositionSizing>(node)) {
      json["method"] = "FIXED_QUANTITY";
      json["params"]["quantity"] = value->quantity();
    } else if(const auto* value =
               position_sizing_node_cast<FixedNotionalPositionSizing>(node)) {
      json["method"] = "FIXED_NOTIONAL";
      json["params"]["notional"] = value->notional();
    } else if(const auto* value =
               position_sizing_node_cast<EquityFractionPositionSizing>(node)) {
      json["method"] = "EQUITY_FRACTION";
      json["params"]["equityFraction"] = value->equity_fraction();
    } else if(const auto* value = position_sizing_node_cast<
               StrategyPerformanceBayesianKellySizing>(node)) {
      json["method"] = "STRATEGY_PERFORMANCE_BAYESIAN_KELLY";
      json["params"]["estimate"] =
       value->estimate() ==
         StrategyPerformanceBayesianKellyEstimate::PosteriorMean
        ? "POSTERIOR_MEAN"
        : "ADVERSE_QUANTILES";
      json["params"]["centralCredibleMass"] = value->central_credible_mass();
      json["params"]["kellyMultiplier"] = value->kelly_multiplier();
      json["params"]["maxEquityFraction"] = value->maximum_equity_fraction();
    } else {
      throw std::invalid_argument{"Unsupported position sizing method"};
    }
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::DrawdownAdjustment> {
  using value_type = pludux::backtest::DrawdownAdjustment;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{
       value_type{required_as<bool>(json, "enabled"),
                  required_as<double>(json, "drawdownStep"),
                  required_as<double>(json, "sizeReduction")}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                      const value_type& adjustment)
  {
    auto json = Json{};
    json["enabled"] = adjustment.enabled();
    json["drawdownStep"] = adjustment.drawdown_step();
    json["sizeReduction"] = adjustment.size_reduction();
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::InsufficientCashPolicy> {
  using value_type = pludux::backtest::InsufficientCashPolicy;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_string();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      const auto value = json.template as<std::string>();
      if(value == "Reject") {
        return result_type{value_type::Reject};
      }
      if(value == "CapToAvailableCash") {
        return result_type{value_type::CapToAvailableCash};
      }
      return conversion_failed<value_type>();
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                      const value_type& policy)
  {
    switch(policy) {
    case value_type::Reject:
      return Json{"Reject"};
    case value_type::CapToAvailableCash:
      return Json{"CapToAvailableCash"};
    }

    return Json{"Reject"};
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::Profile> {
  using value_type = pludux::backtest::Profile;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{value_type{
       required_as<std::string>(json, "name"),
       required_as<pludux::backtest::PositionSizingNode>(json,
                                                         "positionSizing"),
       required_as<pludux::backtest::DrawdownAdjustment>(json,
                                                         "drawdownAdjustment"),
       required_as<pludux::backtest::InsufficientCashPolicy>(
        json, "insufficientCashPolicy"),
       pludux::backtest::parse_execution_filter_node(
        json.at("executionFilter"))}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>& aset,
                      const value_type& profile)
  {
    auto json = Json{};
    json["name"] = profile.name();
    set_json(json, aset, "positionSizing", profile.position_sizing());
    set_json(json, aset, "drawdownAdjustment", profile.drawdown_adjustment());
    set_json(
     json, aset, "insufficientCashPolicy", profile.insufficient_cash_policy());
    json["executionFilter"] = pludux::backtest::serialize_execution_filter_node(
     profile.execution_filter());
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::BrokerFee> {
  using value_type = pludux::backtest::BrokerFee;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{
       value_type{required_as<std::string>(json, "name"),
                  static_cast<value_type::FeeType>(
                   required_as<std::size_t>(json, "feeType")),
                  static_cast<value_type::FeePosition>(
                   required_as<std::size_t>(json, "feePosition")),
                  static_cast<value_type::FeeTrigger>(
                   required_as<std::size_t>(json, "feeTrigger")),
                  required_as<double>(json, "feeValue")}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                      const value_type& fee)
  {
    auto json = Json{};
    json["name"] = fee.name();
    json["feeType"] = static_cast<std::size_t>(fee.fee_type());
    json["feePosition"] = static_cast<std::size_t>(fee.fee_position());
    json["feeTrigger"] = static_cast<std::size_t>(fee.fee_trigger());
    json["feeValue"] = fee.value();
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::Broker> {
  using value_type = pludux::backtest::Broker;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{value_type{
       required_as<std::string>(json, "name"),
       vector_from_json<pludux::backtest::BrokerFee>(json.at("fees"))}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>& aset,
                      const value_type& broker)
  {
    auto json = Json{};
    json["name"] = broker.name();
    json["fees"] = vector_to_json<Json>(aset, broker.fees());
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::Market> {
  using value_type = pludux::backtest::Market;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{
       value_type{required_as<std::string>(json, "name"),
                  required_as<double>(json, "minOrderQuantity"),
                  required_as<double>(json, "quantityStep")}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                      const value_type& market)
  {
    auto json = Json{};
    json["name"] = market.name();
    json["minOrderQuantity"] = market.min_order_quantity();
    json["quantityStep"] = market.quantity_step();
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::AssetQuoteFieldResolver> {
  using value_type = pludux::AssetQuoteFieldResolver;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{
       value_type{required_as<std::string>(json, "datetimeField"),
                  required_as<std::string>(json, "openField"),
                  required_as<std::string>(json, "highField"),
                  required_as<std::string>(json, "lowField"),
                  required_as<std::string>(json, "closeField"),
                  required_as<std::string>(json, "volumeField")}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                      const value_type& resolver)
  {
    auto json = Json{};
    json["datetimeField"] = resolver.datetime_field();
    json["openField"] = resolver.open_field();
    json["highField"] = resolver.high_field();
    json["lowField"] = resolver.low_field();
    json["closeField"] = resolver.close_field();
    json["volumeField"] = resolver.volume_field();
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::AssetData> {
  using value_type = pludux::AssetData;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      auto data = std::vector<double>{};
      for(const auto& item : json.at("data").array_range()) {
        data.push_back(item.template as<double>());
      }

      auto asset_data = value_type{};
      asset_data.data(std::move(data));
      return result_type{std::move(asset_data)};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                      const value_type& asset_data)
  {
    auto json = Json{};
    json["data"] = asset_data.data();
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::AssetHistory> {
  using value_type = pludux::AssetHistory;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      auto history = value_type{};
      for(const auto& [field, data_json] :
          json.at("fieldData").object_range()) {
        history.insert(field, data_json.template as<pludux::AssetData>());
      }
      return result_type{std::move(history)};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>& aset,
                      const value_type& history)
  {
    auto json = Json{};
    auto field_data_json = Json{};
    for(const auto& [field, data] : history.field_data()) {
      set_json(field_data_json, aset, field, data);
    }
    json["fieldData"] = std::move(field_data_json);
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::Asset> {
  using value_type = pludux::backtest::Asset;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{value_type{
       required_as<std::string>(json, "name"),
       required_as<pludux::AssetHistory>(json, "history"),
       required_as<pludux::AssetQuoteFieldResolver>(json, "fieldResolver")}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>& aset,
                      const value_type& asset)
  {
    auto json = Json{};
    json["name"] = asset.name();
    set_json(json, aset, "history", asset.history());
    set_json(json, aset, "fieldResolver", asset.field_resolver());
    return json;
  }
};

template<typename Json, typename THandle>
  requires std::same_as<THandle, pludux::backtest::AssetStoreHandle> ||
           std::same_as<THandle, pludux::backtest::StrategyStoreHandle> ||
           std::same_as<THandle, pludux::backtest::MarketStoreHandle> ||
           std::same_as<THandle, pludux::backtest::BrokerStoreHandle> ||
           std::same_as<THandle, pludux::backtest::ProfileStoreHandle> ||
           std::same_as<THandle, pludux::backtest::BacktestStoreHandle>
struct json_conv_traits<Json, THandle> {
  using value_type = THandle;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{
       value_type{required_as<std::size_t>(json, "slotIndex"),
                  required_as<std::size_t>(json, "generation")}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                      const value_type& handle)
  {
    auto json = Json{};
    json["slotIndex"] = handle.slot_index();
    json["generation"] = handle.generation();
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::StoreSlot> {
  using value_type = pludux::backtest::StoreSlot;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{
       value_type{required_as<std::size_t>(json, "valueIndex"),
                  required_as<std::size_t>(json, "generation"),
                  required_as<bool>(json, "alive")}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                      const value_type& slot)
  {
    auto json = Json{};
    json["valueIndex"] = slot.value_index();
    json["generation"] = slot.generation();
    json["alive"] = slot.alive();
    return json;
  }
};

template<typename Json, typename TValue, typename THandle>
struct json_conv_traits<Json,
                        pludux::backtest::StoreDataResolver<TValue, THandle>> {
  using value_type = pludux::backtest::StoreDataResolver<TValue, THandle>;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{value_type{
       vector_from_json<pludux::backtest::StoreSlot>(json.at("slots")),
       required_as<std::vector<std::size_t>>(json, "valueToSlotIndices"),
       required_as<std::vector<std::size_t>>(json, "freeSlotIndices")}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>& aset,
                      const value_type& resolver)
  {
    auto json = Json{};
    json["slots"] = vector_to_json<Json>(aset, resolver.slots());
    json["valueToSlotIndices"] = resolver.value_to_slot_indices();
    json["freeSlotIndices"] = resolver.free_slot_indices();
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::Backtest> {
  using value_type = pludux::backtest::Backtest;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{value_type{
       required_as<std::string>(json, "name"),
       required_as<double>(json, "initialCapital"),
       required_as<pludux::backtest::AssetStoreHandle>(json, "asset"),
       required_as<pludux::backtest::StrategyStoreHandle>(json, "strategy"),
       required_as<pludux::backtest::MarketStoreHandle>(json, "market"),
       required_as<pludux::backtest::BrokerStoreHandle>(json, "broker"),
       required_as<pludux::backtest::ProfileStoreHandle>(json, "profile"),
       vector_from_json<pludux::NumericInputNode>(json.at("inputs")),
       strategy_performance_from_json(json.at("strategyPerformance"))}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>& aset,
                      const value_type& backtest)
  {
    auto json = Json{};
    json["name"] = backtest.name();
    json["initialCapital"] = backtest.initial_capital();
    set_json(json, aset, "asset", backtest.asset_handle());
    set_json(json, aset, "strategy", backtest.strategy_handle());
    set_json(json, aset, "market", backtest.market_handle());
    set_json(json, aset, "broker", backtest.broker_handle());
    set_json(json, aset, "profile", backtest.profile_handle());
    json["inputs"] = vector_to_json<Json>(aset, backtest.inputs());
    json["strategyPerformance"] =
     strategy_performance_to_json<Json>(backtest.strategy_performance());
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::StoreDescriptor> {
  using value_type = pludux::backtest::StoreDescriptor;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      auto backtest_timeline_resolver = pludux::backtest::StoreDataResolver<
       pludux::backtest::BacktestTimeline,
       pludux::backtest::BacktestStoreHandle>{};
      auto series_results_resolver = pludux::backtest::StoreDataResolver<
       pludux::SeriesEvaluationResults,
       pludux::backtest::BacktestStoreHandle>{};

      return result_type{value_type{
       required_as<pludux::backtest::StoreDataResolver<
        pludux::backtest::Backtest,
        pludux::backtest::BacktestStoreHandle>>(json,
                                                "backtestStoreDataResolver"),
       required_as<pludux::backtest::StoreDataResolver<
        pludux::backtest::Asset,
        pludux::backtest::AssetStoreHandle>>(json, "assetStoreDataResolver"),
       required_as<pludux::backtest::StoreDataResolver<
        pludux::backtest::Strategy,
        pludux::backtest::StrategyStoreHandle>>(json,
                                                "strategyStoreDataResolver"),
       required_as<pludux::backtest::StoreDataResolver<
        pludux::backtest::Market,
        pludux::backtest::MarketStoreHandle>>(json, "marketStoreDataResolver"),
       required_as<pludux::backtest::StoreDataResolver<
        pludux::backtest::Broker,
        pludux::backtest::BrokerStoreHandle>>(json, "brokerStoreDataResolver"),
       required_as<pludux::backtest::StoreDataResolver<
        pludux::backtest::Profile,
        pludux::backtest::ProfileStoreHandle>>(json,
                                               "profileStoreDataResolver"),
       std::move(backtest_timeline_resolver),
       std::move(series_results_resolver)}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>& aset,
                      const value_type& descriptor)
  {
    auto json = Json{};
    set_json(json,
             aset,
             "backtestStoreDataResolver",
             descriptor.backtest_store_data_resolver());
    set_json(json,
             aset,
             "assetStoreDataResolver",
             descriptor.asset_store_data_resolver());
    set_json(json,
             aset,
             "strategyStoreDataResolver",
             descriptor.strategy_store_data_resolver());
    set_json(json,
             aset,
             "marketStoreDataResolver",
             descriptor.market_store_data_resolver());
    set_json(json,
             aset,
             "brokerStoreDataResolver",
             descriptor.broker_store_data_resolver());
    set_json(json,
             aset,
             "profileStoreDataResolver",
             descriptor.profile_store_data_resolver());
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::StoreArena> {
  using value_type = pludux::backtest::StoreArena;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{value_type{
       vector_from_json<pludux::backtest::Backtest>(json.at("backtests")),
       vector_from_json<pludux::backtest::Asset>(json.at("assets")),
       strategies_from_json(json.at("strategies")),
       vector_from_json<pludux::backtest::Market>(json.at("markets")),
       vector_from_json<pludux::backtest::Broker>(json.at("brokers")),
       vector_from_json<pludux::backtest::Profile>(json.at("profiles")),
       {},
       {}}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>& aset,
                      const value_type& arena)
  {
    auto json = Json{};
    json["backtests"] = vector_to_json<Json>(aset, arena.backtests());
    json["assets"] = vector_to_json<Json>(aset, arena.assets());
    json["strategies"] = strategies_to_json<Json>(aset, arena.strategies());
    json["markets"] = vector_to_json<Json>(aset, arena.markets());
    json["brokers"] = vector_to_json<Json>(aset, arena.brokers());
    json["profiles"] = vector_to_json<Json>(aset, arena.profiles());
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::Store> {
  using value_type = pludux::backtest::Store;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{value_type{
       required_as<pludux::backtest::StoreDescriptor>(json, "descriptor"),
       required_as<pludux::backtest::StoreArena>(json, "arena")}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>& aset,
                      const value_type& store)
  {
    auto json = Json{};
    set_json(json, aset, "descriptor", store.descriptor());
    set_json(json, aset, "arena", store.arena());
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::apps::UiState> {
  using value_type = pludux::apps::UiState;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      return result_type{
       value_type{required_as<std::string>(json, "imguiIniSettings"),
                  required_as<pludux::backtest::BacktestStoreHandle>(
                   json, "selectedBacktestHandle"),
                  vector_from_json<pludux::backtest::BacktestStoreHandle>(
                   json.at("backtestHandles")),
                  vector_from_json<pludux::backtest::AssetStoreHandle>(
                   json.at("assetHandles")),
                  vector_from_json<pludux::backtest::StrategyStoreHandle>(
                   json.at("strategyHandles")),
                  vector_from_json<pludux::backtest::MarketStoreHandle>(
                   json.at("marketHandles")),
                  vector_from_json<pludux::backtest::BrokerStoreHandle>(
                   json.at("brokerHandles")),
                  vector_from_json<pludux::backtest::ProfileStoreHandle>(
                   json.at("profileHandles"))}};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>& aset,
                      const value_type& ui_state)
  {
    auto json = Json{};
    json["imguiIniSettings"] = ui_state.imgui_ini_settings();
    set_json(
     json, aset, "selectedBacktestHandle", ui_state.selected_backtest_handle());
    json["backtestHandles"] =
     vector_to_json<Json>(aset, ui_state.backtest_handles());
    json["assetHandles"] = vector_to_json<Json>(aset, ui_state.asset_handles());
    json["strategyHandles"] =
     vector_to_json<Json>(aset, ui_state.strategy_handles());
    json["marketHandles"] =
     vector_to_json<Json>(aset, ui_state.market_handles());
    json["brokerHandles"] =
     vector_to_json<Json>(aset, ui_state.broker_handles());
    json["profileHandles"] =
     vector_to_json<Json>(aset, ui_state.profile_handles());
    return json;
  }
};

template<typename Json>
struct json_conv_traits<Json, pludux::apps::ApplicationState> {
  using value_type = pludux::apps::ApplicationState;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      auto app_state =
       value_type{required_as<pludux::backtest::Store>(json, "store"),
                  required_as<pludux::apps::UiState>(json, "uiState")};

      if(required_as<std::string>(json, "$version") != PLUDUX_VERSION) {
        app_state.reset_all_backtests();
      }

      return result_type{std::move(app_state)};
    } catch(...) {
      return conversion_failed<value_type>();
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>& aset,
                      const value_type& app_state)
  {
    auto json = Json{};
    json["$version"] = std::string{PLUDUX_VERSION};
    set_json(json, aset, "store", app_state.store());
    set_json(json, aset, "uiState", app_state.ui_state());
    return json;
  }
};

} // namespace jsoncons::reflect

export namespace pludux::apps {

void save_application_state_json(std::ostream& out_stream,
                                 const ApplicationState& app_state)
{
  auto root = jsoncons::ojson{};
  root["pludux"] =
   jsoncons::reflect::json_conv_traits<jsoncons::ojson, ApplicationState>::
    to_json(jsoncons::make_alloc_set(), app_state);

  out_stream << root.to_string();
}

auto load_application_state_json(std::istream& in_stream) -> ApplicationState
{
  const auto root_json = jsoncons::ojson::parse(in_stream);
  return root_json.at("pludux").as<ApplicationState>();
}

} // namespace pludux::apps
