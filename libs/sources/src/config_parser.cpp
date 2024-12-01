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
static auto parse_ta_with_period_method(const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto period = parameters["period"].get<int>();

  const auto offset =
   parameters.contains("offset") ? parameters["offset"].get<int>() : 0;

  const auto is_target_exists = parameters.contains("target");
  const auto target_method = is_target_exists
                              ? parse_screener_method(parameters["target"])
                              : screener::FieldMethod{"close", 0};

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
parse_field_method(const nlohmann::json& parameters) -> screener::ScreenerMethod
{
  const auto field = parameters["field"].get<std::string>();

  auto offset = 0;
  if(parameters.contains("offset")) {
    offset = parameters["offset"].get<int>();
  }

  const auto field_method = screener::FieldMethod{field, offset};

  return field_method;
}

auto parse_screener_method(const nlohmann::json& config)
 -> screener::ScreenerMethod
{
  const auto method = config["method"].get<std::string>();

  if(method == "VALUE") {
    return parse_value_method(config);
  }

  if(method == "FIELD") {
    return parse_field_method(config);
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

  const auto error_message = std::format("Unknown filter: {}", filter);
  throw std::invalid_argument{error_message};
}

} // namespace pludux

#endif