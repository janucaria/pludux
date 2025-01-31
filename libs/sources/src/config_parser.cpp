#ifndef PLUDUX_FILTER_PARSER_HPP
#define PLUDUX_FILTER_PARSER_HPP

#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include <pludux/screener.hpp>

#include <pludux/config_parser.hpp>

namespace pludux {

template<typename T>
static auto get_param_or(const nlohmann::json& parameters,
                         const std::string& key,
                         const T& default_value) -> T
{
  return parameters.contains(key) ? parameters[key].get<T>() : default_value;
}

template<typename T>
static auto parse_ta_with_period_method(const ConfigParser& config_parser,
                                        const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto period = parameters["period"].get<int>();

  const auto offset =
   parameters.contains("offset") ? parameters["offset"].get<int>() : 0;

  const auto is_target_exists = parameters.contains("target");
  const auto target_method = is_target_exists
                              ? config_parser.parse_method(parameters["target"])
                              : screener::DataMethod{"close", 0};

  const auto ta_method = T{period, target_method, offset};
  return ta_method;
}

static auto parse_value_method(const ConfigParser& config_parser,
                               const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto value = parameters["value"].get<double>();
  return screener::ValueMethod{value};
}

static auto parse_data_method(const ConfigParser& config_parser,
                              const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto field = parameters["field"].get<std::string>();

  auto offset = std::size_t{0};
  if(parameters.contains("offset")) {
    offset = parameters["offset"].get<int>();
  }

  const auto field_method = screener::DataMethod{field, offset};

  return field_method;
}

static auto parse_atr_method(const ConfigParser& config_parser,
                             const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto period = parameters["period"].get<std::size_t>();
  const auto atr_method = screener::AtrMethod{period};
  return atr_method;
}

static auto parse_abs_diff_method(const ConfigParser& config_parser,
                                  const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto operand1 = config_parser.parse_method(parameters["operand1"]);
  const auto operand2 = config_parser.parse_method(parameters["operand2"]);

  const auto abs_diff_method = screener::AbsDiffMethod{operand1, operand2};
  return abs_diff_method;
}

static auto parse_kc_method(const ConfigParser& config_parser,
                            const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  auto output = KcOutput::middle;
  if(parameters.contains("output")) {
    const auto output_str = parameters["output"].get<std::string>();
    if(output_str == "middle") {
      output = KcOutput::middle;
    } else if(output_str == "upper") {
      output = KcOutput::upper;
    } else if(output_str == "lower") {
      output = KcOutput::lower;
    } else {
      const auto error_message = std::format("Unknown output: {}", output_str);
      throw std::invalid_argument{error_message};
    }
  }

  const auto ma_method = config_parser.parse_method(parameters["ma"]);
  const auto range_method = config_parser.parse_method(parameters["range"]);
  const auto multiplier = parameters["multiplier"].get<double>();
  const auto offset = get_param_or(parameters, "offset", std::size_t{0});

  const auto kc_method =
   screener::KcMethod{output, ma_method, range_method, multiplier, offset};
  return kc_method;
}

template<typename T>
static auto parse_binary_function_method(const ConfigParser& config_parser,
                                         const nlohmann::json& parameters,
                                         const std::string& first_operand_key,
                                         const std::string& second_operand_key)
 -> screener::ScreenerMethod
{
  const auto first_operand =
   config_parser.parse_method(parameters[first_operand_key]);
  const auto second_operand =
   config_parser.parse_method(parameters[second_operand_key]);

  const auto binary_function_method = T{first_operand, second_operand};
  return binary_function_method;
}

template<typename T>
static auto parse_unary_function_method(const ConfigParser& config_parser,
                                        const nlohmann::json& parameters,
                                        const std::string& operand_key)
 -> screener::ScreenerMethod
{
  const auto operand = config_parser.parse_method(parameters[operand_key]);
  const auto unary_function_method = T{operand};
  return unary_function_method;
}

template<typename T>
static auto parse_divergence_method(const ConfigParser& config_parser,
                                    const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto signal = config_parser.parse_method(parameters["signal"]);
  const auto reference = config_parser.parse_method(parameters["reference"]);

  const auto pivot_range =
   get_param_or(parameters, "pivotRange", std::size_t{5});
  const auto lookback_range =
   get_param_or(parameters, "lookbackRange", std::size_t{60});
  const auto offset = get_param_or(parameters, "offset", std::size_t{0});

  const auto divergence_method =
   T{signal, reference, pivot_range, lookback_range, offset};
  return divergence_method;
}

template<typename T>
static auto parse_comparison_filter(const ConfigParser& config_parser,
                                    const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto target = config_parser.parse_method(parameters["target"]);
  auto threshold = config_parser.parse_method(parameters["threshold"]);
  return T{target, threshold};
}

static auto parse_all_of_filter(const ConfigParser& config_parser,
                                const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto filters = std::vector<screener::ScreenerFilter>{};
  for(const auto& subFilter : parameters["filters"]) {
    filters.push_back(config_parser.parse_filter(subFilter));
  }
  return screener::AllOfFilter{filters};
}

static auto parse_any_of_filter(const ConfigParser& config_parser,
                                const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto filters = std::vector<screener::ScreenerFilter>{};
  for(const auto& subFilter : parameters["filters"]) {
    filters.push_back(config_parser.parse_filter(subFilter));
  }
  return screener::AnyOfFilter{filters};
}

static auto parse_crossunder_filter(const ConfigParser& config_parser,
                                    const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto signal = config_parser.parse_method(parameters["signal"]);
  auto reference = config_parser.parse_method(parameters["reference"]);
  return screener::CrossunderFilter{signal, reference};
}

static auto parse_crossover_filter(const ConfigParser& config_parser,
                                   const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto signal = config_parser.parse_method(parameters["signal"]);
  auto reference = config_parser.parse_method(parameters["reference"]);
  return screener::CrossoverFilter{signal, reference};
}

ConfigParser::ConfigParser() = default;

void ConfigParser::register_default_parsers()
{
  register_method_parser("VALUE", parse_value_method);
  register_method_parser("DATA", parse_data_method);
  register_method_parser(
   "CHANGES",
   [](const ConfigParser& config_parser, const nlohmann::json& parameters) {
     return parse_unary_function_method<screener::ChangesMethod>(
      config_parser, parameters, "operand");
   });

  register_method_parser("SMA",
                         parse_ta_with_period_method<screener::SmaMethod>);

  register_method_parser("EMA",
                         parse_ta_with_period_method<screener::EmaMethod>);

  register_method_parser("WMA",
                         parse_ta_with_period_method<screener::WmaMethod>);

  register_method_parser("RMA",
                         parse_ta_with_period_method<screener::RmaMethod>);

  register_method_parser("HMA",
                         parse_ta_with_period_method<screener::HmaMethod>);

  register_method_parser("RSI",
                         parse_ta_with_period_method<screener::RsiMethod>);

  register_method_parser("ROC",
                         parse_ta_with_period_method<screener::RocMethod>);

  register_method_parser("RVOL",
                         parse_ta_with_period_method<screener::RvolMethod>);

  register_method_parser("ATR", parse_atr_method);
  register_method_parser("KC", parse_kc_method);
  register_method_parser("ABS_DIFF", parse_abs_diff_method);
  register_method_parser(
   "ADD",
   [](const ConfigParser& config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::AddMethod>(
      config_parser, parameters, "augend", "addend");
   });
  register_method_parser(
   "SUBTRACT",
   [](const ConfigParser& config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::SubtractMethod>(
      config_parser, parameters, "minuend", "subtrahend");
   });
  register_method_parser(
   "MULTIPLY",
   [](const ConfigParser& config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::MultiplyMethod>(
      config_parser, parameters, "multiplicand", "multiplier");
   });
  register_method_parser(
   "DIVIDE",
   [](const ConfigParser& config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::DivideMethod>(
      config_parser, parameters, "dividend", "divisor");
   });
  register_method_parser(
   "NEGATE",
   [](const ConfigParser& config_parser, const nlohmann::json& parameters) {
     return parse_unary_function_method<screener::NegateMethod>(
      config_parser, parameters, "operand");
   });
  register_method_parser(
   "BULLISH_DIVERGENCE",
   parse_divergence_method<screener::BullishDivergenceMethod>);
  register_method_parser(
   "HIDDEN_BULLISH_DIVERGENCE",
   parse_divergence_method<screener::HiddenBullishDivergenceMethod>);

  register_filter_parser("ALL_OF", parse_all_of_filter);
  register_filter_parser("ANY_OF", parse_any_of_filter);
  register_filter_parser("CROSSUNDER", parse_crossunder_filter);
  register_filter_parser("CROSSOVER", parse_crossover_filter);
  register_filter_parser("GREATER_THAN",
                         parse_comparison_filter<screener::GreaterThanFilter>);
  register_filter_parser("LESS_THAN",
                         parse_comparison_filter<screener::LessThanFilter>);
  register_filter_parser("GREATER_EQUAL",
                         parse_comparison_filter<screener::GreaterEqualFilter>);
  register_filter_parser("LESS_EQUAL",
                         parse_comparison_filter<screener::LessEqualFilter>);
}

void ConfigParser::register_filter_parser(const std::string& filter_name,
                                          const FilterParser& filter_parser)
{
  filter_parsers_.emplace(filter_name, filter_parser);
}

void ConfigParser::register_method_parser(const std::string& method_name,
                                          const MethodParser& method_parser)
{
  method_parsers_.emplace(method_name, method_parser);
}

auto ConfigParser::parse_filter(const nlohmann::json& config) const
 -> screener::ScreenerFilter
{
  const auto filter = config["filter"].get<std::string>();

  if(filter_parsers_.contains(filter)) {
    return filter_parsers_.at(filter)(*this, config);
  }

  const auto error_message = std::format("Unknown filter: {}", filter);
  throw std::invalid_argument{error_message};
}

auto ConfigParser::parse_method(const nlohmann::json& config) const
 -> screener::ScreenerMethod
{
  const auto method = config["method"].get<std::string>();

  if(method_parsers_.contains(method)) {
    return method_parsers_.at(method)(*this, config);
  }

  const auto error_message = std::format("Unknown method: {}", method);
  throw std::invalid_argument{error_message};
}

} // namespace pludux

#endif