module;

#include <format>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <jsoncons/json.hpp>

export module pludux.backtest:config_parser;

import pludux;

import :drawdown_node;
import :equity_node;
import :position_node;
import :pyramiding_layer_node;
import :requested_order_node;
import :risk_distance_node;
import :stop_target_price_node;
import :model_performance_node;

export namespace pludux::backtest {

auto parse_config_json(const std::string& config) -> jsoncons::ojson;
auto config_number(const jsoncons::ojson& config) -> double;
auto config_boolean(const jsoncons::ojson& config) -> bool;
auto config_string(const jsoncons::ojson& config) -> std::string;

} // namespace pludux::backtest

export namespace pludux::backtest {

template<typename TContext>
class ConfigParser {
public:
  using Node = ErasedNode<TContext>;

  class Parser {
  public:
    Parser(ConfigParser& config_parser)
    : config_parser_{config_parser}
    {
    }

    auto parse_node(this Parser& self, const jsoncons::ojson& config) -> Node
    {
      return self.config_parser_.parse_node(config);
    }

  private:
    ConfigParser& config_parser_;
  };

  friend Parser;

  using NodeSerialize =
   std::function<auto(const ConfigParser&, const Node&)->jsoncons::ojson>;

  using NodeDeserialize =
   std::function<auto(Parser, const jsoncons::ojson&)->Node>;

  ConfigParser() = default;

  auto use_series_params(this const ConfigParser& self) noexcept -> bool
  {
    return self.use_series_params_;
  }

  void use_series_params(this ConfigParser& self,
                         bool use_series_params) noexcept
  {
    self.use_series_params_ = use_series_params;
  }

  void register_node_parser(this ConfigParser& self,
                            const std::string& node_name,
                            const NodeSerialize& node_serialize,
                            const NodeDeserialize& node_deserialize)
  {
    const auto [_, inserted] = self.node_parsers_.emplace(
     node_name, std::make_pair(node_serialize, node_deserialize));
    if(!inserted) {
      throw std::invalid_argument{
       std::format("Node parser '{}' is already registered", node_name)};
    }
  }

  auto parser(this ConfigParser& self) -> Parser
  {
    return Parser{self};
  }

  auto parse_node(this ConfigParser& self, const std::string& config_node_str)
   -> Node
  {
    return self.parse_node(parse_config_json(config_node_str));
  }

  auto parse_node(this ConfigParser& self, const jsoncons::ojson& config_node)
   -> Node
  {
    if(config_node.is_number()) {
      return Node{ValueNode{config_number(config_node)}};
    }

    if(config_node.is_bool()) {
      if constexpr(node_context_admissible<TContext, TrueNode>() &&
                   node_context_admissible<TContext, FalseNode>()) {
        return config_boolean(config_node) ? Node{TrueNode{}}
                                           : Node{FalseNode{}};
      } else {
        throw std::invalid_argument{
         "Boolean shorthand is not admissible for this expression context"};
      }
    }

    if(config_node.is_string()) {
      const auto node_name = config_string(config_node);
      const auto expanded_node = jsoncons::ojson::object{{"method", node_name}};
      return self.parse_node(expanded_node);
    }

    const auto node_name = config_string(config_node.at("method"));

    auto& node_parsers = self.node_parsers_;
    if(!node_parsers.contains(node_name)) {
      const auto error_message = std::format("Unknown node: {}", node_name);
      throw std::invalid_argument{error_message};
    }

    try {
      const auto node_deserialize = node_parsers.at(node_name).second;
      auto json_params = jsoncons::ojson{};
      if(self.use_series_params_) {
        json_params = config_node.contains("params")
                       ? config_node.at("params")
                       : jsoncons::ojson::object();
      } else {
        json_params = config_node;
      }

      return node_deserialize(self, json_params);
    } catch(const std::exception& e) {
      const auto error_message =
       std::format("Error parsing node {}:\n{}", node_name, e.what());
      throw std::invalid_argument{error_message};
    } catch(...) {
      const auto error_message =
       std::format("Unknown error parsing node {}", node_name);
      throw std::invalid_argument{error_message};
    }
  }

  auto serialize_node(this const ConfigParser& self, const Node& node)
   -> jsoncons::ojson
  {
    auto serialized_node = jsoncons::ojson::null();
    auto matched_name = std::optional<std::string>{};
    for(const auto& [node_name, node_parser] : self.node_parsers_) {
      const auto& [node_params_serialize, _] = node_parser;
      auto serialized_params_node = node_params_serialize(self, node);
      if(!serialized_params_node.is_null()) {
        if(matched_name) {
          throw std::invalid_argument{std::format(
           "Multiple node parsers match serialization: '{}' and '{}'",
           *matched_name,
           node_name)};
        }
        matched_name = node_name;
        serialized_node = jsoncons::ojson{};
        if(self.use_series_params_) {
          serialized_node["method"] = node_name;
          if(!serialized_params_node.empty()) {
            serialized_node["params"] = std::move(serialized_params_node);
          }
        } else {
          serialized_node = std::move(serialized_params_node);
          serialized_node["method"] = node_name;
        }
      }
    }

    return serialized_node;
  }

private:
  std::unordered_map<std::string, std::pair<NodeSerialize, NodeDeserialize>>
   node_parsers_;

  bool use_series_params_{true};
};

auto make_backtest_model_config_parser() -> ConfigParser<BacktestMethodContext>;

auto make_requested_order_comparator_config_parser()
 -> ConfigParser<RequestedOrderMethodContext>;

auto parse_entry_filter_node(const jsoncons::ojson& config)
 -> ErasedNode<EntryFilterMethodContext>;

auto serialize_entry_filter_node(
 const ErasedNode<EntryFilterMethodContext>& node) -> jsoncons::ojson;

} // namespace pludux::backtest

namespace pludux::backtest {

auto parse_config_json(const std::string& config) -> jsoncons::ojson
{
  return jsoncons::ojson::parse(config);
}

auto config_number(const jsoncons::ojson& config) -> double
{
  return config.as_double();
}

auto config_boolean(const jsoncons::ojson& config) -> bool
{
  return config.as_bool();
}

auto config_string(const jsoncons::ojson& config) -> std::string
{
  return config.as_string();
}

template<typename T>
static auto get_param_or(const jsoncons::ojson& parameters,
                         const std::string& key,
                         const T& default_value) -> T
{
  return parameters.contains(key) ? parameters.at(key).as<T>() : default_value;
}

static auto parse_ma_node_type(const std::string& value,
                               MaNodeType fallback = MaNodeType::Rma)
 -> MaNodeType
{
  if(value == "SMA")
    return MaNodeType::Sma;
  if(value == "EMA")
    return MaNodeType::Ema;
  if(value == "WMA")
    return MaNodeType::Wma;
  if(value == "HMA")
    return MaNodeType::Hma;
  if(value == "RMA")
    return MaNodeType::Rma;
  return fallback;
}

static auto parse_kc_band_node_type(const std::string& value) -> KcBandNodeType
{
  if(value == "TR")
    return KcBandNodeType::Tr;
  if(value == "Range")
    return KcBandNodeType::RangeHighLow;
  return KcBandNodeType::Atr;
}

static auto serialize_ma_node_type(MaNodeType value,
                                   std::string fallback = "RMA") -> std::string
{
  switch(value) {
  case MaNodeType::Sma:
    return "SMA";
  case MaNodeType::Ema:
    return "EMA";
  case MaNodeType::Wma:
    return "WMA";
  case MaNodeType::Hma:
    return "HMA";
  case MaNodeType::Rma:
    return "RMA";
  }
  return fallback;
}

static auto serialize_kc_band_node_type(KcBandNodeType value) -> std::string
{
  switch(value) {
  case KcBandNodeType::Atr:
    return "ATR";
  case KcBandNodeType::Tr:
    return "TR";
  case KcBandNodeType::RangeHighLow:
    return "Range";
  }
  return "ATR";
}

enum class EntryFilterNodeKind { Scalar, Boolean };

static auto parse_model_performance_metric(const std::string& value)
 -> ModelPerformanceMetric
{
  static const auto metrics =
   std::unordered_map<std::string, ModelPerformanceMetric>{
    {"LIFETIME_COUNT", ModelPerformanceMetric::LifetimeCount},
    {"EFFECTIVE_COUNT", ModelPerformanceMetric::EffectiveCount},
    {"WIN_RATE", ModelPerformanceMetric::WinRate},
    {"BREAK_EVEN_RATE", ModelPerformanceMetric::BreakEvenRate},
    {"LOSS_RATE", ModelPerformanceMetric::LossRate},
    {"CURRENT_WINNING_STREAK", ModelPerformanceMetric::CurrentWinningStreak},
    {"CURRENT_LOSING_STREAK", ModelPerformanceMetric::CurrentLosingStreak},
    {"MAXIMUM_WINNING_STREAK", ModelPerformanceMetric::MaximumWinningStreak},
    {"MAXIMUM_LOSING_STREAK", ModelPerformanceMetric::MaximumLosingStreak},
    {"MEAN_RETURN", ModelPerformanceMetric::MeanReturn},
    {"RETURN_STANDARD_DEVIATION",
     ModelPerformanceMetric::ReturnStandardDeviation},
    {"BAYESIAN_WIN_PROBABILITY",
     ModelPerformanceMetric::BayesianWinProbability},
    {"BAYESIAN_WIN_PROBABILITY_LOWER_95",
     ModelPerformanceMetric::BayesianWinProbabilityLower95},
    {"BAYESIAN_WIN_PROBABILITY_UPPER_95",
     ModelPerformanceMetric::BayesianWinProbabilityUpper95},
    {"BAYESIAN_WINNING_PAYOFF", ModelPerformanceMetric::BayesianWinningPayoff},
    {"BAYESIAN_WINNING_PAYOFF_LOWER_95",
     ModelPerformanceMetric::BayesianWinningPayoffLower95},
    {"BAYESIAN_WINNING_PAYOFF_UPPER_95",
     ModelPerformanceMetric::BayesianWinningPayoffUpper95},
    {"BAYESIAN_LOSING_PAYOFF", ModelPerformanceMetric::BayesianLosingPayoff},
    {"BAYESIAN_LOSING_PAYOFF_LOWER_95",
     ModelPerformanceMetric::BayesianLosingPayoffLower95},
    {"BAYESIAN_LOSING_PAYOFF_UPPER_95",
     ModelPerformanceMetric::BayesianLosingPayoffUpper95}};

  if(const auto metric = metrics.find(value); metric != metrics.end()) {
    return metric->second;
  }
  throw std::invalid_argument{
   "EntryFilter strategy-performance metric is invalid"};
}

static auto serialize_model_performance_metric(ModelPerformanceMetric metric)
 -> std::string
{
  switch(metric) {
  case ModelPerformanceMetric::LifetimeCount:
    return "LIFETIME_COUNT";
  case ModelPerformanceMetric::EffectiveCount:
    return "EFFECTIVE_COUNT";
  case ModelPerformanceMetric::WinRate:
    return "WIN_RATE";
  case ModelPerformanceMetric::BreakEvenRate:
    return "BREAK_EVEN_RATE";
  case ModelPerformanceMetric::LossRate:
    return "LOSS_RATE";
  case ModelPerformanceMetric::CurrentWinningStreak:
    return "CURRENT_WINNING_STREAK";
  case ModelPerformanceMetric::CurrentLosingStreak:
    return "CURRENT_LOSING_STREAK";
  case ModelPerformanceMetric::MaximumWinningStreak:
    return "MAXIMUM_WINNING_STREAK";
  case ModelPerformanceMetric::MaximumLosingStreak:
    return "MAXIMUM_LOSING_STREAK";
  case ModelPerformanceMetric::MeanReturn:
    return "MEAN_RETURN";
  case ModelPerformanceMetric::ReturnStandardDeviation:
    return "RETURN_STANDARD_DEVIATION";
  case ModelPerformanceMetric::BayesianWinProbability:
    return "BAYESIAN_WIN_PROBABILITY";
  case ModelPerformanceMetric::BayesianWinProbabilityLower95:
    return "BAYESIAN_WIN_PROBABILITY_LOWER_95";
  case ModelPerformanceMetric::BayesianWinProbabilityUpper95:
    return "BAYESIAN_WIN_PROBABILITY_UPPER_95";
  case ModelPerformanceMetric::BayesianWinningPayoff:
    return "BAYESIAN_WINNING_PAYOFF";
  case ModelPerformanceMetric::BayesianWinningPayoffLower95:
    return "BAYESIAN_WINNING_PAYOFF_LOWER_95";
  case ModelPerformanceMetric::BayesianWinningPayoffUpper95:
    return "BAYESIAN_WINNING_PAYOFF_UPPER_95";
  case ModelPerformanceMetric::BayesianLosingPayoff:
    return "BAYESIAN_LOSING_PAYOFF";
  case ModelPerformanceMetric::BayesianLosingPayoffLower95:
    return "BAYESIAN_LOSING_PAYOFF_LOWER_95";
  case ModelPerformanceMetric::BayesianLosingPayoffUpper95:
    return "BAYESIAN_LOSING_PAYOFF_UPPER_95";
  }
  throw std::invalid_argument{
   "EntryFilter strategy-performance metric is invalid"};
}

struct ParsedEntryFilterNode {
  ErasedNode<EntryFilterMethodContext> node;
  EntryFilterNodeKind kind;
};

static auto parse_entry_filter_child(const jsoncons::ojson& config)
 -> ParsedEntryFilterNode
{
  using Context = EntryFilterMethodContext;

  if(config.is_bool()) {
    return {config.as_bool() ? ErasedNode<Context>{TrueNode{}}
                             : ErasedNode<Context>{FalseNode{}},
            EntryFilterNodeKind::Boolean};
  }
  if(config.is_number()) {
    return {ErasedNode<Context>{ValueNode{config.as_double()}},
            EntryFilterNodeKind::Scalar};
  }
  if(!config.is_object() || !config.contains("method")) {
    throw std::invalid_argument{
     "EntryFilter node must be a boolean, number, or method object"};
  }

  const auto method = config.at("method").as_string();
  if(method == "LOGIC.ALWAYS") {
    return {ErasedNode<Context>{TrueNode{}}, EntryFilterNodeKind::Boolean};
  }
  if(method == "LOGIC.NEVER") {
    return {ErasedNode<Context>{FalseNode{}}, EntryFilterNodeKind::Boolean};
  }
  if(method == "VALUE.CONSTANT") {
    return {ErasedNode<Context>{ValueNode{config.at("value").as_double()}},
            EntryFilterNodeKind::Scalar};
  }
  if(method == "PORTFOLIO.EQUITY") {
    return {ErasedNode<Context>{EquityNode{}}, EntryFilterNodeKind::Scalar};
  }
  if(method == "PORTFOLIO.EQUITY_PERCENT") {
    return {ErasedNode<Context>{EquityPercentNode{}},
            EntryFilterNodeKind::Scalar};
  }
  if(method == "PORTFOLIO.DRAWDOWN") {
    return {ErasedNode<Context>{DrawdownNode{}}, EntryFilterNodeKind::Scalar};
  }
  if(method == "MODEL_PERFORMANCE.VALUE") {
    if(!config.contains("params") || !config.at("params").is_object() ||
       !config.at("params").contains("metric") ||
       !config.at("params").at("metric").is_string()) {
      throw std::invalid_argument{
       "EntryFilter strategy-performance metric is invalid"};
    }
    return {
     ErasedNode<Context>{ModelPerformanceNode{parse_model_performance_metric(
      config.at("params").at("metric").as<std::string>())}},
     EntryFilterNodeKind::Scalar};
  }

#define PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE(Id, Type)       \
  if(method == Id) {                                                   \
    return {ErasedNode<Context>{Type{}}, EntryFilterNodeKind::Scalar}; \
  }
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE("REQUESTED_ORDER.PRICE",
                                                 RequestedOrderPriceNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE("REQUESTED_ORDER.DIRECTION",
                                                 RequestedOrderDirectionNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   "REQUESTED_ORDER.IS_PYRAMIDING", IsPyramidingOrderNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE("REQUESTED_ORDER.RAW_QUANTITY",
                                                 RawRequestedQuantityNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   "REQUESTED_ORDER.RAW_QUANTITY_LIMIT", RawRequestedQuantityLimitNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY", DrawdownAdjustedQuantityNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY_LIMIT",
   DrawdownAdjustedQuantityLimitNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE("REQUESTED_ORDER.QUANTITY",
                                                 RequestedQuantityNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE("REQUESTED_ORDER.NOTIONAL",
                                                 RequestedNotionalNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE("REQUESTED_ORDER.COST",
                                                 RequestedCostNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   "REQUESTED_ORDER.ESTIMATED_ENTRY_FEE", EstimatedEntryFeeNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   "REQUESTED_ORDER.ESTIMATED_1R_EXIT_FEE", EstimatedOneRExitFeeNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   "REQUESTED_ORDER.RISK_DISTANCE", RequestedOrderRiskDistanceNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE("REQUESTED_ORDER.PRICE_RISK",
                                                 RequestedPriceRiskNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   "REQUESTED_ORDER.RISK_WITH_FEES", RequestedRiskWithFeesNode)
  PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   "REQUESTED_ORDER.FROZEN_UNIT_QUANTITY", FrozenUnitQuantityNode)
#undef PLUDUX_PARSE_ENTRY_FILTER_REQUESTED_ORDER_NODE

  const auto parse_scalar = [&](const char* key) {
    auto parsed = parse_entry_filter_child(config.at(key));
    if(parsed.kind != EntryFilterNodeKind::Scalar) {
      throw std::invalid_argument{
       std::format("EntryFilter '{}' must be scalar", key)};
    }
    return parsed.node;
  };
  const auto parse_boolean = [&](const char* key) {
    auto parsed = parse_entry_filter_child(config.at(key));
    if(parsed.kind != EntryFilterNodeKind::Boolean) {
      throw std::invalid_argument{
       std::format("EntryFilter '{}' must be boolean", key)};
    }
    return parsed.node;
  };

  if(method == "COMPARISON.GREATER_THAN") {
    return {ErasedNode<Context>{GreaterThanNode<Context>{
             parse_scalar("target"), parse_scalar("threshold")}},
            EntryFilterNodeKind::Boolean};
  }
  if(method == "COMPARISON.GREATER_EQUAL") {
    return {ErasedNode<Context>{GreaterEqualNode<Context>{
             parse_scalar("target"), parse_scalar("threshold")}},
            EntryFilterNodeKind::Boolean};
  }
  if(method == "COMPARISON.LESS_THAN") {
    return {ErasedNode<Context>{LessThanNode<Context>{
             parse_scalar("target"), parse_scalar("threshold")}},
            EntryFilterNodeKind::Boolean};
  }
  if(method == "COMPARISON.LESS_EQUAL") {
    return {ErasedNode<Context>{LessEqualNode<Context>{
             parse_scalar("target"), parse_scalar("threshold")}},
            EntryFilterNodeKind::Boolean};
  }
  if(method == "COMPARISON.EQUAL") {
    return {ErasedNode<Context>{EqualNode<Context>{parse_scalar("target"),
                                                   parse_scalar("threshold")}},
            EntryFilterNodeKind::Boolean};
  }
  if(method == "COMPARISON.NOT_EQUAL") {
    return {ErasedNode<Context>{NotEqualNode<Context>{
             parse_scalar("target"), parse_scalar("threshold")}},
            EntryFilterNodeKind::Boolean};
  }
  if(method == "LOGIC.AND") {
    return {
     ErasedNode<Context>{LogicalAndNode<Context>{
      parse_boolean("firstCondition"), parse_boolean("secondCondition")}},
     EntryFilterNodeKind::Boolean};
  }
  if(method == "LOGIC.OR") {
    return {
     ErasedNode<Context>{LogicalOrNode<Context>{
      parse_boolean("firstCondition"), parse_boolean("secondCondition")}},
     EntryFilterNodeKind::Boolean};
  }
  if(method == "LOGIC.XOR") {
    return {
     ErasedNode<Context>{LogicalXorNode<Context>{
      parse_boolean("firstCondition"), parse_boolean("secondCondition")}},
     EntryFilterNodeKind::Boolean};
  }
  if(method == "LOGIC.NOT") {
    return {
     ErasedNode<Context>{LogicalNotNode<Context>{parse_boolean("condition")}},
     EntryFilterNodeKind::Boolean};
  }

  throw std::invalid_argument{
   std::format("Node '{}' is not allowed in EntryFilter", method)};
}

static auto
serialize_entry_filter_child(const ErasedNode<EntryFilterMethodContext>& node)
 -> std::pair<jsoncons::ojson, EntryFilterNodeKind>
{
  using Context = EntryFilterMethodContext;
  const auto object = [](std::string method) -> jsoncons::ojson {
    auto config = jsoncons::ojson{};
    config["method"] = std::move(method);
    return config;
  };
  if(node_cast<TrueNode>(node)) {
    return {object("LOGIC.ALWAYS"), EntryFilterNodeKind::Boolean};
  }
  if(node_cast<FalseNode>(node)) {
    return {object("LOGIC.NEVER"), EntryFilterNodeKind::Boolean};
  }
  if(const auto* value = node_cast<ValueNode>(node)) {
    auto config = object("VALUE.CONSTANT");
    config["value"] = value->value();
    return {std::move(config), EntryFilterNodeKind::Scalar};
  }
  if(node_cast<EquityNode>(node)) {
    return {object("PORTFOLIO.EQUITY"), EntryFilterNodeKind::Scalar};
  }
  if(node_cast<EquityPercentNode>(node)) {
    return {object("PORTFOLIO.EQUITY_PERCENT"), EntryFilterNodeKind::Scalar};
  }
  if(node_cast<DrawdownNode>(node)) {
    return {object("PORTFOLIO.DRAWDOWN"), EntryFilterNodeKind::Scalar};
  }
  if(const auto* performance = node_cast<ModelPerformanceNode>(node)) {
    auto config = object("MODEL_PERFORMANCE.VALUE");
    config["params"]["metric"] =
     serialize_model_performance_metric(performance->metric());
    return {std::move(config), EntryFilterNodeKind::Scalar};
  }

#define PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(Type, Id) \
  if(node_cast<Type>(node)) {                                        \
    return {object(Id), EntryFilterNodeKind::Scalar};                \
  }
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(RequestedOrderPriceNode,
                                                     "REQUESTED_ORDER.PRICE")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   RequestedOrderDirectionNode, "REQUESTED_ORDER.DIRECTION")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   IsPyramidingOrderNode, "REQUESTED_ORDER.IS_PYRAMIDING")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   RawRequestedQuantityNode, "REQUESTED_ORDER.RAW_QUANTITY")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   RawRequestedQuantityLimitNode, "REQUESTED_ORDER.RAW_QUANTITY_LIMIT")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   DrawdownAdjustedQuantityNode, "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   DrawdownAdjustedQuantityLimitNode,
   "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY_LIMIT")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(RequestedQuantityNode,
                                                     "REQUESTED_ORDER.QUANTITY")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(RequestedNotionalNode,
                                                     "REQUESTED_ORDER.NOTIONAL")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(RequestedCostNode,
                                                     "REQUESTED_ORDER.COST")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   EstimatedEntryFeeNode, "REQUESTED_ORDER.ESTIMATED_ENTRY_FEE")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   EstimatedOneRExitFeeNode, "REQUESTED_ORDER.ESTIMATED_1R_EXIT_FEE")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   RequestedOrderRiskDistanceNode, "REQUESTED_ORDER.RISK_DISTANCE")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   RequestedPriceRiskNode, "REQUESTED_ORDER.PRICE_RISK")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   RequestedRiskWithFeesNode, "REQUESTED_ORDER.RISK_WITH_FEES")
  PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE(
   FrozenUnitQuantityNode, "REQUESTED_ORDER.FROZEN_UNIT_QUANTITY")
#undef PLUDUX_SERIALIZE_ENTRY_FILTER_REQUESTED_ORDER_NODE

  const auto serialize_comparison = [&](const auto* comparison,
                                        std::string method)
   -> std::optional<std::pair<jsoncons::ojson, EntryFilterNodeKind>> {
    if(!comparison) {
      return std::nullopt;
    }
    auto config = object(std::move(method));
    config["target"] = serialize_entry_filter_child(comparison->target()).first;
    config["threshold"] =
     serialize_entry_filter_child(comparison->threshold()).first;
    return std::pair{std::move(config), EntryFilterNodeKind::Boolean};
  };
#define PLUDUX_SERIALIZE_FILTER_COMPARISON(Type, Name)              \
  if(auto result =                                                  \
      serialize_comparison(node_cast<Type<Context>>(node), Name)) { \
    return *std::move(result);                                      \
  }
  PLUDUX_SERIALIZE_FILTER_COMPARISON(GreaterThanNode, "COMPARISON.GREATER_THAN")
  PLUDUX_SERIALIZE_FILTER_COMPARISON(GreaterEqualNode,
                                     "COMPARISON.GREATER_EQUAL")
  PLUDUX_SERIALIZE_FILTER_COMPARISON(LessThanNode, "COMPARISON.LESS_THAN")
  PLUDUX_SERIALIZE_FILTER_COMPARISON(LessEqualNode, "COMPARISON.LESS_EQUAL")
  PLUDUX_SERIALIZE_FILTER_COMPARISON(EqualNode, "COMPARISON.EQUAL")
  PLUDUX_SERIALIZE_FILTER_COMPARISON(NotEqualNode, "COMPARISON.NOT_EQUAL")
#undef PLUDUX_SERIALIZE_FILTER_COMPARISON

  const auto serialize_binary = [&](const auto* logical, std::string method)
   -> std::optional<std::pair<jsoncons::ojson, EntryFilterNodeKind>> {
    if(!logical) {
      return std::nullopt;
    }
    auto config = object(std::move(method));
    config["firstCondition"] =
     serialize_entry_filter_child(logical->first_condition()).first;
    config["secondCondition"] =
     serialize_entry_filter_child(logical->second_condition()).first;
    return std::pair{std::move(config), EntryFilterNodeKind::Boolean};
  };
#define PLUDUX_SERIALIZE_FILTER_BINARY(Type, Name)                           \
  if(auto result = serialize_binary(node_cast<Type<Context>>(node), Name)) { \
    return *std::move(result);                                               \
  }
  PLUDUX_SERIALIZE_FILTER_BINARY(LogicalAndNode, "LOGIC.AND")
  PLUDUX_SERIALIZE_FILTER_BINARY(LogicalOrNode, "LOGIC.OR")
  PLUDUX_SERIALIZE_FILTER_BINARY(LogicalXorNode, "LOGIC.XOR")
#undef PLUDUX_SERIALIZE_FILTER_BINARY

  if(const auto* logical = node_cast<LogicalNotNode<Context>>(node)) {
    auto config = object("LOGIC.NOT");
    config["condition"] =
     serialize_entry_filter_child(logical->other_condition()).first;
    return {std::move(config), EntryFilterNodeKind::Boolean};
  }

  throw std::invalid_argument{"EntryFilter contains an unsupported node"};
}

auto parse_entry_filter_node(const jsoncons::ojson& config)
 -> ErasedNode<EntryFilterMethodContext>
{
  auto parsed = parse_entry_filter_child(config);
  if(parsed.kind != EntryFilterNodeKind::Boolean) {
    throw std::invalid_argument{"EntryFilter root must be boolean"};
  }
  return std::move(parsed.node);
}

auto serialize_entry_filter_node(
 const ErasedNode<EntryFilterMethodContext>& node) -> jsoncons::ojson
{
  auto [config, kind] = serialize_entry_filter_child(node);
  if(kind != EntryFilterNodeKind::Boolean) {
    throw std::invalid_argument{"EntryFilter root must be boolean"};
  }
  return config;
}

template<typename TContext>
auto make_model_config_parser_for() -> ConfigParser<TContext>
{
  using Parser = ConfigParser<TContext>;
  using Node = ErasedNode<TContext>;
  auto parser = Parser{};

  parser.register_node_parser(
   "VALUE.CONSTANT",
   [](const Parser&, const Node& node) -> jsoncons::ojson {
     const auto* value = node_cast<ValueNode>(node);
     return value ? jsoncons::ojson::object{{"value", value->value()}}
                  : jsoncons::ojson::null();
   },
   [](typename Parser::Parser, const jsoncons::ojson& params) -> Node {
     return ValueNode{params.at("value").as_double()};
   });
  parser.register_node_parser(
   "MARKET_DATA.FIELD",
   [](const Parser&, const Node& node) -> jsoncons::ojson {
     const auto* data = node_cast<DataNode>(node);
     return data ? jsoncons::ojson::object{{"field", data->field()}}
                 : jsoncons::ojson::null();
   },
   [](typename Parser::Parser, const jsoncons::ojson& params) -> Node {
     return DataNode{params.at("field").as_string()};
   });

  const auto register_parameterless = [&]<typename TNode>(const char* name) {
    parser.register_node_parser(
     name,
     [](const Parser&, const Node& node) -> jsoncons::ojson {
       return node_cast<TNode>(node) ? jsoncons::ojson{}
                                     : jsoncons::ojson::null();
     },
     [](typename Parser::Parser, const jsoncons::ojson&) -> Node {
       return TNode{};
     });
  };
  register_parameterless.template operator()<OpenNode>("MARKET_DATA.OPEN");
  register_parameterless.template operator()<HighNode>("MARKET_DATA.HIGH");
  register_parameterless.template operator()<LowNode>("MARKET_DATA.LOW");
  register_parameterless.template operator()<CloseNode>("MARKET_DATA.CLOSE");
  register_parameterless.template operator()<VolumeNode>("MARKET_DATA.VOLUME");
  register_parameterless.template operator()<PyramidingLayerNode>(
   "POSITION.PYRAMIDING_LAYER");
  register_parameterless.template operator()<InitialEntryPriceNode>(
   "POSITION.INITIAL_ENTRY_PRICE");
  register_parameterless.template operator()<LatestEntryPriceNode>(
   "POSITION.LATEST_ENTRY_PRICE");
  register_parameterless.template operator()<AveragePriceNode>(
   "POSITION.AVERAGE_PRICE");
  register_parameterless.template operator()<StopTargetRefPriceNode>(
   "POSITION.STOP_TARGET_REF_PRICE");
  register_parameterless.template operator()<PositionDirectionNode>(
   "POSITION.DIRECTION");
  register_parameterless.template operator()<Sl1RNode>("STOP_LOSS.ONE_R");
  register_parameterless.template operator()<TrueNode>("LOGIC.ALWAYS");
  register_parameterless.template operator()<FalseNode>("LOGIC.NEVER");

  parser.register_node_parser(
   "SERIES.REFERENCE",
   [](const Parser&, const Node& node) -> jsoncons::ojson {
     const auto* series = node_cast<SeriesNode>(node);
     return series ? jsoncons::ojson::object{{"name", series->name()}}
                   : jsoncons::ojson::null();
   },
   [](typename Parser::Parser, const jsoncons::ojson& params) -> Node {
     return SeriesNode{get_param_or<std::string>(params, "name", "")};
   });

  const auto register_binary = [&]<typename TNode>(const char* name,
                                                   const char* first,
                                                   const char* second) {
    parser.register_node_parser(
     name,
     [first, second](const Parser& parser,
                     const Node& node) -> jsoncons::ojson {
       const auto* binary = node_cast<TNode>(node);
       if(!binary) {
         return jsoncons::ojson::null();
       }
       return jsoncons::ojson::object{
        {first, parser.serialize_node(binary->operand1())},
        {second, parser.serialize_node(binary->operand2())}};
     },
     [first, second](typename Parser::Parser parser,
                     const jsoncons::ojson& params) -> Node {
       return TNode{parser.parse_node(params.at(first)),
                    parser.parse_node(params.at(second))};
     });
  };
  register_binary
   .template operator()<BinaryOperatorNode<std::plus<>, TContext>>(
    "OPERATOR.ADD", "augend", "addend");
  register_binary
   .template operator()<BinaryOperatorNode<std::minus<>, TContext>>(
    "OPERATOR.SUBTRACT", "minuend", "subtrahend");
  register_binary
   .template operator()<BinaryOperatorNode<std::multiplies<>, TContext>>(
    "OPERATOR.MULTIPLY", "multiplicand", "multiplier");
  register_binary
   .template operator()<BinaryOperatorNode<std::divides<>, TContext>>(
    "OPERATOR.DIVIDE", "dividend", "divisor");
  register_binary.template operator()<BinaryOperatorNode<Maximum<>, TContext>>(
   "OPERATOR.MAX", "left", "right");
  register_binary.template operator()<BinaryOperatorNode<Minimum<>, TContext>>(
   "OPERATOR.MIN", "left", "right");

  const auto register_comparison = [&]<typename TNode>(const char* name) {
    parser.register_node_parser(
     name,
     [](const Parser& parser, const Node& node) -> jsoncons::ojson {
       const auto* comparison = node_cast<TNode>(node);
       if(!comparison) {
         return jsoncons::ojson::null();
       }
       return jsoncons::ojson::object{
        {"target", parser.serialize_node(comparison->target())},
        {"threshold", parser.serialize_node(comparison->threshold())}};
     },
     [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
       return TNode{parser.parse_node(params.at("target")),
                    parser.parse_node(params.at("threshold"))};
     });
  };
  register_comparison.template operator()<GreaterThanNode<TContext>>(
   "COMPARISON.GREATER_THAN");
  register_comparison.template operator()<GreaterEqualNode<TContext>>(
   "COMPARISON.GREATER_EQUAL");
  register_comparison.template operator()<LessThanNode<TContext>>(
   "COMPARISON.LESS_THAN");
  register_comparison.template operator()<LessEqualNode<TContext>>(
   "COMPARISON.LESS_EQUAL");
  register_comparison.template operator()<EqualNode<TContext>>(
   "COMPARISON.EQUAL");
  register_comparison.template operator()<NotEqualNode<TContext>>(
   "COMPARISON.NOT_EQUAL");

  const auto register_cross = [&]<typename TNode>(const char* name) {
    parser.register_node_parser(
     name,
     [](const Parser& parser, const Node& node) -> jsoncons::ojson {
       const auto* cross = node_cast<TNode>(node);
       if(!cross) {
         return jsoncons::ojson::null();
       }
       return jsoncons::ojson::object{
        {"value", parser.serialize_node(cross->source())},
        {"baseline", parser.serialize_node(cross->reference())}};
     },
     [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
       return TNode{parser.parse_node(params.at("value")),
                    parser.parse_node(params.at("baseline"))};
     });
  };
  register_cross.template operator()<CrossoverNode<TContext>>(
   "COMPARISON.CROSSOVER");
  register_cross.template operator()<CrossunderNode<TContext>>(
   "COMPARISON.CROSSUNDER");

  parser.register_node_parser(
   "OPERATOR.CHANGE",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* change = node_cast<ChangeNode<TContext>>(node);
     return change ? jsoncons::ojson::object{{"source",
                                              parser.serialize_node(
                                               change->source())}}
                   : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     return ChangeNode<TContext>{params.contains("source")
                                  ? parser.parse_node(params.at("source"))
                                  : Node{CloseNode{}}};
   });

  const auto register_distance = [&]<typename TNode>(const char* name,
                                                     const char* value_key,
                                                     double fallback) {
    parser.register_node_parser(
     name,
     [value_key](const Parser& parser, const Node& node) -> jsoncons::ojson {
       const auto* distance = node_cast<TNode>(node);
       if(!distance) {
         return jsoncons::ojson::null();
       }
       return jsoncons::ojson::object{
        {value_key, parser.serialize_node(distance->value())}};
     },
     [value_key, fallback](typename Parser::Parser parser,
                           const jsoncons::ojson& params) -> Node {
       return TNode{params.contains(value_key)
                     ? parser.parse_node(params.at(value_key))
                     : Node{ValueNode{fallback}}};
     });
  };
  register_distance.template operator()<RiskDistanceAmountNode<TContext>>(
   "RISK_DISTANCE.AMOUNT", "amount", 1.0);
  register_distance.template operator()<RiskDistancePercentNode<TContext>>(
   "RISK_DISTANCE.PERCENT", "percentage", 1.0);
  register_distance.template operator()<SlAmountNode<TContext>>(
   "STOP_LOSS.AMOUNT", "amount", 0.0);
  register_distance.template operator()<TpAmountNode<TContext>>(
   "TAKE_PROFIT.AMOUNT", "amount", 0.0);
  register_distance.template operator()<SlPercentNode<TContext>>(
   "STOP_LOSS.PERCENT", "percent", 0.0);
  register_distance.template operator()<TpPercentNode<TContext>>(
   "TAKE_PROFIT.PERCENT", "percent", 0.0);
  register_distance.template operator()<SlRMultipleNode<TContext>>(
   "STOP_LOSS.R_MULTIPLE", "multiple", 1.0);
  register_distance.template operator()<TpRMultipleNode<TContext>>(
   "TAKE_PROFIT.R_MULTIPLE", "multiple", 2.0);

  const auto register_atr_distance = [&]<typename TNode>(const char* name) {
    parser.register_node_parser(
     name,
     [](const Parser& parser, const Node& node) -> jsoncons::ojson {
       const auto* distance = node_cast<TNode>(node);
       if(!distance) {
         return jsoncons::ojson::null();
       }
       return jsoncons::ojson::object{
        {"period", parser.serialize_node(distance->period())},
        {"multiplier", parser.serialize_node(distance->multiplier())},
        {"maSmoothingType",
         distance->ma_smoothing_type() == MaNodeType::Sma     ? "SMA"
         : distance->ma_smoothing_type() == MaNodeType::Ema   ? "EMA"
          : distance->ma_smoothing_type() == MaNodeType::Wma  ? "WMA"
           : distance->ma_smoothing_type() == MaNodeType::Hma ? "HMA"
                                                              : "RMA"}};
     },
     [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
       return TNode{
        params.contains("period") ? parser.parse_node(params.at("period"))
                                  : Node{ValueNode{14.0}},
        params.contains("multiplier")
         ? parser.parse_node(params.at("multiplier"))
         : Node{ValueNode{2.0}},
        [&] {
          const auto type =
           get_param_or<std::string>(params, "maSmoothingType", "RMA");
          return type == "SMA"     ? MaNodeType::Sma
                 : type == "EMA"   ? MaNodeType::Ema
                  : type == "WMA"  ? MaNodeType::Wma
                   : type == "HMA" ? MaNodeType::Hma
                                   : MaNodeType::Rma;
        }()};
     });
  };
  register_atr_distance.template operator()<RiskDistanceAtrNode<TContext>>(
   "RISK_DISTANCE.ATR");
  register_atr_distance.template operator()<SlAtrNode<TContext>>(
   "STOP_LOSS.ATR");
  register_atr_distance.template operator()<TpAtrNode<TContext>>(
   "TAKE_PROFIT.ATR");

  parser.register_node_parser(
   "INPUT.NUMERIC",
   [](const Parser&, const Node& node) -> jsoncons::ojson {
     const auto* input = node_cast<NumericInputNode>(node);
     if(!input) {
       return jsoncons::ojson::null();
     }
     const auto representation =
      input->representation() ==
        NumericInputNode::ValueRepresentation::SignedInteger
       ? "SignedInteger"
      : input->representation() ==
         NumericInputNode::ValueRepresentation::UnsignedInteger
        ? "UnsignedInteger"
        : "Decimal";
     return jsoncons::ojson::object{{"label", input->label()},
                                    {"representation", representation},
                                    {"value", input->value()}};
   },
   [](typename Parser::Parser, const jsoncons::ojson& params) -> Node {
     const auto representation =
      get_param_or<std::string>(params, "representation", "Decimal");
     return NumericInputNode{
      get_param_or<std::string>(params, "label", ""),
      representation == "SignedInteger"
       ? NumericInputNode::ValueRepresentation::SignedInteger
      : representation == "UnsignedInteger"
        ? NumericInputNode::ValueRepresentation::UnsignedInteger
        : NumericInputNode::ValueRepresentation::Decimal,
      get_param_or<double>(params, "value", 0.0)};
   });

  parser.register_node_parser(
   "OPERATOR.LOOKBACK",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* value = node_cast<LookbackNode<TContext>>(node);
     return value
             ? jsoncons::ojson::object{{"period", value->period()},
                                       {"source",
                                        parser.serialize_node(value->source())}}
             : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     return LookbackNode<TContext>{params.contains("source")
                                    ? parser.parse_node(params.at("source"))
                                    : Node{CloseNode{}},
                                   params.at("period").as<std::size_t>()};
   });

  const auto register_ta = [&]<typename TNode>(const char* name,
                                               std::size_t default_period) {
    parser.register_node_parser(
     name,
     [](const Parser& parser, const Node& node) -> jsoncons::ojson {
       const auto* value = node_cast<TNode>(node);
       return value ? jsoncons::ojson::object{{"period",
                                               parser.serialize_node(
                                                value->period())},
                                              {"source",
                                               parser.serialize_node(
                                                value->source())}}
                    : jsoncons::ojson::null();
     },
     [default_period](typename Parser::Parser parser,
                      const jsoncons::ojson& params) -> Node {
       return TNode{params.contains("source")
                     ? parser.parse_node(params.at("source"))
                     : Node{CloseNode{}},
                    params.contains("period")
                     ? parser.parse_node(params.at("period"))
                     : Node{NumericInputNode{
                        "Period",
                        NumericInputNode::ValueRepresentation::UnsignedInteger,
                        static_cast<double>(default_period)}}};
     });
  };
  register_ta.template operator()<SmaNode<TContext>>("INDICATOR.SMA", 20);
  register_ta.template operator()<EmaNode<TContext>>("INDICATOR.EMA", 20);
  register_ta.template operator()<RmaNode<TContext>>("INDICATOR.RMA", 20);
  register_ta.template operator()<WmaNode<TContext>>("INDICATOR.WMA", 20);
  register_ta.template operator()<HmaNode<TContext>>("INDICATOR.HMA", 20);
  register_ta.template operator()<HighestNode<TContext>>("INDICATOR.HIGHEST",
                                                         14);
  register_ta.template operator()<LowestNode<TContext>>("INDICATOR.LOWEST", 14);
  register_ta.template operator()<RocNode<TContext>>("INDICATOR.ROC", 14);
  register_ta.template operator()<RsiNode<TContext>>("INDICATOR.RSI", 14);
  register_ta.template operator()<StddevNode<TContext>>("INDICATOR.STDDEV", 20);

  parser.register_node_parser(
   "INDICATOR.RVOL",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* value = node_cast<RvolNode<TContext>>(node);
     return value
             ? jsoncons::ojson::object{{"period",
                                        parser.serialize_node(value->period())}}
             : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     return RvolNode<TContext>{
      params.contains("period")
       ? parser.parse_node(params.at("period"))
       : Node{NumericInputNode{
          "Period",
          NumericInputNode::ValueRepresentation::UnsignedInteger,
          14.0}}};
   });
  parser.register_node_parser(
   "INDICATOR.TR",
   [](const Parser&, const Node& node) -> jsoncons::ojson {
     return node_cast<TrNode<TContext>>(node) ? jsoncons::ojson{}
                                              : jsoncons::ojson::null();
   },
   [](typename Parser::Parser, const jsoncons::ojson&) -> Node {
     return TrNode<TContext>{};
   });
  parser.register_node_parser(
   "INDICATOR.ATR",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* value = node_cast<AtrNode<TContext>>(node);
     return value
             ? jsoncons::ojson::object{{"period",
                                        parser.serialize_node(value->period())},
                                       {"maSmoothingType",
                                        serialize_ma_node_type(
                                         value->ma_smoothing_type())}}
             : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     auto value = AtrNode<TContext>{
      params.contains("period")
       ? parser.parse_node(params.at("period"))
       : Node{NumericInputNode{
          "Period",
          NumericInputNode::ValueRepresentation::UnsignedInteger,
          14.0}}};
     value.ma_smoothing_type(parse_ma_node_type(
      get_param_or<std::string>(params, "maSmoothingType", "RMA")));
     return value;
   });

  const auto register_unary = [&]<typename TNode>(const char* name) {
    parser.register_node_parser(
     name,
     [](const Parser& parser, const Node& node) -> jsoncons::ojson {
       const auto* value = node_cast<TNode>(node);
       return value ? jsoncons::ojson::object{{"operand",
                                               parser.serialize_node(
                                                value->operand())}}
                    : jsoncons::ojson::null();
     },
     [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
       return TNode{parser.parse_node(params.at("operand"))};
     });
  };
  register_binary
   .template operator()<BinaryOperatorNode<AbsoluteDifference<>, TContext>>(
    "OPERATOR.ABS_DIFF", "minuend", "subtrahend");
  register_unary.template
  operator()<UnaryOperatorNode<std::negate<>, TContext>>("OPERATOR.NEGATE");
  register_unary.template operator()<UnaryOperatorNode<Absolute<>, TContext>>(
   "OPERATOR.ABS");
  register_unary.template operator()<UnaryOperatorNode<SquareRoot<>, TContext>>(
   "OPERATOR.SQRT");
  register_unary
   .template operator()<UnaryOperatorNode<PositivePart<>, TContext>>(
    "OPERATOR.POSITIVE_PART");
  register_unary
   .template operator()<UnaryOperatorNode<NegativePart<>, TContext>>(
    "OPERATOR.NEGATIVE_PART");

  const auto register_logical = [&]<typename TNode>(const char* name) {
    parser.register_node_parser(
     name,
     [](const Parser& parser, const Node& node) -> jsoncons::ojson {
       const auto* value = node_cast<TNode>(node);
       return value ? jsoncons::ojson::object{{"firstCondition",
                                               parser.serialize_node(
                                                value->first_condition())},
                                              {"secondCondition",
                                               parser.serialize_node(
                                                value->second_condition())}}
                    : jsoncons::ojson::null();
     },
     [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
       return TNode{parser.parse_node(params.at("firstCondition")),
                    parser.parse_node(params.at("secondCondition"))};
     });
  };
  register_logical.template operator()<LogicalAndNode<TContext>>("LOGIC.AND");
  register_logical.template operator()<LogicalOrNode<TContext>>("LOGIC.OR");
  register_logical.template operator()<LogicalXorNode<TContext>>("LOGIC.XOR");
  parser.register_node_parser(
   "LOGIC.NOT",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* value = node_cast<LogicalNotNode<TContext>>(node);
     return value ? jsoncons::ojson::object{{"condition",
                                             parser.serialize_node(
                                              value->other_condition())}}
                  : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     return LogicalNotNode<TContext>{parser.parse_node(params.at("condition"))};
   });

  const auto register_collection = [&]<typename TNode>(const char* name) {
    parser.register_node_parser(
     name,
     [](const Parser& parser, const Node& node) -> jsoncons::ojson {
       const auto* value = node_cast<TNode>(node);
       if(!value) {
         return jsoncons::ojson::null();
       }
       auto items = jsoncons::ojson::array();
       for(const auto& item : value->conditions()) {
         items.push_back(parser.serialize_node(item));
       }
       return jsoncons::ojson::object{{"items", std::move(items)}};
     },
     [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
       const auto& items = params.at("items");
       if(!items.is_array()) {
         throw std::invalid_argument{"'items' must be an array"};
       }
       auto conditions = std::vector<Node>{};
       conditions.reserve(items.size());
       for(const auto& item : items.array_range()) {
         conditions.push_back(parser.parse_node(item));
       }
       return TNode{std::move(conditions)};
     });
  };
  register_collection.template operator()<AllOfNode<TContext>>("LOGIC.ALL_OF");
  register_collection.template operator()<AnyOfNode<TContext>>("LOGIC.ANY_OF");

  parser.register_node_parser(
   "POSITION.R_MULTIPLE",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* value = node_cast<PositionRMultipleNode<TContext>>(node);
     return value
             ? jsoncons::ojson::object{{"source",
                                        parser.serialize_node(value->source())}}
             : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     return PositionRMultipleNode<TContext>{
      params.contains("source") ? parser.parse_node(params.at("source"))
                                : Node{CloseNode{}}};
   });

  const auto register_context_value = [&]<typename TNode>(const char* name) {
    parser.register_node_parser(
     name,
     [](const Parser&, const Node& node) -> jsoncons::ojson {
       return node_cast<TNode>(node) ? jsoncons::ojson{}
                                     : jsoncons::ojson::null();
     },
     [](typename Parser::Parser, const jsoncons::ojson&) -> Node {
       return TNode{};
     });
  };
  register_context_value.template operator()<EquityNode>("PORTFOLIO.EQUITY");
  register_context_value.template operator()<EquityPercentNode>(
   "PORTFOLIO.EQUITY_PERCENT");
  register_context_value.template operator()<DrawdownNode>(
   "PORTFOLIO.DRAWDOWN");

  parser.register_node_parser(
   "INDICATOR.BB",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* bb = node_cast<BbNode<TContext>>(node);
     return bb ? jsoncons::ojson::object{{"maType",
                                          serialize_ma_node_type(
                                           bb->ma_node_type(), "SMA")},
                                         {"maSource",
                                          parser.serialize_node(bb->source())},
                                         {"period",
                                          parser.serialize_node(bb->period())},
                                         {"stddev",
                                          parser.serialize_node(bb->stddev())}}
               : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     return BbNode<TContext>{
      params.contains("maSource") ? parser.parse_node(params.at("maSource"))
                                  : Node{CloseNode{}},
      params.contains("period")
       ? parser.parse_node(params.at("period"))
       : Node{NumericInputNode{
          "Period",
          NumericInputNode::ValueRepresentation::UnsignedInteger,
          20.0}},
      params.contains("stddev")
       ? parser.parse_node(params.at("stddev"))
       : Node{NumericInputNode{
          "StdDev", NumericInputNode::ValueRepresentation::Decimal, 2.0}},
      parse_ma_node_type(get_param_or<std::string>(params, "maType", "SMA"),
                         MaNodeType::Sma)};
   });

  parser.register_node_parser(
   "INDICATOR.MACD",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* macd = node_cast<MacdNode<TContext>>(node);
     return macd
             ? jsoncons::ojson::object{{"fastPeriod",
                                        parser.serialize_node(
                                         macd->fast_period())},
                                       {"slowPeriod",
                                        parser.serialize_node(
                                         macd->slow_period())},
                                       {"signalPeriod",
                                        parser.serialize_node(
                                         macd->signal_period())},
                                       {"source",
                                        parser.serialize_node(macd->source())}}
             : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     const auto input =
      [&parser, &params](const char* key, const char* label, double value) {
        return params.contains(key)
                ? parser.parse_node(params.at(key))
                : Node{NumericInputNode{
                   label,
                   NumericInputNode::ValueRepresentation::UnsignedInteger,
                   value}};
      };
     return MacdNode<TContext>{params.contains("source")
                                ? parser.parse_node(params.at("source"))
                                : Node{CloseNode{}},
                               input("fastPeriod", "Fast Period", 12.0),
                               input("slowPeriod", "Slow Period", 26.0),
                               input("signalPeriod", "Signal Period", 9.0)};
   });

  parser.register_node_parser(
   "INDICATOR.STOCH",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* stoch = node_cast<StochNode<TContext>>(node);
     return stoch
             ? jsoncons::ojson::
                object{{"kPeriod", parser.serialize_node(stoch->k_period())},
                       {"kSmooth", parser.serialize_node(stoch->k_smooth())},
                       {"dPeriod", parser.serialize_node(stoch->d_period())}}
             : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     const auto input =
      [&parser, &params](const char* key, const char* label, double value) {
        return params.contains(key)
                ? parser.parse_node(params.at(key))
                : Node{NumericInputNode{
                   label,
                   NumericInputNode::ValueRepresentation::UnsignedInteger,
                   value}};
      };
     return StochNode<TContext>{input("kPeriod", "K Period", 5.0),
                                input("kSmooth", "K Smooth", 3.0),
                                input("dPeriod", "D Period", 3.0)};
   });

  parser.register_node_parser(
   "INDICATOR.STOCH_RSI",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* stoch = node_cast<StochRsiNode<TContext>>(node);
     return stoch
             ? jsoncons::ojson::
                object{{"rsiSource",
                        parser.serialize_node(stoch->rsi_source())},
                       {"rsiPeriod",
                        parser.serialize_node(stoch->rsi_period())},
                       {"kPeriod", parser.serialize_node(stoch->k_period())},
                       {"kSmooth", parser.serialize_node(stoch->k_smooth())},
                       {"dPeriod", parser.serialize_node(stoch->d_period())}}
             : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     const auto input =
      [&parser, &params](const char* key, const char* label, double value) {
        return params.contains(key)
                ? parser.parse_node(params.at(key))
                : Node{NumericInputNode{
                   label,
                   NumericInputNode::ValueRepresentation::UnsignedInteger,
                   value}};
      };
     return StochRsiNode<TContext>{params.contains("rsiSource")
                                    ? parser.parse_node(params.at("rsiSource"))
                                    : Node{CloseNode{}},
                                   input("rsiPeriod", "RSI Period", 14.0),
                                   input("kPeriod", "K Period", 5.0),
                                   input("kSmooth", "K Smooth", 3.0),
                                   input("dPeriod", "D Period", 3.0)};
   });

  parser.register_node_parser(
   "INDICATOR.DC",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* dc = node_cast<DonchianChannelNode<TContext>>(node);
     return dc ? jsoncons::ojson::object{{"period",
                                          parser.serialize_node(dc->period())}}
               : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     return DonchianChannelNode<TContext>{
      params.contains("period")
       ? parser.parse_node(params.at("period"))
       : Node{NumericInputNode{
          "Period",
          NumericInputNode::ValueRepresentation::UnsignedInteger,
          20.0}}};
   });

  parser.register_node_parser(
   "INDICATOR.KC",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* kc = node_cast<KcNode<TContext>>(node);
     return kc
             ? jsoncons::ojson::
                object{{"maMethodType",
                        serialize_ma_node_type(kc->ma_node_type(), "EMA")},
                       {"period", parser.serialize_node(kc->period())},
                       {"maSource", parser.serialize_node(kc->source())},
                       {"bandMethodType",
                        serialize_kc_band_node_type(kc->band_node_type())},
                       {"bandAtrPeriod",
                        parser.serialize_node(kc->band_atr_period())},
                       {"multiplier", parser.serialize_node(kc->multiplier())}}
             : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     return KcNode<TContext>{
      params.contains("maSource") ? parser.parse_node(params.at("maSource"))
                                  : Node{CloseNode{}},
      params.contains("period")
       ? parser.parse_node(params.at("period"))
       : Node{NumericInputNode{
          "Period",
          NumericInputNode::ValueRepresentation::UnsignedInteger,
          20.0}},
      params.contains("multiplier")
       ? parser.parse_node(params.at("multiplier"))
       : Node{NumericInputNode{
          "Multiplier", NumericInputNode::ValueRepresentation::Decimal, 1.5}},
      params.contains("bandAtrPeriod")
       ? parser.parse_node(params.at("bandAtrPeriod"))
       : Node{NumericInputNode{
          "Band ATR Period",
          NumericInputNode::ValueRepresentation::UnsignedInteger,
          14.0}},
      parse_kc_band_node_type(
       get_param_or<std::string>(params, "bandMethodType", "ATR")),
      parse_ma_node_type(
       get_param_or<std::string>(params, "maMethodType", "EMA"),
       MaNodeType::Ema)};
   });

  parser.register_node_parser(
   "OPERATOR.SELECT_OUTPUT",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* select_output = node_cast<SelectOutputNode<TContext>>(node);
     if(!select_output) {
       return jsoncons::ojson::null();
     }
     const auto output = [&] -> std::string {
       switch(select_output->output()) {
       case NodeOutput::MacdLine:
         return "macd-line";
       case NodeOutput::SignalLine:
         return "signal-line";
       case NodeOutput::Histogram:
         return "histogram";
       case NodeOutput::KPercent:
         return "k-percent";
       case NodeOutput::DPercent:
         return "d-percent";
       case NodeOutput::MiddleBand:
         return "middle-band";
       case NodeOutput::UpperBand:
         return "upper-band";
       case NodeOutput::LowerBand:
         return "lower-band";
       }
       return "default";
     }();
     return jsoncons::ojson::object{
      {"output", output},
      {"source", parser.serialize_node(select_output->source())}};
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     const auto output_name =
      get_param_or<std::string>(params, "output", "default");
     const auto output =
      output_name == "macd-line"          ? NodeOutput::MacdLine
      : output_name == "signal-line"      ? NodeOutput::SignalLine
       : output_name == "histogram"       ? NodeOutput::Histogram
        : output_name == "k-percent"      ? NodeOutput::KPercent
         : output_name == "d-percent"     ? NodeOutput::DPercent
          : output_name == "middle-band"  ? NodeOutput::MiddleBand
           : output_name == "upper-band"  ? NodeOutput::UpperBand
            : output_name == "lower-band" ? NodeOutput::LowerBand
                                          : static_cast<NodeOutput>(-1);
     return SelectOutputNode<TContext>{params.contains("source")
                                        ? parser.parse_node(params.at("source"))
                                        : Node{CloseNode{}},
                                       output};
   });

  parser.register_node_parser(
   "OPERATOR.PERCENTAGE",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* percentage = node_cast<PercentageNode<TContext>>(node);
     return percentage
             ? jsoncons::ojson::object{{"base",
                                        parser.serialize_node(
                                         percentage->base())},
                                       {"percent", percentage->percent()}}
             : jsoncons::ojson::null();
   },
   [](typename Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     return PercentageNode<TContext>{
      params.contains("base") ? parser.parse_node(params.at("base"))
                              : Node{CloseNode{}},
      get_param_or<double>(params, "percent", 100.0)};
   });

  return parser;
}

auto make_backtest_model_config_parser() -> ConfigParser<BacktestMethodContext>
{
  return make_model_config_parser_for<BacktestMethodContext>();
}

auto make_requested_order_comparator_config_parser()
 -> ConfigParser<RequestedOrderMethodContext>
{
  using Context = RequestedOrderMethodContext;
  using Parser = ConfigParser<Context>;
  using Node = ErasedNode<Context>;
  auto parser = Parser{};

  parser.register_node_parser(
   "VALUE.CONSTANT",
   [](const Parser&, const Node& node) -> jsoncons::ojson {
     const auto* value = node_cast<ValueNode>(node);
     return value ? jsoncons::ojson::object{{"value", value->value()}}
                  : jsoncons::ojson::null();
   },
   [](Parser::Parser, const jsoncons::ojson& params) -> Node {
     return ValueNode{params.at("value").as_double()};
   });
  parser.register_node_parser(
   "MARKET_DATA.FIELD",
   [](const Parser&, const Node& node) -> jsoncons::ojson {
     const auto* data = node_cast<DataNode>(node);
     return data ? jsoncons::ojson::object{{"field", data->field()}}
                 : jsoncons::ojson::null();
   },
   [](Parser::Parser, const jsoncons::ojson& params) -> Node {
     return DataNode{params.at("field").as_string()};
   });

  const auto register_parameterless = [&]<typename TNode>(const char* name) {
    parser.register_node_parser(
     name,
     [](const Parser&, const Node& node) -> jsoncons::ojson {
       return node_cast<TNode>(node) ? jsoncons::ojson{}
                                     : jsoncons::ojson::null();
     },
     [](Parser::Parser, const jsoncons::ojson&) -> Node { return TNode{}; });
  };
  register_parameterless.template operator()<OpenNode>("MARKET_DATA.OPEN");
  register_parameterless.template operator()<HighNode>("MARKET_DATA.HIGH");
  register_parameterless.template operator()<LowNode>("MARKET_DATA.LOW");
  register_parameterless.template operator()<CloseNode>("MARKET_DATA.CLOSE");
  register_parameterless.template operator()<VolumeNode>("MARKET_DATA.VOLUME");
  register_parameterless.template operator()<RequestedOrderPriceNode>(
   "REQUESTED_ORDER.PRICE");
  register_parameterless.template operator()<RequestedOrderDirectionNode>(
   "REQUESTED_ORDER.DIRECTION");
  register_parameterless.template operator()<IsPyramidingOrderNode>(
   "REQUESTED_ORDER.IS_PYRAMIDING");
  register_parameterless.template operator()<RawRequestedQuantityNode>(
   "REQUESTED_ORDER.RAW_QUANTITY");
  register_parameterless.template operator()<RawRequestedQuantityLimitNode>(
   "REQUESTED_ORDER.RAW_QUANTITY_LIMIT");
  register_parameterless.template operator()<DrawdownAdjustedQuantityNode>(
   "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY");
  register_parameterless.template operator()<DrawdownAdjustedQuantityLimitNode>(
   "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY_LIMIT");
  register_parameterless.template operator()<RequestedQuantityNode>(
   "REQUESTED_ORDER.QUANTITY");
  register_parameterless.template operator()<RequestedNotionalNode>(
   "REQUESTED_ORDER.NOTIONAL");
  register_parameterless.template operator()<RequestedCostNode>(
   "REQUESTED_ORDER.COST");
  register_parameterless.template operator()<EstimatedEntryFeeNode>(
   "REQUESTED_ORDER.ESTIMATED_ENTRY_FEE");
  register_parameterless.template operator()<EstimatedOneRExitFeeNode>(
   "REQUESTED_ORDER.ESTIMATED_1R_EXIT_FEE");
  register_parameterless.template operator()<RequestedOrderRiskDistanceNode>(
   "REQUESTED_ORDER.RISK_DISTANCE");
  register_parameterless.template operator()<RequestedPriceRiskNode>(
   "REQUESTED_ORDER.PRICE_RISK");
  register_parameterless.template operator()<RequestedRiskWithFeesNode>(
   "REQUESTED_ORDER.RISK_WITH_FEES");
  register_parameterless.template operator()<FrozenUnitQuantityNode>(
   "REQUESTED_ORDER.FROZEN_UNIT_QUANTITY");

  parser.register_node_parser(
   "OPERATOR.LOOKBACK",
   [](const Parser& parser, const Node& node) -> jsoncons::ojson {
     const auto* lookback = node_cast<LookbackNode<Context>>(node);
     return lookback ? jsoncons::ojson::object{{"period", lookback->period()},
                                               {"source",
                                                parser.serialize_node(
                                                 lookback->source())}}
                     : jsoncons::ojson::null();
   },
   [](Parser::Parser parser, const jsoncons::ojson& params) -> Node {
     return LookbackNode<Context>{params.contains("source")
                                   ? parser.parse_node(params.at("source"))
                                   : Node{CloseNode{}},
                                  params.at("period").as<std::size_t>()};
   });

  const auto register_binary = [&]<typename TNode>(const char* name,
                                                   const char* first,
                                                   const char* second) {
    parser.register_node_parser(
     name,
     [first, second](const Parser& parser,
                     const Node& node) -> jsoncons::ojson {
       const auto* binary = node_cast<TNode>(node);
       return binary ? jsoncons::ojson::object{{first,
                                                parser.serialize_node(
                                                 binary->operand1())},
                                               {second,
                                                parser.serialize_node(
                                                 binary->operand2())}}
                     : jsoncons::ojson::null();
     },
     [first, second](Parser::Parser parser,
                     const jsoncons::ojson& params) -> Node {
       return TNode{parser.parse_node(params.at(first)),
                    parser.parse_node(params.at(second))};
     });
  };
  register_binary.template operator()<BinaryOperatorNode<std::plus<>, Context>>(
   "OPERATOR.ADD", "augend", "addend");
  register_binary
   .template operator()<BinaryOperatorNode<std::minus<>, Context>>(
    "OPERATOR.SUBTRACT", "minuend", "subtrahend");
  register_binary
   .template operator()<BinaryOperatorNode<std::multiplies<>, Context>>(
    "OPERATOR.MULTIPLY", "multiplicand", "multiplier");
  register_binary
   .template operator()<BinaryOperatorNode<std::divides<>, Context>>(
    "OPERATOR.DIVIDE", "dividend", "divisor");
  register_binary
   .template operator()<BinaryOperatorNode<AbsoluteDifference<>, Context>>(
    "OPERATOR.ABS_DIFF", "left", "right");
  register_binary.template operator()<BinaryOperatorNode<Maximum<>, Context>>(
   "OPERATOR.MAX", "left", "right");
  register_binary.template operator()<BinaryOperatorNode<Minimum<>, Context>>(
   "OPERATOR.MIN", "left", "right");

  const auto register_unary = [&]<typename TNode>(const char* name) {
    parser.register_node_parser(
     name,
     [](const Parser& parser, const Node& node) -> jsoncons::ojson {
       const auto* unary = node_cast<TNode>(node);
       return unary ? jsoncons::ojson::object{{"operand",
                                               parser.serialize_node(
                                                unary->operand())}}
                    : jsoncons::ojson::null();
     },
     [](Parser::Parser parser, const jsoncons::ojson& params) -> Node {
       return TNode{parser.parse_node(params.at("operand"))};
     });
  };
  register_unary.template operator()<UnaryOperatorNode<std::negate<>, Context>>(
   "OPERATOR.NEGATE");
  register_unary.template operator()<UnaryOperatorNode<Absolute<>, Context>>(
   "OPERATOR.ABS");
  register_unary.template operator()<UnaryOperatorNode<SquareRoot<>, Context>>(
   "OPERATOR.SQRT");
  register_unary
   .template operator()<UnaryOperatorNode<PositivePart<>, Context>>(
    "OPERATOR.POSITIVE_PART");
  register_unary
   .template operator()<UnaryOperatorNode<NegativePart<>, Context>>(
    "OPERATOR.NEGATIVE_PART");
  return parser;
}

} // namespace pludux::backtest
