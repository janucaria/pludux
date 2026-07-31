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
import :risk_distance_node;
import :stop_target_price_node;
import :strategy_performance_node;

export namespace pludux::backtest {

class ConfigParser {
public:
  class Parser {
  public:
    Parser(ConfigParser& config_parser)
    : config_parser_{config_parser}
    {
    }

    auto parse_node(this Parser& self, const jsoncons::ojson& config)
     -> ErasedNode<ErasedSeriesMethodContext>
    {
      return self.config_parser_.parse_node(config);
    }

  private:
    ConfigParser& config_parser_;
  };

  friend Parser;

  using NodeSerialize =
   std::function<auto(const ConfigParser&,
                      const ErasedNode<ErasedSeriesMethodContext>&)
                  ->jsoncons::ojson>;

  using NodeDeserialize =
   std::function<auto(ConfigParser::Parser, const jsoncons::ojson&)
                  ->ErasedNode<ErasedSeriesMethodContext>>;

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
    self.node_parsers_.emplace(
     node_name, std::make_pair(node_serialize, node_deserialize));
  }

  auto parser(this ConfigParser& self) -> Parser
  {
    return Parser{self};
  }

  auto parse_node(this ConfigParser& self, const std::string& config_node_str)
   -> ErasedNode<ErasedSeriesMethodContext>
  {
    return self.parse_node(jsoncons::ojson::parse(config_node_str));
  }

  auto parse_node(this ConfigParser& self, const jsoncons::ojson& config_node)
   -> ErasedNode<ErasedSeriesMethodContext>
  {
    if(config_node.is_number()) {
      return ValueNode{config_node.as_double()};
    }

    if(config_node.is_bool()) {
      return config_node.as_bool()
              ? ErasedNode<ErasedSeriesMethodContext>{TrueNode{}}
              : ErasedNode<ErasedSeriesMethodContext>{FalseNode{}};
    }

    if(config_node.is_string()) {
      const auto node_name = config_node.as_string();
      const auto expanded_node = jsoncons::ojson::object{{"method", node_name}};
      return self.parse_node(expanded_node);
    }

    const auto node_name = config_node.at("method").as_string();

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

  auto serialize_node(this const ConfigParser& self,
                      const ErasedNode<ErasedSeriesMethodContext>& node)
   -> jsoncons::ojson
  {
    for(const auto& [node_name, node_parser] : self.node_parsers_) {
      const auto& [node_params_serialize, _] = node_parser;
      auto serialized_params_node = node_params_serialize(self, node);
      if(!serialized_params_node.is_null()) {
        auto serialized_node = jsoncons::ojson{};
        if(self.use_series_params_) {
          serialized_node["method"] = node_name;
          if(!serialized_params_node.empty()) {
            serialized_node["params"] = std::move(serialized_params_node);
          }
        } else {
          serialized_node = std::move(serialized_params_node);
          serialized_node["method"] = node_name;
        }

        return serialized_node;
      }
    }

    return jsoncons::ojson::null();
  }

private:
  std::unordered_map<std::string, std::pair<NodeSerialize, NodeDeserialize>>
   node_parsers_;

  bool use_series_params_{true};
};

auto make_default_registered_config_parser() -> ConfigParser;

auto parse_execution_filter_node(const jsoncons::ojson& config)
 -> ErasedNode<ExecutionFilterMethodContext>;

auto serialize_execution_filter_node(
 const ErasedNode<ExecutionFilterMethodContext>& node) -> jsoncons::ojson;

} // namespace pludux::backtest

namespace pludux::backtest {

template<typename T>
static auto get_param_or(const jsoncons::ojson& parameters,
                         const std::string& key,
                         const T& default_value) -> T
{
  return parameters.contains(key) ? parameters.at(key).as<T>() : default_value;
}

static auto parse_node_from_param_or(
 ConfigParser::Parser config_parser,
 const jsoncons::ojson& parameters,
 const std::string& key,
 const ErasedNode<ErasedSeriesMethodContext>& default_value)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  if(!parameters.contains(key)) {
    return default_value;
  }

  return config_parser.parse_node(parameters.at(key));
}

template<typename TNode>
static auto serialize_node_as(const ErasedNode<ErasedSeriesMethodContext>& node)
 -> const TNode*
{
  return node_cast<TNode>(node);
}

template<typename TNode, std::size_t default_period>
static auto parse_ta_with_erased_period_node(ConfigParser::Parser config_parser,
                                             const jsoncons::ojson& parameters)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  auto period = parse_node_from_param_or(
   config_parser,
   parameters,
   "period",
   NumericInputNode{"Period",
                    NumericInputNode::ValueRepresentation::UnsignedInteger,
                    static_cast<double>(default_period)});

  const auto source =
   parse_node_from_param_or(config_parser, parameters, "source", CloseNode{});

  return TNode{source, period};
}

template<typename TNode>
static auto serialize_ta_with_erased_period_node(
 const ConfigParser& config_parser,
 const ErasedNode<ErasedSeriesMethodContext>& node) -> jsoncons::ojson
{
  const auto ta_node = serialize_node_as<TNode>(node);
  if(!ta_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node["period"] = config_parser.serialize_node(ta_node->period());
  serialized_node["source"] = config_parser.serialize_node(ta_node->source());
  return serialized_node;
}

template<typename TNode>
static auto
serialize_ohlcv_node(const ConfigParser&,
                     const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  return node_cast<TNode>(node) ? jsoncons::ojson{} : jsoncons::ojson::null();
}

template<typename TNode>
static auto parse_ohlcv_node(ConfigParser::Parser, const jsoncons::ojson&)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  return TNode{};
}

static auto
serialize_value_node(const ConfigParser&,
                     const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  const auto value_node = node_cast<ValueNode>(node);
  if(!value_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node["value"] = value_node->value();
  return serialized_node;
}

static auto parse_value_node(ConfigParser::Parser,
                             const jsoncons::ojson& params)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  return ValueNode{params.at("value").as_double()};
}

static auto
serialize_data_node(const ConfigParser&,
                    const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  const auto data_node = node_cast<DataNode>(node);
  if(!data_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node["field"] = data_node->field();
  return serialized_node;
}

static auto parse_data_node(ConfigParser::Parser, const jsoncons::ojson& params)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  return DataNode{params.at("field").as_string()};
}

static auto
serialize_equity_node(const ConfigParser&,
                      const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  return node_cast<EquityNode>(node) ? jsoncons::ojson{}
                                     : jsoncons::ojson::null();
}

static auto parse_equity_node(ConfigParser::Parser, const jsoncons::ojson&)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  return EquityNode{};
}

static auto
serialize_equity_percent_node(const ConfigParser&,
                              const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  return node_cast<EquityPercentNode>(node) ? jsoncons::ojson{}
                                            : jsoncons::ojson::null();
}

static auto parse_equity_percent_node(ConfigParser::Parser,
                                      const jsoncons::ojson&)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  return EquityPercentNode{};
}

static auto
serialize_drawdown_node(const ConfigParser&,
                        const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  return node_cast<DrawdownNode>(node) ? jsoncons::ojson{}
                                       : jsoncons::ojson::null();
}

static auto parse_drawdown_node(ConfigParser::Parser, const jsoncons::ojson&)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  return DrawdownNode{};
}

static auto parse_ma_node_type(const std::string& ma_type_str,
                               MaNodeType fallback = MaNodeType::Rma)
 -> MaNodeType
{
  if(ma_type_str == "SMA") {
    return MaNodeType::Sma;
  }
  if(ma_type_str == "EMA") {
    return MaNodeType::Ema;
  }
  if(ma_type_str == "WMA") {
    return MaNodeType::Wma;
  }
  if(ma_type_str == "HMA") {
    return MaNodeType::Hma;
  }
  if(ma_type_str == "RMA") {
    return MaNodeType::Rma;
  }

  return fallback;
}

static auto serialize_ma_node_type(MaNodeType ma_type,
                                   std::string fallback = "RMA") -> std::string
{
  switch(ma_type) {
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

static auto parse_atr_node(ConfigParser::Parser config_parser,
                           const jsoncons::ojson& params)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  auto atr_node = AtrNode{parse_node_from_param_or(
   config_parser,
   params,
   "period",
   NumericInputNode{
    "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 14.0})};

  if(params.contains("maSmoothingType")) {
    atr_node.ma_smoothing_type(parse_ma_node_type(
     params.at("maSmoothingType").as_string(), MaNodeType::Rma));
  }

  return atr_node;
}

static auto
serialize_atr_node(const ConfigParser& config_parser,
                   const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  const auto atr_node = node_cast<AtrNode>(node);
  if(!atr_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node["period"] = config_parser.serialize_node(atr_node->period());
  serialized_node["maSmoothingType"] =
   serialize_ma_node_type(atr_node->ma_smoothing_type());
  return serialized_node;
}

template<typename TNode>
static auto parse_stop_target_value_node(ConfigParser::Parser config_parser,
                                         const jsoncons::ojson& params,
                                         const std::string& key,
                                         double fallback)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  return TNode{
   parse_node_from_param_or(config_parser, params, key, ValueNode{fallback})};
}

template<typename TNode>
static auto serialize_stop_target_value_node(
 const ConfigParser& config_parser,
 const ErasedNode<ErasedSeriesMethodContext>& node,
 const std::string& key) -> jsoncons::ojson
{
  const auto value_node = node_cast<TNode>(node);
  if(!value_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node[key] = config_parser.serialize_node(value_node->value());
  return serialized_node;
}

template<typename TNode>
static auto parse_stop_target_atr_node(ConfigParser::Parser config_parser,
                                       const jsoncons::ojson& params)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  auto period =
   parse_node_from_param_or(config_parser, params, "period", ValueNode{14.0});
  auto multiplier = parse_node_from_param_or(
   config_parser, params, "multiplier", ValueNode{2.0});
  const auto ma_smoothing_type = parse_ma_node_type(
   get_param_or<std::string>(params, "maSmoothingType", "RMA"),
   MaNodeType::Rma);

  return TNode{period, multiplier, ma_smoothing_type};
}

template<typename TNode>
static auto serialize_stop_target_atr_node(
 const ConfigParser& config_parser,
 const ErasedNode<ErasedSeriesMethodContext>& node) -> jsoncons::ojson
{
  const auto atr_node = node_cast<TNode>(node);
  if(!atr_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node["period"] = config_parser.serialize_node(atr_node->period());
  serialized_node["multiplier"] =
   config_parser.serialize_node(atr_node->multiplier());
  serialized_node["maSmoothingType"] =
   serialize_ma_node_type(atr_node->ma_smoothing_type());
  return serialized_node;
}

static auto parse_kc_band_node_type(const std::string& band_type_str)
 -> KcBandNodeType
{
  if(band_type_str == "ATR") {
    return KcBandNodeType::Atr;
  }
  if(band_type_str == "TR") {
    return KcBandNodeType::Tr;
  }
  if(band_type_str == "Range") {
    return KcBandNodeType::RangeHighLow;
  }

  return KcBandNodeType::Atr;
}

static auto serialize_kc_band_node_type(KcBandNodeType band_type) -> std::string
{
  switch(band_type) {
  case KcBandNodeType::Atr:
    return "ATR";
  case KcBandNodeType::Tr:
    return "TR";
  case KcBandNodeType::RangeHighLow:
    return "Range";
  }

  return "ATR";
}

static auto serialize_kc_node(const ConfigParser& config_parser,
                              const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  const auto kc_node = node_cast<KcNode>(node);
  if(!kc_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node["maMethodType"] =
   serialize_ma_node_type(kc_node->ma_node_type(), "EMA");
  serialized_node["period"] = config_parser.serialize_node(kc_node->period());
  serialized_node["maSource"] = config_parser.serialize_node(kc_node->source());
  serialized_node["bandMethodType"] =
   serialize_kc_band_node_type(kc_node->band_node_type());
  serialized_node["bandAtrPeriod"] =
   config_parser.serialize_node(kc_node->band_atr_period());
  serialized_node["multiplier"] =
   config_parser.serialize_node(kc_node->multiplier());
  return serialized_node;
}

static auto parse_kc_node(ConfigParser::Parser config_parser,
                          const jsoncons::ojson& params)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  const auto ma_node_type = parse_ma_node_type(
   get_param_or<std::string>(params, "maMethodType", "EMA"), MaNodeType::Ema);
  const auto period = parse_node_from_param_or(
   config_parser,
   params,
   "period",
   NumericInputNode{
    "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 20.0});
  const auto ma_source =
   parse_node_from_param_or(config_parser, params, "maSource", CloseNode{});

  const auto band_node_type = parse_kc_band_node_type(
   get_param_or<std::string>(params, "bandMethodType", "ATR"));
  const auto band_atr_period = parse_node_from_param_or(
   config_parser,
   params,
   "bandAtrPeriod",
   NumericInputNode{"Band ATR Period",
                    NumericInputNode::ValueRepresentation::UnsignedInteger,
                    14.0});

  const auto multiplier = parse_node_from_param_or(
   config_parser,
   params,
   "multiplier",
   NumericInputNode{
    "Multiplier", NumericInputNode::ValueRepresentation::Decimal, 1.5});

  return KcNode{ma_source,
                period,
                multiplier,
                band_atr_period,
                band_node_type,
                ma_node_type};
}

template<typename TNode>
static auto parse_binary_operator_node(ConfigParser::Parser config_parser,
                                       const jsoncons::ojson& params,
                                       const std::string& first_operand_key,
                                       const std::string& second_operand_key)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  const auto first_operand =
   config_parser.parse_node(params.at(first_operand_key));
  const auto second_operand =
   config_parser.parse_node(params.at(second_operand_key));

  return TNode{first_operand, second_operand};
}

template<typename TNode>
static auto serialize_binary_operator_node(
 const ConfigParser& config_parser,
 const ErasedNode<ErasedSeriesMethodContext>& node,
 const std::string& first_operand_key,
 const std::string& second_operand_key) -> jsoncons::ojson
{
  const auto binary_operator_node = node_cast<TNode>(node);
  if(!binary_operator_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node[first_operand_key] =
   config_parser.serialize_node(binary_operator_node->operand1());
  serialized_node[second_operand_key] =
   config_parser.serialize_node(binary_operator_node->operand2());
  return serialized_node;
}

template<typename TNode>
static auto parse_unary_operator_node(ConfigParser::Parser config_parser,
                                      const jsoncons::ojson& params,
                                      const std::string& operand_key)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  return TNode{config_parser.parse_node(params.at(operand_key))};
}

template<typename TNode>
static auto
serialize_unary_operator_node(const ConfigParser& config_parser,
                              const ErasedNode<ErasedSeriesMethodContext>& node,
                              const std::string& operand_key) -> jsoncons::ojson
{
  const auto unary_operator_node = node_cast<TNode>(node);
  if(!unary_operator_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node[operand_key] =
   config_parser.serialize_node(unary_operator_node->operand());
  return serialized_node;
}

template<typename TNode>
static auto parse_binary_logical_node(ConfigParser::Parser config_parser,
                                      const jsoncons::ojson& params,
                                      const std::string& first_operand_key,
                                      const std::string& second_operand_key)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  const auto first_operand =
   config_parser.parse_node(params.at(first_operand_key));
  const auto second_operand =
   config_parser.parse_node(params.at(second_operand_key));

  return TNode{first_operand, second_operand};
}

template<typename TNode>
static auto
serialize_binary_logical_node(const ConfigParser& config_parser,
                              const ErasedNode<ErasedSeriesMethodContext>& node,
                              const std::string& first_operand_key,
                              const std::string& second_operand_key)
 -> jsoncons::ojson
{
  const auto binary_logical_node = node_cast<TNode>(node);
  if(!binary_logical_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node[first_operand_key] =
   config_parser.serialize_node(binary_logical_node->first_condition());
  serialized_node[second_operand_key] =
   config_parser.serialize_node(binary_logical_node->second_condition());
  return serialized_node;
}

template<typename TNode>
static auto parse_unary_logical_node(ConfigParser::Parser config_parser,
                                     const jsoncons::ojson& params,
                                     const std::string& operand_key)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  return TNode{config_parser.parse_node(params.at(operand_key))};
}

template<typename TNode>
static auto
serialize_unary_logical_node(const ConfigParser& config_parser,
                             const ErasedNode<ErasedSeriesMethodContext>& node,
                             const std::string& operand_key) -> jsoncons::ojson
{
  const auto unary_logical_node = node_cast<TNode>(node);
  if(!unary_logical_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node[operand_key] =
   config_parser.serialize_node(unary_logical_node->other_condition());
  return serialized_node;
}

template<typename TNode>
static auto parse_comparison_node(ConfigParser::Parser config_parser,
                                  const jsoncons::ojson& params)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  auto target = config_parser.parse_node(params.at("target"));
  auto threshold = config_parser.parse_node(params.at("threshold"));
  return TNode{target, threshold};
}

template<typename TNode>
static auto
serialize_comparison_node(const ConfigParser& config_parser,
                          const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  const auto comparison_node = node_cast<TNode>(node);
  if(!comparison_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node["target"] =
   config_parser.serialize_node(comparison_node->target());
  serialized_node["threshold"] =
   config_parser.serialize_node(comparison_node->threshold());
  return serialized_node;
}

static auto parse_all_of_node(ConfigParser::Parser config_parser,
                              const jsoncons::ojson& params)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  if(!params.contains("items")) {
    throw std::invalid_argument{"ALL_OF: 'items' is not found"};
  }

  const auto& items = params.at("items");
  if(!items.is_array()) {
    throw std::invalid_argument{"ALL_OF: 'items' must be an array"};
  }

  auto conditions = std::vector<ErasedNode<ErasedSeriesMethodContext>>{};
  conditions.reserve(items.size());
  for(const auto& item : items.array_range()) {
    conditions.push_back(config_parser.parse_node(item));
  }
  return AllOfNode{std::move(conditions)};
}

static auto
serialize_all_of_node(const ConfigParser& config_parser,
                      const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  const auto all_of_node = node_cast<AllOfNode>(node);
  if(!all_of_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  auto conditions = jsoncons::ojson::array();
  for(const auto& condition : all_of_node->conditions()) {
    conditions.push_back(config_parser.serialize_node(condition));
  }
  serialized_node["items"] = conditions;
  return serialized_node;
}

static auto parse_any_of_node(ConfigParser::Parser config_parser,
                              const jsoncons::ojson& params)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  if(!params.contains("items")) {
    throw std::invalid_argument{"ANY_OF: 'items' is not found"};
  }

  const auto& items = params.at("items");
  if(!items.is_array()) {
    throw std::invalid_argument{"ANY_OF: 'items' must be an array"};
  }

  auto conditions = std::vector<ErasedNode<ErasedSeriesMethodContext>>{};
  conditions.reserve(items.size());
  for(const auto& item : items.array_range()) {
    conditions.push_back(config_parser.parse_node(item));
  }
  return AnyOfNode{std::move(conditions)};
}

static auto
serialize_any_of_node(const ConfigParser& config_parser,
                      const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  const auto any_of_node = node_cast<AnyOfNode>(node);
  if(!any_of_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  auto conditions = jsoncons::ojson::array();
  for(const auto& condition : any_of_node->conditions()) {
    conditions.push_back(config_parser.serialize_node(condition));
  }
  serialized_node["items"] = conditions;
  return serialized_node;
}

static auto parse_crossunder_node(ConfigParser::Parser config_parser,
                                  const jsoncons::ojson& params)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  auto signal = config_parser.parse_node(params.at("value"));
  auto reference = config_parser.parse_node(params.at("baseline"));
  return CrossunderNode{signal, reference};
}

static auto
serialize_crossunder_node(const ConfigParser& config_parser,
                          const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  const auto crossunder_node = node_cast<CrossunderNode>(node);
  if(!crossunder_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node["value"] =
   config_parser.serialize_node(crossunder_node->source());
  serialized_node["baseline"] =
   config_parser.serialize_node(crossunder_node->reference());
  return serialized_node;
}

static auto parse_crossover_node(ConfigParser::Parser config_parser,
                                 const jsoncons::ojson& params)
 -> ErasedNode<ErasedSeriesMethodContext>
{
  auto signal = config_parser.parse_node(params.at("value"));
  auto reference = config_parser.parse_node(params.at("baseline"));
  return CrossoverNode{signal, reference};
}

static auto
serialize_crossover_node(const ConfigParser& config_parser,
                         const ErasedNode<ErasedSeriesMethodContext>& node)
 -> jsoncons::ojson
{
  const auto crossover_node = node_cast<CrossoverNode>(node);
  if(!crossover_node) {
    return jsoncons::ojson::null();
  }

  auto serialized_node = jsoncons::ojson{};
  serialized_node["value"] =
   config_parser.serialize_node(crossover_node->source());
  serialized_node["baseline"] =
   config_parser.serialize_node(crossover_node->reference());
  return serialized_node;
}

static auto
serialize_boolean_node(const ErasedNode<ErasedSeriesMethodContext>& node,
                       bool expected) -> jsoncons::ojson
{
  if(expected && node_cast<TrueNode>(node)) {
    return jsoncons::ojson{};
  }
  if(!expected && node_cast<FalseNode>(node)) {
    return jsoncons::ojson{};
  }
  return jsoncons::ojson::null();
}

enum class ExecutionFilterNodeKind { Scalar, Boolean };

struct ParsedExecutionFilterNode {
  ErasedNode<ExecutionFilterMethodContext> node;
  ExecutionFilterNodeKind kind;
};

static auto parse_execution_filter_child(const jsoncons::ojson& config)
 -> ParsedExecutionFilterNode
{
  using Context = ExecutionFilterMethodContext;

  if(config.is_bool()) {
    return {config.as_bool() ? ErasedNode<Context>{TrueNode{}}
                             : ErasedNode<Context>{FalseNode{}},
            ExecutionFilterNodeKind::Boolean};
  }
  if(config.is_number()) {
    return {ErasedNode<Context>{ValueNode{config.as_double()}},
            ExecutionFilterNodeKind::Scalar};
  }
  if(!config.is_object() || !config.contains("method")) {
    throw std::invalid_argument{
     "ExecutionFilter node must be a boolean, number, or method object"};
  }

  const auto method = config.at("method").as_string();
  if(method == "ALWAYS") {
    return {ErasedNode<Context>{TrueNode{}}, ExecutionFilterNodeKind::Boolean};
  }
  if(method == "NEVER") {
    return {ErasedNode<Context>{FalseNode{}}, ExecutionFilterNodeKind::Boolean};
  }
  if(method == "VALUE") {
    return {ErasedNode<Context>{ValueNode{config.at("value").as_double()}},
            ExecutionFilterNodeKind::Scalar};
  }
  if(method == "EQUITY") {
    return {ErasedNode<Context>{EquityNode{}}, ExecutionFilterNodeKind::Scalar};
  }
  if(method == "EQUITY_PERCENT") {
    return {ErasedNode<Context>{EquityPercentNode{}},
            ExecutionFilterNodeKind::Scalar};
  }
  if(method == "DRAWDOWN") {
    return {ErasedNode<Context>{DrawdownNode{}},
            ExecutionFilterNodeKind::Scalar};
  }
  if(method == "STRATEGY_PERFORMANCE") {
    const auto metric_value = config.at("metric").as<int>();
    if(metric_value <
        static_cast<int>(StrategyPerformanceMetric::LifetimeCount) ||
       metric_value >
        static_cast<int>(
         StrategyPerformanceMetric::BayesianLosingPayoffUpper95)) {
      throw std::invalid_argument{
       "ExecutionFilter strategy-performance metric is invalid"};
    }
    return {ErasedNode<Context>{StrategyPerformanceNode{
             static_cast<StrategyPerformanceMetric>(metric_value)}},
            ExecutionFilterNodeKind::Scalar};
  }

  const auto parse_scalar = [&](const char* key) {
    auto parsed = parse_execution_filter_child(config.at(key));
    if(parsed.kind != ExecutionFilterNodeKind::Scalar) {
      throw std::invalid_argument{
       std::format("ExecutionFilter '{}' must be scalar", key)};
    }
    return parsed.node;
  };
  const auto parse_boolean = [&](const char* key) {
    auto parsed = parse_execution_filter_child(config.at(key));
    if(parsed.kind != ExecutionFilterNodeKind::Boolean) {
      throw std::invalid_argument{
       std::format("ExecutionFilter '{}' must be boolean", key)};
    }
    return parsed.node;
  };

  if(method == "GREATER_THAN") {
    return {ErasedNode<Context>{GreaterThanNode<Context>{
             parse_scalar("target"), parse_scalar("threshold")}},
            ExecutionFilterNodeKind::Boolean};
  }
  if(method == "GREATER_EQUAL") {
    return {ErasedNode<Context>{GreaterEqualNode<Context>{
             parse_scalar("target"), parse_scalar("threshold")}},
            ExecutionFilterNodeKind::Boolean};
  }
  if(method == "LESS_THAN") {
    return {ErasedNode<Context>{LessThanNode<Context>{
             parse_scalar("target"), parse_scalar("threshold")}},
            ExecutionFilterNodeKind::Boolean};
  }
  if(method == "LESS_EQUAL") {
    return {ErasedNode<Context>{LessEqualNode<Context>{
             parse_scalar("target"), parse_scalar("threshold")}},
            ExecutionFilterNodeKind::Boolean};
  }
  if(method == "EQUAL") {
    return {ErasedNode<Context>{EqualNode<Context>{parse_scalar("target"),
                                                   parse_scalar("threshold")}},
            ExecutionFilterNodeKind::Boolean};
  }
  if(method == "NOT_EQUAL") {
    return {ErasedNode<Context>{NotEqualNode<Context>{
             parse_scalar("target"), parse_scalar("threshold")}},
            ExecutionFilterNodeKind::Boolean};
  }
  if(method == "AND") {
    return {
     ErasedNode<Context>{LogicalAndNode<Context>{
      parse_boolean("firstCondition"), parse_boolean("secondCondition")}},
     ExecutionFilterNodeKind::Boolean};
  }
  if(method == "OR") {
    return {
     ErasedNode<Context>{LogicalOrNode<Context>{
      parse_boolean("firstCondition"), parse_boolean("secondCondition")}},
     ExecutionFilterNodeKind::Boolean};
  }
  if(method == "XOR") {
    return {
     ErasedNode<Context>{LogicalXorNode<Context>{
      parse_boolean("firstCondition"), parse_boolean("secondCondition")}},
     ExecutionFilterNodeKind::Boolean};
  }
  if(method == "NOT") {
    return {
     ErasedNode<Context>{LogicalNotNode<Context>{parse_boolean("condition")}},
     ExecutionFilterNodeKind::Boolean};
  }

  throw std::invalid_argument{
   std::format("Node '{}' is not allowed in ExecutionFilter", method)};
}

static auto serialize_execution_filter_child(
 const ErasedNode<ExecutionFilterMethodContext>& node)
 -> std::pair<jsoncons::ojson, ExecutionFilterNodeKind>
{
  using Context = ExecutionFilterMethodContext;
  const auto object = [](std::string method) -> jsoncons::ojson {
    auto config = jsoncons::ojson{};
    config["method"] = std::move(method);
    return config;
  };
  if(node_cast<TrueNode>(node)) {
    return {object("ALWAYS"), ExecutionFilterNodeKind::Boolean};
  }
  if(node_cast<FalseNode>(node)) {
    return {object("NEVER"), ExecutionFilterNodeKind::Boolean};
  }
  if(const auto* value = node_cast<ValueNode>(node)) {
    auto config = object("VALUE");
    config["value"] = value->value();
    return {std::move(config), ExecutionFilterNodeKind::Scalar};
  }
  if(node_cast<EquityNode>(node)) {
    return {object("EQUITY"), ExecutionFilterNodeKind::Scalar};
  }
  if(node_cast<EquityPercentNode>(node)) {
    return {object("EQUITY_PERCENT"), ExecutionFilterNodeKind::Scalar};
  }
  if(node_cast<DrawdownNode>(node)) {
    return {object("DRAWDOWN"), ExecutionFilterNodeKind::Scalar};
  }
  if(const auto* performance = node_cast<StrategyPerformanceNode>(node)) {
    auto config = object("STRATEGY_PERFORMANCE");
    config["metric"] = static_cast<int>(performance->metric());
    return {std::move(config), ExecutionFilterNodeKind::Scalar};
  }

  const auto serialize_comparison = [&](const auto* comparison,
                                        std::string method)
   -> std::optional<std::pair<jsoncons::ojson, ExecutionFilterNodeKind>> {
    if(!comparison) {
      return std::nullopt;
    }
    auto config = object(std::move(method));
    config["target"] =
     serialize_execution_filter_child(comparison->target()).first;
    config["threshold"] =
     serialize_execution_filter_child(comparison->threshold()).first;
    return std::pair{std::move(config), ExecutionFilterNodeKind::Boolean};
  };
#define PLUDUX_SERIALIZE_FILTER_COMPARISON(Type, Name)              \
  if(auto result =                                                  \
      serialize_comparison(node_cast<Type<Context>>(node), Name)) { \
    return *std::move(result);                                      \
  }
  PLUDUX_SERIALIZE_FILTER_COMPARISON(GreaterThanNode, "GREATER_THAN")
  PLUDUX_SERIALIZE_FILTER_COMPARISON(GreaterEqualNode, "GREATER_EQUAL")
  PLUDUX_SERIALIZE_FILTER_COMPARISON(LessThanNode, "LESS_THAN")
  PLUDUX_SERIALIZE_FILTER_COMPARISON(LessEqualNode, "LESS_EQUAL")
  PLUDUX_SERIALIZE_FILTER_COMPARISON(EqualNode, "EQUAL")
  PLUDUX_SERIALIZE_FILTER_COMPARISON(NotEqualNode, "NOT_EQUAL")
#undef PLUDUX_SERIALIZE_FILTER_COMPARISON

  const auto serialize_binary = [&](const auto* logical, std::string method)
   -> std::optional<std::pair<jsoncons::ojson, ExecutionFilterNodeKind>> {
    if(!logical) {
      return std::nullopt;
    }
    auto config = object(std::move(method));
    config["firstCondition"] =
     serialize_execution_filter_child(logical->first_condition()).first;
    config["secondCondition"] =
     serialize_execution_filter_child(logical->second_condition()).first;
    return std::pair{std::move(config), ExecutionFilterNodeKind::Boolean};
  };
#define PLUDUX_SERIALIZE_FILTER_BINARY(Type, Name)                           \
  if(auto result = serialize_binary(node_cast<Type<Context>>(node), Name)) { \
    return *std::move(result);                                               \
  }
  PLUDUX_SERIALIZE_FILTER_BINARY(LogicalAndNode, "AND")
  PLUDUX_SERIALIZE_FILTER_BINARY(LogicalOrNode, "OR")
  PLUDUX_SERIALIZE_FILTER_BINARY(LogicalXorNode, "XOR")
#undef PLUDUX_SERIALIZE_FILTER_BINARY

  if(const auto* logical = node_cast<LogicalNotNode<Context>>(node)) {
    auto config = object("NOT");
    config["condition"] =
     serialize_execution_filter_child(logical->other_condition()).first;
    return {std::move(config), ExecutionFilterNodeKind::Boolean};
  }

  throw std::invalid_argument{"ExecutionFilter contains an unsupported node"};
}

auto parse_execution_filter_node(const jsoncons::ojson& config)
 -> ErasedNode<ExecutionFilterMethodContext>
{
  auto parsed = parse_execution_filter_child(config);
  if(parsed.kind != ExecutionFilterNodeKind::Boolean) {
    throw std::invalid_argument{"ExecutionFilter root must be boolean"};
  }
  return std::move(parsed.node);
}

auto serialize_execution_filter_node(
 const ErasedNode<ExecutionFilterMethodContext>& node) -> jsoncons::ojson
{
  auto [config, kind] = serialize_execution_filter_child(node);
  if(kind != ExecutionFilterNodeKind::Boolean) {
    throw std::invalid_argument{"ExecutionFilter root must be boolean"};
  }
  return config;
}

auto make_default_registered_config_parser() -> ConfigParser
{
  auto config_parser = ConfigParser{};

  config_parser.register_node_parser(
   "VALUE", serialize_value_node, parse_value_node);

  config_parser.register_node_parser(
   "DATA", serialize_data_node, parse_data_node);

  config_parser.register_node_parser(
   "EQUITY", serialize_equity_node, parse_equity_node);

  config_parser.register_node_parser(
   "EQUITY_PERCENT", serialize_equity_percent_node, parse_equity_percent_node);

  config_parser.register_node_parser(
   "DRAWDOWN", serialize_drawdown_node, parse_drawdown_node);

  config_parser.register_node_parser(
   "OPEN", serialize_ohlcv_node<OpenNode>, parse_ohlcv_node<OpenNode>);
  config_parser.register_node_parser(
   "HIGH", serialize_ohlcv_node<HighNode>, parse_ohlcv_node<HighNode>);
  config_parser.register_node_parser(
   "LOW", serialize_ohlcv_node<LowNode>, parse_ohlcv_node<LowNode>);
  config_parser.register_node_parser(
   "CLOSE", serialize_ohlcv_node<CloseNode>, parse_ohlcv_node<CloseNode>);
  config_parser.register_node_parser(
   "VOLUME", serialize_ohlcv_node<VolumeNode>, parse_ohlcv_node<VolumeNode>);

  config_parser.register_node_parser(
   "CHANGE",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     const auto change_node = node_cast<ChangeNode>(node);
     if(!change_node) {
       return jsoncons::ojson::null();
     }
     auto serialized_node = jsoncons::ojson{};
     serialized_node["source"] =
      config_parser.serialize_node(change_node->source());
     return serialized_node;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     const auto source =
      parse_node_from_param_or(config_parser, params, "source", CloseNode{});
     return ErasedNode<ErasedSeriesMethodContext>{ChangeNode{source}};
   });

  config_parser.register_node_parser(
   "SMA",
   serialize_ta_with_erased_period_node<SmaNode>,
   parse_ta_with_erased_period_node<SmaNode, 20>);
  config_parser.register_node_parser(
   "EMA",
   serialize_ta_with_erased_period_node<EmaNode>,
   parse_ta_with_erased_period_node<EmaNode, 20>);
  config_parser.register_node_parser(
   "WMA",
   serialize_ta_with_erased_period_node<WmaNode>,
   parse_ta_with_erased_period_node<WmaNode, 20>);
  config_parser.register_node_parser(
   "RMA",
   serialize_ta_with_erased_period_node<RmaNode>,
   parse_ta_with_erased_period_node<RmaNode, 20>);
  config_parser.register_node_parser(
   "HMA",
   serialize_ta_with_erased_period_node<HmaNode>,
   parse_ta_with_erased_period_node<HmaNode, 20>);
  config_parser.register_node_parser(
   "RSI",
   serialize_ta_with_erased_period_node<RsiNode>,
   parse_ta_with_erased_period_node<RsiNode, 14>);
  config_parser.register_node_parser(
   "HIGHEST",
   serialize_ta_with_erased_period_node<HighestNode>,
   parse_ta_with_erased_period_node<HighestNode, 14>);
  config_parser.register_node_parser(
   "LOWEST",
   serialize_ta_with_erased_period_node<LowestNode>,
   parse_ta_with_erased_period_node<LowestNode, 14>);
  config_parser.register_node_parser(
   "ROC",
   serialize_ta_with_erased_period_node<RocNode>,
   parse_ta_with_erased_period_node<RocNode, 14>);

  config_parser.register_node_parser(
   "RVOL",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     const auto rvol_node = node_cast<RvolNode>(node);
     if(!rvol_node) {
       return jsoncons::ojson::null();
     }
     auto serialized_node = jsoncons::ojson{};
     serialized_node["period"] =
      config_parser.serialize_node(rvol_node->period());
     return serialized_node;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return ErasedNode<ErasedSeriesMethodContext>{
      RvolNode{parse_node_from_param_or(
       config_parser,
       params,
       "period",
       NumericInputNode{"Period",
                        NumericInputNode::ValueRepresentation::UnsignedInteger,
                        14.0})}};
   });

  config_parser.register_node_parser("ATR", serialize_atr_node, parse_atr_node);
  config_parser.register_node_parser(
   "TR", serialize_ohlcv_node<TrNode>, parse_ohlcv_node<TrNode>);
  config_parser.register_node_parser("KC", serialize_kc_node, parse_kc_node);

  config_parser.register_node_parser(
   "DC",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     const auto dc_node = node_cast<DonchianChannelNode>(node);
     if(!dc_node) {
       return jsoncons::ojson::null();
     }
     auto serialized_node = jsoncons::ojson{};
     serialized_node["period"] =
      config_parser.serialize_node(dc_node->period());
     return serialized_node;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return ErasedNode<ErasedSeriesMethodContext>{
      DonchianChannelNode{parse_node_from_param_or(
       config_parser,
       params,
       "period",
       NumericInputNode{"Period",
                        NumericInputNode::ValueRepresentation::UnsignedInteger,
                        20.0})}};
   });

  config_parser.register_node_parser(
   "SERIES",
   [](const ConfigParser&, const ErasedNode<ErasedSeriesMethodContext>& node) {
     const auto series_node = node_cast<SeriesNode>(node);
     if(!series_node) {
       return jsoncons::ojson::null();
     }
     auto serialized_node = jsoncons::ojson{};
     serialized_node["name"] = series_node->name();
     return serialized_node;
   },
   [](ConfigParser::Parser, const jsoncons::ojson& params) {
     const auto name = get_param_or<std::string>(params, "name", "");
     return ErasedNode<ErasedSeriesMethodContext>{SeriesNode{name}};
   });

  config_parser.register_node_parser(
   "LOOKBACK",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     const auto lookback_node = node_cast<LookbackNode>(node);
     if(!lookback_node) {
       return jsoncons::ojson::null();
     }
     auto serialized_node = jsoncons::ojson{};
     serialized_node["period"] = lookback_node->period();
     serialized_node["source"] =
      config_parser.serialize_node(lookback_node->source());
     return serialized_node;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     const auto period = params.at("period").as<std::size_t>();
     const auto source =
      parse_node_from_param_or(config_parser, params, "source", CloseNode{});
     return ErasedNode<ErasedSeriesMethodContext>{LookbackNode{source, period}};
   });

  config_parser.register_node_parser(
   "INPUT",
   [](const ConfigParser&, const ErasedNode<ErasedSeriesMethodContext>& node) {
     const auto input_node = node_cast<NumericInputNode>(node);
     if(!input_node) {
       return jsoncons::ojson::null();
     }
     auto serialized_node = jsoncons::ojson{};
     serialized_node["label"] = input_node->label();
     serialized_node["representation"] = [&]() -> std::string {
       switch(input_node->representation()) {
       case NumericInputNode::ValueRepresentation::SignedInteger:
         return "SignedInteger";
       case NumericInputNode::ValueRepresentation::UnsignedInteger:
         return "UnsignedInteger";
       case NumericInputNode::ValueRepresentation::Decimal:
       default:
         return "Decimal";
       }
     }();
     serialized_node["value"] = input_node->value();
     return serialized_node;
   },
   [](ConfigParser::Parser, const jsoncons::ojson& params) {
     const auto label = get_param_or<std::string>(params, "label", "");
     const auto representation_str =
      get_param_or<std::string>(params, "representation", "Decimal");
     const auto value = get_param_or<double>(params, "value", 0.0);
     const auto representation =
      representation_str == "SignedInteger"
       ? NumericInputNode::ValueRepresentation::SignedInteger
      : representation_str == "UnsignedInteger"
        ? NumericInputNode::ValueRepresentation::UnsignedInteger
        : NumericInputNode::ValueRepresentation::Decimal;
     return ErasedNode<ErasedSeriesMethodContext>{
      NumericInputNode{label, representation, value}};
   });

  config_parser.register_node_parser(
   "SELECT_OUTPUT",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     const auto select_output_node = node_cast<SelectOutputNode>(node);
     if(!select_output_node) {
       return jsoncons::ojson::null();
     }
     auto serialized_node = jsoncons::ojson{};
     serialized_node["output"] = [&]() -> std::string {
       switch(select_output_node->output()) {
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
       default:
         return "default";
       }
     }();
     serialized_node["source"] =
      config_parser.serialize_node(select_output_node->source());
     return serialized_node;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     const auto output_name =
      get_param_or<std::string>(params, "output", "default");
     const auto output = [&]() -> NodeOutput {
       if(output_name == "macd-line") {
         return NodeOutput::MacdLine;
       } else if(output_name == "signal-line") {
         return NodeOutput::SignalLine;
       } else if(output_name == "histogram") {
         return NodeOutput::Histogram;
       } else if(output_name == "k-percent") {
         return NodeOutput::KPercent;
       } else if(output_name == "d-percent") {
         return NodeOutput::DPercent;
       } else if(output_name == "middle-band") {
         return NodeOutput::MiddleBand;
       } else if(output_name == "upper-band") {
         return NodeOutput::UpperBand;
       } else if(output_name == "lower-band") {
         return NodeOutput::LowerBand;
       }
       return static_cast<NodeOutput>(-1);
     }();
     const auto source =
      parse_node_from_param_or(config_parser, params, "source", CloseNode{});
     return ErasedNode<ErasedSeriesMethodContext>{
      SelectOutputNode{source, output}};
   });

  config_parser.register_node_parser(
   "ABS_DIFF",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_binary_operator_node<AbsDiffNode>(
      config_parser, node, "minuend", "subtrahend");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_binary_operator_node<AbsDiffNode>(
      config_parser, params, "minuend", "subtrahend");
   });

  config_parser.register_node_parser(
   "BB",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     const auto bb_node = node_cast<BbNode>(node);
     if(!bb_node) {
       return jsoncons::ojson::null();
     }
     auto serialized_node = jsoncons::ojson{};
     serialized_node["maType"] =
      serialize_ma_node_type(bb_node->ma_node_type(), "SMA");
     serialized_node["maSource"] =
      config_parser.serialize_node(bb_node->source());
     serialized_node["period"] =
      config_parser.serialize_node(bb_node->period());
     serialized_node["stddev"] =
      config_parser.serialize_node(bb_node->stddev());
     return serialized_node;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     const auto ma_type = parse_ma_node_type(
      get_param_or<std::string>(params, "maType", "SMA"), MaNodeType::Sma);
     const auto ma_source =
      parse_node_from_param_or(config_parser, params, "maSource", CloseNode{});
     const auto period = parse_node_from_param_or(
      config_parser,
      params,
      "period",
      NumericInputNode{
       "Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 20.0});

     const auto stddev = parse_node_from_param_or(
      config_parser,
      params,
      "stddev",
      NumericInputNode{
       "StdDev", NumericInputNode::ValueRepresentation::Decimal, 2.0});

     return BbNode{ma_source, period, stddev, ma_type};
   });

  config_parser.register_node_parser(
   "MACD",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     const auto macd_node = node_cast<MacdNode>(node);
     if(!macd_node) {
       return jsoncons::ojson::null();
     }
     auto serialized_node = jsoncons::ojson{};
     serialized_node["fastPeriod"] =
      config_parser.serialize_node(macd_node->fast_period());
     serialized_node["slowPeriod"] =
      config_parser.serialize_node(macd_node->slow_period());
     serialized_node["signalPeriod"] =
      config_parser.serialize_node(macd_node->signal_period());
     serialized_node["source"] =
      config_parser.serialize_node(macd_node->source());
     return serialized_node;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     const auto fast_period = parse_node_from_param_or(
      config_parser,
      params,
      "fastPeriod",
      NumericInputNode{"Fast Period",
                       NumericInputNode::ValueRepresentation::UnsignedInteger,
                       12.0});
     const auto slow_period = parse_node_from_param_or(
      config_parser,
      params,
      "slowPeriod",
      NumericInputNode{"Slow Period",
                       NumericInputNode::ValueRepresentation::UnsignedInteger,
                       26.0});
     const auto signal_period = parse_node_from_param_or(
      config_parser,
      params,
      "signalPeriod",
      NumericInputNode{"Signal Period",
                       NumericInputNode::ValueRepresentation::UnsignedInteger,
                       9.0});
     const auto source =
      parse_node_from_param_or(config_parser, params, "source", CloseNode{});
     return ErasedNode<ErasedSeriesMethodContext>{
      MacdNode{source, fast_period, slow_period, signal_period}};
   });

  config_parser.register_node_parser(
   "STOCH",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     const auto stoch_node = node_cast<StochNode>(node);
     if(!stoch_node) {
       return jsoncons::ojson::null();
     }
     auto serialized_node = jsoncons::ojson{};
     serialized_node["kPeriod"] =
      config_parser.serialize_node(stoch_node->k_period());
     serialized_node["kSmooth"] =
      config_parser.serialize_node(stoch_node->k_smooth());
     serialized_node["dPeriod"] =
      config_parser.serialize_node(stoch_node->d_period());
     return serialized_node;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     const auto k_period = parse_node_from_param_or(
      config_parser,
      params,
      "kPeriod",
      NumericInputNode{"K Period",
                       NumericInputNode::ValueRepresentation::UnsignedInteger,
                       5.0});
     const auto k_smooth = parse_node_from_param_or(
      config_parser,
      params,
      "kSmooth",
      NumericInputNode{"K Smooth",
                       NumericInputNode::ValueRepresentation::UnsignedInteger,
                       3.0});
     const auto d_period = parse_node_from_param_or(
      config_parser,
      params,
      "dPeriod",
      NumericInputNode{"D Period",
                       NumericInputNode::ValueRepresentation::UnsignedInteger,
                       3.0});
     return ErasedNode<ErasedSeriesMethodContext>{
      StochNode{k_period, k_smooth, d_period}};
   });

  config_parser.register_node_parser(
   "STOCH_RSI",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     const auto stoch_rsi_node = node_cast<StochRsiNode>(node);
     if(!stoch_rsi_node) {
       return jsoncons::ojson::null();
     }
     auto serialized_node = jsoncons::ojson{};
     serialized_node["rsiSource"] =
      config_parser.serialize_node(stoch_rsi_node->rsi_source());
     serialized_node["rsiPeriod"] =
      config_parser.serialize_node(stoch_rsi_node->rsi_period());
     serialized_node["kPeriod"] =
      config_parser.serialize_node(stoch_rsi_node->k_period());
     serialized_node["kSmooth"] =
      config_parser.serialize_node(stoch_rsi_node->k_smooth());
     serialized_node["dPeriod"] =
      config_parser.serialize_node(stoch_rsi_node->d_period());
     return serialized_node;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     const auto rsi_source =
      parse_node_from_param_or(config_parser, params, "rsiSource", CloseNode{});
     const auto rsi_period = parse_node_from_param_or(
      config_parser,
      params,
      "rsiPeriod",
      NumericInputNode{"RSI Period",
                       NumericInputNode::ValueRepresentation::UnsignedInteger,
                       14.0});
     const auto k_period = parse_node_from_param_or(
      config_parser,
      params,
      "kPeriod",
      NumericInputNode{"K Period",
                       NumericInputNode::ValueRepresentation::UnsignedInteger,
                       5.0});
     const auto k_smooth = parse_node_from_param_or(
      config_parser,
      params,
      "kSmooth",
      NumericInputNode{"K Smooth",
                       NumericInputNode::ValueRepresentation::UnsignedInteger,
                       3.0});
     const auto d_period = parse_node_from_param_or(
      config_parser,
      params,
      "dPeriod",
      NumericInputNode{"D Period",
                       NumericInputNode::ValueRepresentation::UnsignedInteger,
                       3.0});
     return ErasedNode<ErasedSeriesMethodContext>{
      StochRsiNode{rsi_source, rsi_period, k_period, k_smooth, d_period}};
   });

  config_parser.register_node_parser(
   "ADD",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_binary_operator_node<AddNode>(
      config_parser, node, "augend", "addend");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_binary_operator_node<AddNode>(
      config_parser, params, "augend", "addend");
   });
  config_parser.register_node_parser(
   "SUBTRACT",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_binary_operator_node<SubtractNode>(
      config_parser, node, "minuend", "subtrahend");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_binary_operator_node<SubtractNode>(
      config_parser, params, "minuend", "subtrahend");
   });
  config_parser.register_node_parser(
   "MULTIPLY",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_binary_operator_node<MultiplyNode>(
      config_parser, node, "multiplicand", "multiplier");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_binary_operator_node<MultiplyNode>(
      config_parser, params, "multiplicand", "multiplier");
   });
  config_parser.register_node_parser(
   "DIVIDE",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_binary_operator_node<DivideNode>(
      config_parser, node, "dividend", "divisor");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_binary_operator_node<DivideNode>(
      config_parser, params, "dividend", "divisor");
   });
  config_parser.register_node_parser(
   "NEGATE",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_unary_operator_node<NegateNode>(
      config_parser, node, "operand");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_unary_operator_node<NegateNode>(
      config_parser, params, "operand");
   });

  config_parser.register_node_parser(
   "PERCENTAGE",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     const auto percentage_node = node_cast<PercentageNode>(node);
     if(!percentage_node) {
       return jsoncons::ojson::null();
     }
     auto serialized_node = jsoncons::ojson{};
     serialized_node["base"] =
      config_parser.serialize_node(percentage_node->base());
     serialized_node["percent"] = percentage_node->percent();
     return serialized_node;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     auto base =
      parse_node_from_param_or(config_parser, params, "base", CloseNode{});
     auto percent = get_param_or<double>(params, "percent", 100.0);
     return ErasedNode<ErasedSeriesMethodContext>{
      PercentageNode{base, percent}};
   });

  config_parser.register_node_parser(
   "SL_AMOUNT",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_stop_target_value_node<SlAmountNode>(
      config_parser, node, "amount");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_stop_target_value_node<SlAmountNode>(
      config_parser, params, "amount", 0.0);
   });

  config_parser.register_node_parser(
   "TP_AMOUNT",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_stop_target_value_node<TpAmountNode>(
      config_parser, node, "amount");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_stop_target_value_node<TpAmountNode>(
      config_parser, params, "amount", 0.0);
   });

  config_parser.register_node_parser(
   "SL_PERCENT",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_stop_target_value_node<SlPercentNode>(
      config_parser, node, "percent");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_stop_target_value_node<SlPercentNode>(
      config_parser, params, "percent", 0.0);
   });

  config_parser.register_node_parser(
   "TP_PERCENT",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_stop_target_value_node<TpPercentNode>(
      config_parser, node, "percent");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_stop_target_value_node<TpPercentNode>(
      config_parser, params, "percent", 0.0);
   });

  config_parser.register_node_parser("SL_ATR",
                                     serialize_stop_target_atr_node<SlAtrNode>,
                                     parse_stop_target_atr_node<SlAtrNode>);

  config_parser.register_node_parser("TP_ATR",
                                     serialize_stop_target_atr_node<TpAtrNode>,
                                     parse_stop_target_atr_node<TpAtrNode>);

  config_parser.register_node_parser(
   "R_DISTANCE_AMOUNT",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_stop_target_value_node<RiskDistanceAmountNode>(
      config_parser, node, "amount");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_stop_target_value_node<RiskDistanceAmountNode>(
      config_parser, params, "amount", 1.0);
   });

  config_parser.register_node_parser(
   "R_DISTANCE_PERCENTAGE",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_stop_target_value_node<RiskDistancePercentNode>(
      config_parser, node, "percentage");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_stop_target_value_node<RiskDistancePercentNode>(
      config_parser, params, "percentage", 1.0);
   });

  config_parser.register_node_parser(
   "R_DISTANCE_ATR",
   serialize_stop_target_atr_node<RiskDistanceAtrNode>,
   parse_stop_target_atr_node<RiskDistanceAtrNode>);

  config_parser.register_node_parser(
   "SL_1R", serialize_ohlcv_node<Sl1RNode>, parse_ohlcv_node<Sl1RNode>);

  config_parser.register_node_parser(
   "TP_R_MULTIPLE",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_stop_target_value_node<TpRMultipleNode>(
      config_parser, node, "multiple");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_stop_target_value_node<TpRMultipleNode>(
      config_parser, params, "multiple", 2.0);
   });

  config_parser.register_node_parser(
   "INITIAL_ENTRY_PRICE",
   serialize_ohlcv_node<InitialEntryPriceNode>,
   parse_ohlcv_node<InitialEntryPriceNode>);
  config_parser.register_node_parser("LATEST_ENTRY_PRICE",
                                     serialize_ohlcv_node<LatestEntryPriceNode>,
                                     parse_ohlcv_node<LatestEntryPriceNode>);
  config_parser.register_node_parser("AVERAGE_PRICE",
                                     serialize_ohlcv_node<AveragePriceNode>,
                                     parse_ohlcv_node<AveragePriceNode>);
  config_parser.register_node_parser(
   "STOP_TARGET_REF_PRICE",
   serialize_ohlcv_node<StopTargetRefPriceNode>,
   parse_ohlcv_node<StopTargetRefPriceNode>);
  config_parser.register_node_parser(
   "POSITION_DIRECTION",
   serialize_ohlcv_node<PositionDirectionNode>,
   parse_ohlcv_node<PositionDirectionNode>);

  config_parser.register_node_parser(
   "SQRT",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_unary_operator_node<SqrtNode>(
      config_parser, node, "operand");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_unary_operator_node<SqrtNode>(
      config_parser, params, "operand");
   });
  config_parser.register_node_parser(
   "STDDEV",
   serialize_ta_with_erased_period_node<StddevNode>,
   parse_ta_with_erased_period_node<StddevNode, 20>);

  config_parser.register_node_parser(
   "ALL_OF", serialize_all_of_node, parse_all_of_node);
  config_parser.register_node_parser(
   "ANY_OF", serialize_any_of_node, parse_any_of_node);
  config_parser.register_node_parser(
   "CROSSUNDER", serialize_crossunder_node, parse_crossunder_node);
  config_parser.register_node_parser(
   "CROSSOVER", serialize_crossover_node, parse_crossover_node);
  config_parser.register_node_parser(
   "GREATER_THAN",
   serialize_comparison_node<GreaterThanNode<ErasedSeriesMethodContext>>,
   parse_comparison_node<GreaterThanNode<ErasedSeriesMethodContext>>);
  config_parser.register_node_parser(
   "LESS_THAN",
   serialize_comparison_node<LessThanNode<ErasedSeriesMethodContext>>,
   parse_comparison_node<LessThanNode<ErasedSeriesMethodContext>>);
  config_parser.register_node_parser(
   "GREATER_EQUAL",
   serialize_comparison_node<GreaterEqualNode<ErasedSeriesMethodContext>>,
   parse_comparison_node<GreaterEqualNode<ErasedSeriesMethodContext>>);
  config_parser.register_node_parser(
   "LESS_EQUAL",
   serialize_comparison_node<LessEqualNode<ErasedSeriesMethodContext>>,
   parse_comparison_node<LessEqualNode<ErasedSeriesMethodContext>>);
  config_parser.register_node_parser(
   "EQUAL",
   serialize_comparison_node<EqualNode<ErasedSeriesMethodContext>>,
   parse_comparison_node<EqualNode<ErasedSeriesMethodContext>>);
  config_parser.register_node_parser(
   "NOT_EQUAL",
   serialize_comparison_node<NotEqualNode<ErasedSeriesMethodContext>>,
   parse_comparison_node<NotEqualNode<ErasedSeriesMethodContext>>);

  config_parser.register_node_parser(
   "ALWAYS",
   [](const ConfigParser&, const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_boolean_node(node, true);
   },
   [](ConfigParser::Parser, const jsoncons::ojson&) {
     return ErasedNode<ErasedSeriesMethodContext>{TrueNode{}};
   });
  config_parser.register_node_parser(
   "NEVER",
   [](const ConfigParser&, const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_boolean_node(node, false);
   },
   [](ConfigParser::Parser, const jsoncons::ojson&) {
     return ErasedNode<ErasedSeriesMethodContext>{FalseNode{}};
   });
  config_parser.register_node_parser(
   "AND",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_binary_logical_node<
      LogicalAndNode<ErasedSeriesMethodContext>>(
      config_parser, node, "firstCondition", "secondCondition");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_binary_logical_node<
      LogicalAndNode<ErasedSeriesMethodContext>>(
      config_parser, params, "firstCondition", "secondCondition");
   });
  config_parser.register_node_parser(
   "OR",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_binary_logical_node<
      LogicalOrNode<ErasedSeriesMethodContext>>(
      config_parser, node, "firstCondition", "secondCondition");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_binary_logical_node<LogicalOrNode<ErasedSeriesMethodContext>>(
      config_parser, params, "firstCondition", "secondCondition");
   });
  config_parser.register_node_parser(
   "NOT",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_unary_logical_node<
      LogicalNotNode<ErasedSeriesMethodContext>>(
      config_parser, node, "condition");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_unary_logical_node<LogicalNotNode<ErasedSeriesMethodContext>>(
      config_parser, params, "condition");
   });
  config_parser.register_node_parser(
   "XOR",
   [](const ConfigParser& config_parser,
      const ErasedNode<ErasedSeriesMethodContext>& node) {
     return serialize_binary_logical_node<
      LogicalXorNode<ErasedSeriesMethodContext>>(
      config_parser, node, "firstCondition", "secondCondition");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::ojson& params) {
     return parse_binary_logical_node<
      LogicalXorNode<ErasedSeriesMethodContext>>(
      config_parser, params, "firstCondition", "secondCondition");
   });

  return config_parser;
}

} // namespace pludux::backtest
