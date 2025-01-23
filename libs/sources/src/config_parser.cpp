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
static auto parse_ta_with_period_method(const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto period = parameters["period"].get<int>();

  const auto offset =
   parameters.contains("offset") ? parameters["offset"].get<int>() : 0;

  const auto is_target_exists = parameters.contains("target");
  const auto target_method = is_target_exists
                              ? parse_screener_method(parameters["target"])
                              : screener::DataMethod{"close", 0};

  const auto ta_method = T{period, target_method, offset};
  return ta_method;
}

static auto
parse_value_method(const nlohmann::json& parameters) -> screener::ScreenerMethod
{
  const auto value = parameters["value"].get<double>();
  return screener::ValueMethod{value};
}

static auto
parse_data_method(const nlohmann::json& parameters) -> screener::ScreenerMethod
{
  const auto field = parameters["field"].get<std::string>();

  auto offset = std::size_t{0};
  if(parameters.contains("offset")) {
    offset = parameters["offset"].get<int>();
  }

  const auto field_method = screener::DataMethod{field, offset};

  return field_method;
}

static auto
parse_atr_method(const nlohmann::json& parameters) -> screener::ScreenerMethod
{
  const auto period = parameters["period"].get<std::size_t>();
  const auto atr_method = screener::AtrMethod{period};
  return atr_method;
}

static auto parse_abs_diff_method(const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto operand1 = parse_screener_method(parameters["operand1"]);
  const auto operand2 = parse_screener_method(parameters["operand2"]);

  const auto abs_diff_method = screener::AbsDiffMethod{operand1, operand2};
  return abs_diff_method;
}

template<typename T>
static auto parse_binary_function_method(const nlohmann::json& parameters,
                                         const std::string& first_operand_key,
                                         const std::string& second_operand_key)
 -> screener::ScreenerMethod
{
  const auto first_operand =
   parse_screener_method(parameters[first_operand_key]);
  const auto second_operand =
   parse_screener_method(parameters[second_operand_key]);

  const auto binary_function_method = T{first_operand, second_operand};
  return binary_function_method;
}

template<typename T>
static auto parse_unary_function_method(const nlohmann::json& parameters,
                                        const std::string& operand_key)
 -> screener::ScreenerMethod
{
  const auto operand = parse_screener_method(parameters[operand_key]);
  const auto unary_function_method = T{operand};
  return unary_function_method;
}

template<typename T>
static auto parse_divergence_method(const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto signal = parse_screener_method(parameters["signal"]);
  const auto reference = parse_screener_method(parameters["reference"]);

  const auto pivot_range =
   get_param_or(parameters, "pivotRange", std::size_t{5});
  const auto lookback_range =
   get_param_or(parameters, "lookbackRange", std::size_t{60});
  const auto offset = get_param_or(parameters, "offset", std::size_t{0});

  const auto divergence_method =
   T{signal, reference, pivot_range, lookback_range, offset};
  return divergence_method;
}

auto parse_screener_method(const nlohmann::json& config)
 -> screener::ScreenerMethod
{
  const auto method = config["method"].get<std::string>();

  if(method == "VALUE") {
    return parse_value_method(config);
  }

  if(method == "DATA") {
    return parse_data_method(config);
  }

  if(method == "CHANGES") {
    return parse_unary_function_method<screener::ChangesMethod>(config,
                                                                "operand");
  }

  if(method == "SMA") {
    return parse_ta_with_period_method<screener::SmaMethod>(config);
  }

  if(method == "EMA") {
    return parse_ta_with_period_method<screener::EmaMethod>(config);
  }

  if(method == "WMA") {
    return parse_ta_with_period_method<screener::WmaMethod>(config);
  }

  if(method == "RMA") {
    return parse_ta_with_period_method<screener::RmaMethod>(config);
  }

  if(method == "HMA") {
    return parse_ta_with_period_method<screener::HmaMethod>(config);
  }

  if(method == "RSI") {
    return parse_ta_with_period_method<screener::RsiMethod>(config);
  }

  if(method == "ATR") {
    return parse_atr_method(config);
  }

  if(method == "ABS_DIFF") {
    return parse_abs_diff_method(config);
  }

  if(method == "ADD") {
    return parse_binary_function_method<screener::AddMethod>(
     config, "augend", "addend");
  }

  if(method == "SUBTRACT") {
    return parse_binary_function_method<screener::SubtractMethod>(
     config, "minuend", "subtrahend");
  }

  if(method == "MULTIPLY") {
    return parse_binary_function_method<screener::MultiplyMethod>(
     config, "multiplicand", "multiplier");
  }

  if(method == "DIVIDE") {
    return parse_binary_function_method<screener::DivideMethod>(
     config, "dividend", "divisor");
  }

  if(method == "NEGATE") {
    return parse_unary_function_method<screener::NegateMethod>(config,
                                                               "operand");
  }

  if(method == "BULLISH_DIVERGENCE") {
    return parse_divergence_method<screener::BullishDivergenceMethod>(config);
  }

  const auto error_message = std::format("Unknown method: {}", method);
  throw std::invalid_argument(error_message);
}

template<typename T>
static auto parse_comparison_filter(const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto target = parse_screener_method(parameters["target"]);
  auto threshold = parse_screener_method(parameters["threshold"]);
  return T{target, threshold};
}

static auto parse_all_of_filter(const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto filters = std::vector<screener::ScreenerFilter>{};
  for(const auto& subFilter : parameters["filters"]) {
    filters.push_back(parse_screener_filter(subFilter));
  }
  return screener::AllOfFilter{filters};
}

static auto parse_any_of_filter(const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto filters = std::vector<screener::ScreenerFilter>{};
  for(const auto& subFilter : parameters["filters"]) {
    filters.push_back(parse_screener_filter(subFilter));
  }
  return screener::AnyOfFilter{filters};
}

static auto parse_crossunder_filter(const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto signal = parse_screener_method(parameters["signal"]);
  auto reference = parse_screener_method(parameters["reference"]);
  return screener::CrossunderFilter{signal, reference};
}

static auto parse_crossover_filter(const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto signal = parse_screener_method(parameters["signal"]);
  auto reference = parse_screener_method(parameters["reference"]);
  return screener::CrossoverFilter{signal, reference};
}

auto parse_screener_filter(const nlohmann::json& config)
 -> screener::ScreenerFilter
{
  std::string filter = config["filter"];

  if(filter == "ALL_OF") {
    return parse_all_of_filter(config);
  }

  if(filter == "ANY_OF") {
    return parse_any_of_filter(config);
  }

  if(filter == "GREATER_THAN") {
    return parse_comparison_filter<screener::GreaterThanFilter>(config);
  }

  if(filter == "GREATER_EQUAL") {
    return parse_comparison_filter<screener::GreaterEqualFilter>(config);
  }

  if(filter == "LESS_THAN") {
    return parse_comparison_filter<screener::LessThanFilter>(config);
  }

  if(filter == "LESS_EQUAL") {
    return parse_comparison_filter<screener::LessEqualFilter>(config);
  }

  if(filter == "EQUAL") {
    return parse_comparison_filter<screener::EqualFilter>(config);
  }

  if(filter == "NOT_EQUAL") {
    return parse_comparison_filter<screener::NotEqualFilter>(config);
  }

  if(filter == "CROSSUNDER") {
    return parse_crossunder_filter(config);
  }

  if(filter == "CROSSOVER") {
    return parse_crossover_filter(config);
  }

  const auto error_message = std::format("Unknown filter: {}", filter);
  throw std::invalid_argument{error_message};
}

} // namespace pludux

#endif