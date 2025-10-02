module;

#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <jsoncons/json.hpp>

export module pludux:config_parser;

import :conditions;
import :series;

export namespace pludux {

class ConfigParser {
public:
  class Parser {
  public:
    Parser(ConfigParser& config_parser)
    : config_parser_{config_parser}
    {
    }

    auto parse_method(this auto& self, const jsoncons::json& config)
     -> AnySeriesMethod
    {
      return self.config_parser_.parse_method(config);
    }

    auto parse_filter(this auto& self, const jsoncons::json& config)
     -> AnyConditionMethod
    {
      return self.config_parser_.parse_filter(config);
    }

  private:
    ConfigParser& config_parser_;
  };

  friend Parser;

  using ConditionSerialize = std::function<
   auto(const ConfigParser&, const AnyConditionMethod&)->jsoncons::json>;

  using ConditionDeserialize = std::function<
   auto(ConfigParser::Parser, const jsoncons::json&)->AnyConditionMethod>;

  using MethodSerialize = std::function<
   auto(const ConfigParser&, const AnySeriesMethod&)->jsoncons::json>;

  using MethodDeserialize = std::function<
   auto(ConfigParser::Parser, const jsoncons::json&)->AnySeriesMethod>;

  ConfigParser()
  : filter_parsers_{}
  , method_parsers_{}
  {
  }

  void register_filter_parser(this auto& self,
                              const std::string& filter_name,
                              const ConditionSerialize& filter_serialize,
                              const ConditionDeserialize& filter_deserialize)
  {
    self.filter_parsers_.emplace(
     filter_name, std::make_pair(filter_serialize, filter_deserialize));
  }

  void register_method_parser(this auto& self,
                              const std::string& method_name,
                              const MethodSerialize& method_serialize,
                              const MethodDeserialize& method_deserialize)
  {
    self.method_parsers_.emplace(
     method_name, std::make_pair(method_serialize, method_deserialize));
  }

  auto parser(this auto& self) -> Parser
  {
    return Parser{self};
  }

  auto parse_filter(this auto& self, const jsoncons::json& config)
   -> AnyConditionMethod
  {
    if(config.is_bool()) {
      if(config.as_bool()) {
        return TrueMethod{};
      }

      return FalseMethod{};
    }
    const auto& filter_parsers = self.filter_parsers_;
    const auto filter = config.at("filter").as_string();

    try {
      if(filter_parsers.contains(filter)) {
        const auto& [_, filter_deserialize] = filter_parsers.at(filter);
        return filter_deserialize(self, config);
      }

      const auto error_message = std::format("Unknown filter: {}", filter);
      throw std::invalid_argument{error_message};
    } catch(const std::exception& e) {
      const auto error_message =
       std::format("Error parsing filter {}:\n{}", filter, e.what());
      throw std::invalid_argument{error_message};
    } catch(...) {
      const auto error_message =
       std::format("Unknown error parsing filter {}", filter);
      throw std::invalid_argument{error_message};
    }
  }

  auto serialize_filter(this const auto& self, const AnyConditionMethod& filter)
   -> jsoncons::json
  {
    auto serialized_filter = jsoncons::json{};

    for(const auto& [filter_name, filter_parser] : self.filter_parsers_) {
      const auto& [filter_serialize, _] = filter_parser;
      serialized_filter = filter_serialize(self, filter);
      if(!serialized_filter.empty()) {
        if(!serialized_filter.is_object()) {
          const auto error_message = std::format(
           "Condition {} serialization must return an object", filter_name);
          throw std::invalid_argument{error_message};
        }

        if(!serialized_filter.contains("filter")) {
          serialized_filter["filter"] = filter_name;
        } else if(serialized_filter.at("filter").as_string() != filter_name) {
          const auto error_message = std::format(
           "Condition {} serialization must return 'filter' key with value {}",
           filter_name,
           filter_name);
          throw std::invalid_argument{error_message};
        }
        break;
      }
    }

    return serialized_filter;
  }

  auto parse_method(this auto& self, const jsoncons::json& config)
   -> AnySeriesMethod
  {
    if(config.is_number()) {
      return ValueMethod{config.as_double()};
    }

    if(config.is_string()) {
      const auto named_method = config.as_string();
      const auto expanded_method =
       jsoncons::json::object{{"method", named_method}};
      return self.parse_method(expanded_method);
    }

    auto config_method = config;
    const auto method = config.at("method").as_string();

    auto& method_parsers = self.method_parsers_;
    if(!method_parsers.contains(method)) {
      const auto error_message = std::format("Unknown method: {}", method);
      throw std::invalid_argument{error_message};
    }

    try {
      const auto method_deserialize = method_parsers.at(method).second;
      const auto method_result = method_deserialize(self, config_method);

      return method_result;
    } catch(const std::exception& e) {
      const auto error_message =
       std::format("Error parsing method {}:\n{}", method, e.what());
      throw std::invalid_argument{error_message};
    } catch(...) {
      const auto error_message =
       std::format("Unknown error parsing method {}", method);
      throw std::invalid_argument{error_message};
    }
  }

  auto serialize_method(this const auto& self, const AnySeriesMethod& method)
   -> jsoncons::json
  {
    auto serialized_method = jsoncons::json::null();

    for(const auto& [method_name, method_parser] : self.method_parsers_) {
      const auto& [method_serialize, _] = method_parser;
      serialized_method = method_serialize(self, method);
      if(serialized_method.is_object()) {
        serialized_method["method"] = method_name;
        break;
      }
    }

    return serialized_method;
  }

private:
  std::unordered_map<std::string,
                     std::pair<ConditionSerialize, ConditionDeserialize>>
   filter_parsers_;
  std::unordered_map<std::string, std::pair<MethodSerialize, MethodDeserialize>>
   method_parsers_;
};

auto make_default_registered_config_parser() -> ConfigParser;

} // namespace pludux

namespace pludux {

template<typename T>
static auto get_param_or(const jsoncons::json& parameters,
                         const std::string& key,
                         const T& default_value) -> T
{
  return parameters.contains(key) ? parameters.at(key).as<T>() : default_value;
}

static auto parse_method_from_param_or(ConfigParser::Parser config_parser,
                                       const jsoncons::json& parameters,
                                       const std::string& key,
                                       const AnySeriesMethod& default_value)
 -> AnySeriesMethod
{
  if(!parameters.contains(key)) {
    return default_value;
  }

  return config_parser.parse_method(parameters.at(key));
}

template<template<typename> typename TMethod>
static auto parse_ta_with_period_method(ConfigParser::Parser config_parser,
                                        const jsoncons::json& parameters)
 -> AnySeriesMethod
{
  const auto period = get_param_or<std::size_t>(parameters, "period", 14);
  const auto source = parse_method_from_param_or(
   config_parser, parameters, "source", CloseMethod{});

  return TMethod<AnySeriesMethod>{source, period};
}

template<template<typename> typename TMethod>
static auto serialize_ta_with_period_method(const ConfigParser& config_parser,
                                            const AnySeriesMethod& method)
 -> jsoncons::json
{
  auto serialized_method = jsoncons::json::null();

  auto ta_method = series_method_cast<TMethod<AnySeriesMethod>>(method);

  if(ta_method) {
    serialized_method = jsoncons::json{};
    serialized_method["period"] = ta_method->period();
    serialized_method["source"] =
     config_parser.serialize_method(ta_method->source());
  }

  return serialized_method;
}

template<typename T>
static auto serialize_ohlcv_method(const ConfigParser& config_parser,
                                   const AnySeriesMethod& method)
 -> jsoncons::json
{
  auto serialized_method = jsoncons::json::null();
  auto ohlcv_method = series_method_cast<T>(method);
  if(ohlcv_method) {
    serialized_method = jsoncons::json{};
  }
  return serialized_method;
};

template<typename T>
static auto parse_ohlcv_method(ConfigParser::Parser config_parser,
                               const jsoncons::json& parameters)
 -> AnySeriesMethod
{
  auto ohlcv_method = T{};
  return ohlcv_method;
}

static auto serialize_value_method(const ConfigParser& config_parser,
                                   const AnySeriesMethod& method)
 -> jsoncons::json
{
  auto serialized_method = jsoncons::json::null();

  auto value_method = series_method_cast<ValueMethod>(method);

  if(value_method) {
    serialized_method = jsoncons::json{};
    serialized_method["value"] = value_method->value();
  }

  return serialized_method;
}

static auto deserialize_value_method(ConfigParser::Parser config_parser,
                                     const jsoncons::json& parameters)
 -> AnySeriesMethod
{
  const auto value = parameters.at("value").as_double();
  return ValueMethod{value};
}

static auto parse_data_method(ConfigParser::Parser config_parser,
                              const jsoncons::json& parameters)
 -> AnySeriesMethod
{
  const auto field = parameters.at("field").as_string();

  const auto field_method = DataMethod{field};

  return field_method;
}

static auto serialize_data_method(const ConfigParser& config_parser,
                                  const AnySeriesMethod& method)
 -> jsoncons::json
{
  auto serialized_method = jsoncons::json::null();

  auto data_method = series_method_cast<DataMethod>(method);

  if(data_method) {
    serialized_method = jsoncons::json{};
    serialized_method["field"] = data_method->field();
  }

  return serialized_method;
}

static auto parse_atr_method(ConfigParser::Parser config_parser,
                             const jsoncons::json& parameters)
 -> AnySeriesMethod
{
  auto atr_method = AtrMethod{};

  if(parameters.contains("period")) {
    atr_method.period(parameters.at("period").as<std::size_t>());
  }

  if(parameters.contains("multiplier")) {
    atr_method.multiplier(parameters.at("multiplier").as_double());
  }

  return atr_method;
}

static auto serialize_atr_method(const ConfigParser& config_parser,
                                 const AnySeriesMethod& method)
 -> jsoncons::json
{
  auto serialized_method = jsoncons::json::null();

  auto atr_method = series_method_cast<AtrMethod>(method);
  if(atr_method) {
    serialized_method = jsoncons::json{};
    serialized_method["period"] = atr_method->period();
    serialized_method["multiplier"] = atr_method->multiplier();
  }

  return serialized_method;
}

static auto serialize_kc_method(const ConfigParser& config_parser,
                                const AnySeriesMethod& method) -> jsoncons::json
{
  auto serialized_method = jsoncons::json::null();

  auto kc_method =
   series_method_cast<KcMethod<AnySeriesMethod, AnySeriesMethod>>(method);
  if(kc_method) {
    serialized_method = jsoncons::json{};
    serialized_method["ma"] = config_parser.serialize_method(kc_method->ma());
    serialized_method["range"] =
     config_parser.serialize_method(kc_method->range());
    serialized_method["multiplier"] = kc_method->multiplier();
  }

  return serialized_method;
}

static auto parse_kc_method(ConfigParser::Parser config_parser,
                            const jsoncons::json& parameters) -> AnySeriesMethod
{
  const auto ma_method = config_parser.parse_method(parameters.at("ma"));
  const auto range_method = config_parser.parse_method(parameters.at("range"));
  const auto multiplier = parameters.at("multiplier").as_double();

  const auto kc_method = KcMethod{ma_method, range_method, multiplier};
  return kc_method;
}

template<template<typename, typename> typename T>
static auto parse_binary_function_method(ConfigParser::Parser config_parser,
                                         const jsoncons::json& parameters,
                                         const std::string& first_operand_key,
                                         const std::string& second_operand_key)
 -> AnySeriesMethod
{
  const auto first_operand =
   config_parser.parse_method(parameters.at(first_operand_key));
  const auto second_operand =
   config_parser.parse_method(parameters.at(second_operand_key));

  const auto binary_function_method = T{first_operand, second_operand};
  return binary_function_method;
}

template<template<typename, typename> typename T>
static auto
serialize_binary_function_method(const ConfigParser& config_parser,
                                 const AnySeriesMethod& method,
                                 const std::string& first_operand_key,
                                 const std::string& second_operand_key)
 -> jsoncons::json
{
  using TMethod = T<AnySeriesMethod, AnySeriesMethod>;

  auto serialized_method = jsoncons::json::null();

  auto binary_function_method = series_method_cast<TMethod>(method);
  if(binary_function_method) {
    serialized_method = jsoncons::json{};
    serialized_method[first_operand_key] =
     config_parser.serialize_method(binary_function_method->operand1());
    serialized_method[second_operand_key] =
     config_parser.serialize_method(binary_function_method->operand2());
  }

  return serialized_method;
}

template<template<typename> typename T>
static auto parse_unary_function_method(ConfigParser::Parser config_parser,
                                        const jsoncons::json& parameters,
                                        const std::string& operand_key)
 -> AnySeriesMethod
{
  const auto operand = config_parser.parse_method(parameters.at(operand_key));
  const auto unary_function_method = T{operand};
  return unary_function_method;
}

template<template<typename> typename T>
static auto serialize_unary_function_method(const ConfigParser& config_parser,
                                            const AnySeriesMethod& method,
                                            const std::string& operand_key)
 -> jsoncons::json
{
  auto serialized_method = jsoncons::json::null();

  auto unary_function_method = series_method_cast<T<AnySeriesMethod>>(method);
  if(unary_function_method) {
    serialized_method = jsoncons::json{};
    serialized_method[operand_key] =
     config_parser.serialize_method(unary_function_method->operand());
  }

  return serialized_method;
}

template<typename T>
static auto parse_divergence_method(ConfigParser::Parser config_parser,
                                    const jsoncons::json& parameters)
 -> AnySeriesMethod
{
  auto divergence_method = T{};

  if(parameters.contains("pivotRange")) {
    divergence_method.pivot_range(
     parameters.at("pivotRange").as<std::size_t>());
  }

  if(parameters.contains("lookbackRange")) {
    divergence_method.lookback_range(
     parameters.at("lookbackRange").as<std::size_t>());
  }

  if(parameters.contains("signal")) {
    divergence_method.signal(
     config_parser.parse_method(parameters.at("signal")));
  }

  if(parameters.contains("reference")) {
    divergence_method.reference(
     config_parser.parse_method(parameters.at("reference")));
  }

  return divergence_method;
}

template<typename T>
static auto serialize_divergence_method(const ConfigParser& config_parser,
                                        const AnySeriesMethod& method)
 -> jsoncons::json
{
  auto serialized_method = jsoncons::json::null();

  auto divergence_method = series_method_cast<T>(method);
  if(divergence_method) {
    serialized_method = jsoncons::json{};
    serialized_method["signal"] =
     config_parser.serialize_method(divergence_method->signal());
    serialized_method["reference"] =
     config_parser.serialize_method(divergence_method->reference());
    serialized_method["pivotRange"] = divergence_method->pivot_range();
    serialized_method["lookbackRange"] = divergence_method->lookback_range();
  }

  return serialized_method;
}

template<typename T>
static auto parse_binary_function_filter(ConfigParser::Parser config_parser,
                                         const jsoncons::json& parameters,
                                         const std::string& first_operand_key,
                                         const std::string& second_operand_key)
 -> AnyConditionMethod
{
  const auto first_operand =
   config_parser.parse_filter(parameters.at(first_operand_key));
  const auto second_operand =
   config_parser.parse_filter(parameters.at(second_operand_key));

  const auto binary_function_filter = T{first_operand, second_operand};
  return binary_function_filter;
}

template<typename T>
static auto
serialize_binary_function_filter(const ConfigParser& config_parser,
                                 const AnyConditionMethod& filter,
                                 const std::string& first_operand_key,
                                 const std::string& second_operand_key)
 -> jsoncons::json
{
  auto serialized_filter = jsoncons::json{};

  auto binary_function_filter = condition_method_cast<T>(filter);
  if(binary_function_filter) {
    serialized_filter[first_operand_key] =
     config_parser.serialize_filter(binary_function_filter->first_condition());
    serialized_filter[second_operand_key] =
     config_parser.serialize_filter(binary_function_filter->second_condition());
  }

  return serialized_filter;
}

template<typename T>
static auto parse_unary_function_filter(ConfigParser::Parser config_parser,
                                        const jsoncons::json& parameters,
                                        const std::string& operand_key)
 -> AnyConditionMethod
{
  const auto operand = config_parser.parse_filter(parameters.at(operand_key));
  const auto unary_function_filter = T{operand};
  return unary_function_filter;
}

template<typename T>
static auto serialize_unary_function_filter(const ConfigParser& config_parser,
                                            const AnyConditionMethod& filter,
                                            const std::string& operand_key)
 -> jsoncons::json
{
  auto serialized_filter = jsoncons::json{};

  auto unary_function_filter = condition_method_cast<T>(filter);
  if(unary_function_filter) {
    serialized_filter[operand_key] =
     config_parser.serialize_filter(unary_function_filter->other_condition());
  }

  return serialized_filter;
}

template<typename T>
static auto parse_comparison_filter(ConfigParser::Parser config_parser,
                                    const jsoncons::json& parameters)
 -> AnyConditionMethod
{
  auto target = config_parser.parse_method(parameters.at("target"));
  auto threshold = config_parser.parse_method(parameters.at("threshold"));
  return T{target, threshold};
}

template<typename T>
static auto serialize_comparison_filter(const ConfigParser& config_parser,
                                        const AnyConditionMethod& filter)
 -> jsoncons::json
{
  auto serialized_filter = jsoncons::json{};

  auto comparison_filter = condition_method_cast<T>(filter);
  if(comparison_filter) {
    serialized_filter["target"] =
     config_parser.serialize_method(comparison_filter->target());
    serialized_filter["threshold"] =
     config_parser.serialize_method(comparison_filter->threshold());
  }

  return serialized_filter;
}

static auto parse_all_of_filter(ConfigParser::Parser config_parser,
                                const jsoncons::json& parameters)
 -> AnyConditionMethod
{
  if(!parameters.contains("conditions")) {
    throw std::invalid_argument{"ALL_OF: 'conditions' is not found"};
  }

  auto conditions = std::vector<AnyConditionMethod>{};
  for(const auto& filter : parameters.at("conditions").array_range()) {
    conditions.push_back(config_parser.parse_filter(filter));
  }
  return AllOfMethod{conditions};
}

static auto serialize_all_of_filter(const ConfigParser& config_parser,
                                    const AnyConditionMethod& filter)
 -> jsoncons::json
{
  auto serialized_filter = jsoncons::json{};

  auto all_of_filter = condition_method_cast<AllOfMethod>(filter);
  if(all_of_filter) {
    auto conditions = jsoncons::json::array();
    for(const auto& condition : all_of_filter->conditions()) {
      conditions.push_back(config_parser.serialize_filter(condition));
    }
    serialized_filter["conditions"] = conditions;
  }

  return serialized_filter;
}

static auto parse_any_of_filter(ConfigParser::Parser config_parser,
                                const jsoncons::json& parameters)
 -> AnyConditionMethod
{
  if(!parameters.contains("conditions")) {
    throw std::invalid_argument{"ANY_OF: 'conditions' is not found"};
  }
  auto conditions = std::vector<AnyConditionMethod>{};
  for(const auto& filter : parameters.at("conditions").array_range()) {
    conditions.push_back(config_parser.parse_filter(filter));
  }
  return AnyOfMethod{conditions};
}

static auto serialize_any_of_filter(const ConfigParser& config_parser,
                                    const AnyConditionMethod& filter)
 -> jsoncons::json
{
  auto serialized_filter = jsoncons::json{};

  auto any_of_filter = condition_method_cast<AnyOfMethod>(filter);
  if(any_of_filter) {
    auto conditions = jsoncons::json::array();
    for(const auto& condition : any_of_filter->conditions()) {
      conditions.push_back(config_parser.serialize_filter(condition));
    }
    serialized_filter["conditions"] = conditions;
  }

  return serialized_filter;
}

static auto parse_crossunder_filter(ConfigParser::Parser config_parser,
                                    const jsoncons::json& parameters)
 -> AnyConditionMethod
{
  auto signal = config_parser.parse_method(parameters.at("signal"));
  auto reference = config_parser.parse_method(parameters.at("reference"));
  return CrossunderMethod{signal, reference};
}

static auto serialize_crossunder_filter(const ConfigParser& config_parser,
                                        const AnyConditionMethod& filter)
 -> jsoncons::json
{
  auto serialized_filter = jsoncons::json{};

  auto crossunder_filter = condition_method_cast<CrossunderMethod>(filter);
  if(crossunder_filter) {
    serialized_filter["signal"] =
     config_parser.serialize_method(crossunder_filter->signal());
    serialized_filter["reference"] =
     config_parser.serialize_method(crossunder_filter->reference());
  }

  return serialized_filter;
}

static auto parse_crossover_filter(ConfigParser::Parser config_parser,
                                   const jsoncons::json& parameters)
 -> AnyConditionMethod
{
  auto signal = config_parser.parse_method(parameters.at("signal"));
  auto reference = config_parser.parse_method(parameters.at("reference"));
  return CrossoverMethod{signal, reference};
}

static auto serialize_crossover_filter(const ConfigParser& config_parser,
                                       const AnyConditionMethod& filter)
 -> jsoncons::json
{
  auto serialized_filter = jsoncons::json{};

  auto crossover_filter = condition_method_cast<CrossoverMethod>(filter);
  if(crossover_filter) {
    serialized_filter["signal"] =
     config_parser.serialize_method(crossover_filter->signal());
    serialized_filter["reference"] =
     config_parser.serialize_method(crossover_filter->reference());
  }

  return serialized_filter;
}

auto make_default_registered_config_parser() -> ConfigParser
{
  ConfigParser config_parser;

  config_parser.register_method_parser(
   "VALUE", serialize_value_method, deserialize_value_method);

  config_parser.register_method_parser(
   "DATA", serialize_data_method, parse_data_method);

  config_parser.register_method_parser(
   "OPEN", serialize_ohlcv_method<OpenMethod>, parse_ohlcv_method<OpenMethod>);

  config_parser.register_method_parser(
   "HIGH", serialize_ohlcv_method<HighMethod>, parse_ohlcv_method<HighMethod>);

  config_parser.register_method_parser(
   "LOW", serialize_ohlcv_method<LowMethod>, parse_ohlcv_method<LowMethod>);

  config_parser.register_method_parser("CLOSE",
                                       serialize_ohlcv_method<CloseMethod>,
                                       parse_ohlcv_method<CloseMethod>);

  config_parser.register_method_parser("VOLUME",
                                       serialize_ohlcv_method<VolumeMethod>,
                                       parse_ohlcv_method<VolumeMethod>);

  config_parser.register_method_parser(
   "CHANGE",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod any_series_method) -> jsoncons::json {
     auto serialized_method = jsoncons::json::null();

     auto changes_method =
      series_method_cast<ChangeMethod<AnySeriesMethod>>(any_series_method);
     if(changes_method) {
       serialized_method = jsoncons::json{};
       serialized_method["source"] =
        config_parser.serialize_method(changes_method->source());
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     const auto source = parse_method_from_param_or(
      config_parser, parameters, "source", CloseMethod{});

     return ChangeMethod{source};
   });

  config_parser.register_method_parser(
   "SMA",
   serialize_ta_with_period_method<SmaMethod>,
   parse_ta_with_period_method<SmaMethod>);

  config_parser.register_method_parser(
   "EMA",
   serialize_ta_with_period_method<EmaMethod>,
   parse_ta_with_period_method<EmaMethod>);

  config_parser.register_method_parser(
   "WMA",
   serialize_ta_with_period_method<WmaMethod>,
   parse_ta_with_period_method<WmaMethod>);

  config_parser.register_method_parser(
   "RMA",
   serialize_ta_with_period_method<RmaMethod>,
   parse_ta_with_period_method<RmaMethod>);

  config_parser.register_method_parser(
   "HMA",
   serialize_ta_with_period_method<HmaMethod>,
   parse_ta_with_period_method<HmaMethod>);

  config_parser.register_method_parser(
   "RSI",
   serialize_ta_with_period_method<RsiMethod>,
   parse_ta_with_period_method<RsiMethod>);

  config_parser.register_method_parser(
   "ROC",
   serialize_ta_with_period_method<RocMethod>,
   parse_ta_with_period_method<RocMethod>);

  config_parser.register_method_parser(
   "RVOL",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod any_series_method) -> jsoncons::json {
     auto serialized_method = jsoncons::json::null();

     auto rvol_method = series_method_cast<RvolMethod>(any_series_method);

     if(rvol_method) {
       serialized_method = jsoncons::json{};
       serialized_method["period"] = rvol_method->period();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     const auto period = get_param_or<std::size_t>(parameters, "period", 14);
     return RvolMethod{period};
   });

  config_parser.register_method_parser(
   "ATR", serialize_atr_method, parse_atr_method);

  config_parser.register_method_parser(
   "KC", serialize_kc_method, parse_kc_method);

  config_parser.register_method_parser(
   "REFERENCE",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod any_series_method) -> jsoncons::json {
     auto serialized_method = jsoncons::json::null();

     auto reference_method =
      series_method_cast<ReferenceMethod>(any_series_method);
     if(reference_method) {
       serialized_method = jsoncons::json{};
       serialized_method["name"] = reference_method->name();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     const auto name = get_param_or<std::string>(parameters, "name", "");
     return ReferenceMethod{name};
   });

  config_parser.register_method_parser(
   "LOOKBACK",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod any_series_method) -> jsoncons::json {
     auto serialized_method = jsoncons::json::null();

     auto lookback_method =
      series_method_cast<LookbackMethod<AnySeriesMethod>>(any_series_method);
     if(lookback_method) {
       serialized_method = jsoncons::json{};
       serialized_method["period"] = lookback_method->period();
       serialized_method["source"] =
        config_parser.serialize_method(lookback_method->source());
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     const auto period = parameters.at("period").as<std::size_t>();
     const auto source_method = parse_method_from_param_or(
      config_parser, parameters, "source", CloseMethod{});
     return LookbackMethod{source_method, period};
   });

  config_parser.register_method_parser(
   "SELECT_OUTPUT",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod any_series_method) -> jsoncons::json {
     auto serialized_method = jsoncons::json::null();

     auto output_by_name_method =
      series_method_cast<SelectOutputMethod<AnySeriesMethod>>(
       any_series_method);
     if(output_by_name_method) {
       serialized_method = jsoncons::json{};
       serialized_method["name"] = [&]() -> std::string {
         switch(output_by_name_method->output()) {
         case SeriesOutput::MacdLine:
           return "macd-line";
         case SeriesOutput::SignalLine:
           return "signal-line";
         case SeriesOutput::Histogram:
           return "histogram";
         case SeriesOutput::KPercent:
           return "k-percent";
         case SeriesOutput::DPercent:
           return "d-percent";
         case SeriesOutput::MiddleBand:
           return "middle-band";
         case SeriesOutput::UpperBand:
           return "upper-band";
         case SeriesOutput::LowerBand:
           return "lower-band";
         default:
           return "default";
         }
       }();

       serialized_method["source"] =
        config_parser.serialize_method(output_by_name_method->source());
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     const auto name = get_param_or<std::string>(parameters, "name", "default");
     const auto output = [&]() -> SeriesOutput {
       if(name == "macd-line") {
         return SeriesOutput::MacdLine;
       } else if(name == "signal-line") {
         return SeriesOutput::SignalLine;
       } else if(name == "histogram") {
         return SeriesOutput::Histogram;
       } else if(name == "k-percent") {
         return SeriesOutput::KPercent;
       } else if(name == "d-percent") {
         return SeriesOutput::DPercent;
       } else if(name == "middle-band") {
         return SeriesOutput::MiddleBand;
       } else if(name == "upper-band") {
         return SeriesOutput::UpperBand;
       } else if(name == "lower-band") {
         return SeriesOutput::LowerBand;
       } else {
         return static_cast<SeriesOutput>(-1);
       }
     }();

     const auto source_method = parse_method_from_param_or(
      config_parser, parameters, "source", CloseMethod{});

     return SelectOutputMethod{source_method, output};
   });

  config_parser.register_method_parser(
   "ABS_DIFF",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod any_series_method) -> jsoncons::json {
     return serialize_binary_function_method<AbsDiffMethod>(
      config_parser, any_series_method, "minuend", "subtrahend");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     return parse_binary_function_method<AbsDiffMethod>(
      config_parser, parameters, "minuend", "subtrahend");
   });

  config_parser.register_method_parser(
   "BB",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod any_series_method) -> jsoncons::json {
     auto serialized_method = jsoncons::json::null();

     auto bb_method =
      series_method_cast<BbMethod<AnySeriesMethod>>(any_series_method);
     if(bb_method) {
       serialized_method = jsoncons::json{};
       serialized_method["maType"] =
        [](BbMaType ma_type) static -> std::string {
         switch(ma_type) {
         case BbMaType::Sma:
           return "SMA";
         case BbMaType::Ema:
           return "EMA";
         case BbMaType::Wma:
           return "WMA";
         case BbMaType::Rma:
           return "RMA";
         case BbMaType::Hma:
           return "HMA";
         default:
           const auto error_message =
            std::format("Unknown BB.maType: {}", static_cast<int>(ma_type));
           throw std::invalid_argument{error_message};
         }
       }(bb_method->ma_type());

       serialized_method["maSource"] =
        config_parser.serialize_method(bb_method->ma_source());
       serialized_method["period"] = bb_method->period();
       serialized_method["stddev"] = bb_method->stddev();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const jsoncons::json& parameters) -> AnySeriesMethod {
     auto ma_type = BbMaType::Sma;
     const auto param_ma_type =
      get_param_or<std::string>(parameters, "maType", "SMA");

     if(param_ma_type == "SMA") {
       ma_type = BbMaType::Sma;
     } else if(param_ma_type == "EMA") {
       ma_type = BbMaType::Ema;
     } else if(param_ma_type == "WMA") {
       ma_type = BbMaType::Wma;
     } else if(param_ma_type == "RMA") {
       ma_type = BbMaType::Rma;
     } else if(param_ma_type == "HMA") {
       ma_type = BbMaType::Hma;
     } else {
       const auto error_message =
        std::format("Error BB.maType: Unknown maType {}", param_ma_type);
       throw std::invalid_argument{error_message};
     }

     const auto ma_source_method = parse_method_from_param_or(
      config_parser, parameters, "maSource", CloseMethod{});
     const auto period = get_param_or(parameters, "period", std::size_t{20});
     const auto stddev = get_param_or(parameters, "stddev", 2.0);

     const auto bb_method = BbMethod{ma_type, ma_source_method, period, stddev};

     return bb_method;
   });

  config_parser.register_method_parser(
   "MACD",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod any_series_method) -> jsoncons::json {
     auto serialized_method = jsoncons::json::null();

     auto macd_method =
      series_method_cast<MacdMethod<AnySeriesMethod>>(any_series_method);
     if(macd_method) {
       serialized_method = jsoncons::json{};
       serialized_method["fast"] = macd_method->fast_period();
       serialized_method["slow"] = macd_method->slow_period();
       serialized_method["signal"] = macd_method->signal_period();
       serialized_method["source"] =
        config_parser.serialize_method(macd_method->source());
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const jsoncons::json& parameters) -> AnySeriesMethod {
     const auto fast = get_param_or<std::size_t>(parameters, "fast", 12);
     const auto slow = get_param_or<std::size_t>(parameters, "slow", 26);
     const auto signal = get_param_or<std::size_t>(parameters, "signal", 9);
     const auto source_method = parse_method_from_param_or(
      config_parser, parameters, "source", CloseMethod{});

     const auto macd_method = MacdMethod{source_method, fast, slow, signal};
     return macd_method;
   });

  config_parser.register_method_parser(
   "STOCH",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod any_series_method) -> jsoncons::json {
     auto serialized_method = jsoncons::json::null();

     auto stoch_method = series_method_cast<StochMethod>(any_series_method);
     if(stoch_method) {
       serialized_method = jsoncons::json{};
       serialized_method["kPeriod"] = stoch_method->k_period();
       serialized_method["kSmooth"] = stoch_method->k_smooth();
       serialized_method["dPeriod"] = stoch_method->d_period();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const jsoncons::json& parameters) -> AnySeriesMethod {
     const auto k_period = get_param_or<std::size_t>(parameters, "kPeriod", 5);
     const auto k_smooth = get_param_or<std::size_t>(parameters, "kSmooth", 3);
     const auto d_period = get_param_or<std::size_t>(parameters, "dPeriod", 3);

     const auto stoch_method = StochMethod{k_period, k_smooth, d_period};
     return stoch_method;
   });

  config_parser.register_method_parser(
   "STOCH_RSI",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod any_series_method) -> jsoncons::json {
     auto serialized_method = jsoncons::json::null();

     auto stoch_rsi_method =
      series_method_cast<StochRsiMethod<AnySeriesMethod>>(any_series_method);
     if(stoch_rsi_method) {
       serialized_method = jsoncons::json{};
       serialized_method["rsiSource"] =
        config_parser.serialize_method(stoch_rsi_method->rsi_source());
       serialized_method["rsiPeriod"] = stoch_rsi_method->rsi_period();
       serialized_method["kPeriod"] = stoch_rsi_method->k_period();
       serialized_method["kSmooth"] = stoch_rsi_method->k_smooth();
       serialized_method["dPeriod"] = stoch_rsi_method->d_period();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const jsoncons::json& parameters) -> AnySeriesMethod {
     const auto rsi_source_method = parse_method_from_param_or(
      config_parser, parameters, "rsiSource", CloseMethod{});
     const auto rsi_period =
      get_param_or<std::size_t>(parameters, "rsiPeriod", 14);
     const auto k_period = get_param_or<std::size_t>(parameters, "kPeriod", 5);
     const auto k_smooth = get_param_or<std::size_t>(parameters, "kSmooth", 3);
     const auto d_period = get_param_or<std::size_t>(parameters, "dPeriod", 3);

     const auto stoch_rsi_method = StochRsiMethod{
      rsi_source_method, rsi_period, k_period, k_smooth, d_period};
     return stoch_rsi_method;
   });

  config_parser.register_method_parser(
   "ADD",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod& any_series_method) -> jsoncons::json {
     return serialize_binary_function_method<AddMethod>(
      config_parser, any_series_method, "augend", "addend");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     return parse_binary_function_method<AddMethod>(
      config_parser, parameters, "augend", "addend");
   });
  config_parser.register_method_parser(
   "SUBTRACT",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod& any_series_method) -> jsoncons::json {
     return serialize_binary_function_method<SubtractMethod>(
      config_parser, any_series_method, "minuend", "subtrahend");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     return parse_binary_function_method<SubtractMethod>(
      config_parser, parameters, "minuend", "subtrahend");
   });
  config_parser.register_method_parser(
   "MULTIPLY",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod& any_series_method) -> jsoncons::json {
     return serialize_binary_function_method<MultiplyMethod>(
      config_parser, any_series_method, "multiplicand", "multiplier");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     return parse_binary_function_method<MultiplyMethod>(
      config_parser, parameters, "multiplicand", "multiplier");
   });
  config_parser.register_method_parser(
   "DIVIDE",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod& any_series_method) -> jsoncons::json {
     return serialize_binary_function_method<DivideMethod>(
      config_parser, any_series_method, "dividend", "divisor");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     return parse_binary_function_method<DivideMethod>(
      config_parser, parameters, "dividend", "divisor");
   });
  config_parser.register_method_parser(
   "NEGATE",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod& any_series_method) -> jsoncons::json {
     return serialize_unary_function_method<NegateMethod>(
      config_parser, any_series_method, "operand");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     return parse_unary_function_method<NegateMethod>(
      config_parser, parameters, "operand");
   });
  config_parser.register_method_parser(
   "PERCENTAGE",
   [](const ConfigParser& config_parser,
      const AnySeriesMethod& any_series_method) -> jsoncons::json {
     auto serialized_method = jsoncons::json::null();

     auto percentage_method =
      series_method_cast<PercentageMethod<AnySeriesMethod>>(any_series_method);
     if(percentage_method) {
       serialized_method = jsoncons::json{};

       serialized_method["base"] =
        config_parser.serialize_method(percentage_method->base());
       serialized_method["percent"] = percentage_method->percent();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     auto base_method = parse_method_from_param_or(
      config_parser, parameters, "base", CloseMethod{});
     auto percent = get_param_or<double>(parameters, "percent", 100.0);

     return PercentageMethod{base_method, percent};
   });
  config_parser.register_filter_parser(
   "ALL_OF", serialize_all_of_filter, parse_all_of_filter);
  config_parser.register_filter_parser(
   "ANY_OF", serialize_any_of_filter, parse_any_of_filter);
  config_parser.register_filter_parser(
   "CROSSUNDER", serialize_crossunder_filter, parse_crossunder_filter);
  config_parser.register_filter_parser(
   "CROSSOVER", serialize_crossover_filter, parse_crossover_filter);
  config_parser.register_filter_parser(
   "GREATER_THAN",
   serialize_comparison_filter<GreaterThanMethod>,
   parse_comparison_filter<GreaterThanMethod>);
  config_parser.register_filter_parser(
   "LESS_THAN",
   serialize_comparison_filter<LessThanMethod>,
   parse_comparison_filter<LessThanMethod>);
  config_parser.register_filter_parser(
   "GREATER_EQUAL",
   serialize_comparison_filter<GreaterEqualMethod>,
   parse_comparison_filter<GreaterEqualMethod>);
  config_parser.register_filter_parser(
   "LESS_EQUAL",
   serialize_comparison_filter<LessEqualMethod>,
   parse_comparison_filter<LessEqualMethod>);
  config_parser.register_filter_parser("EQUAL",
                                       serialize_comparison_filter<EqualMethod>,
                                       parse_comparison_filter<EqualMethod>);
  config_parser.register_filter_parser(
   "NOT_EQUAL",
   serialize_comparison_filter<NotEqualMethod>,
   parse_comparison_filter<NotEqualMethod>);

  config_parser.register_filter_parser(
   "TRUE",
   [](const ConfigParser& config_parser,
      const AnyConditionMethod& condition_method) -> jsoncons::json {
     auto serialized_filter = jsoncons::json{};

     const auto true_filter =
      condition_method_cast<TrueMethod>(condition_method);

     if(true_filter) {
       serialized_filter["filter"] = "TRUE";
     }

     return serialized_filter;
   },
   [](ConfigParser::Parser, const jsoncons::json&) -> AnyConditionMethod {
     return TrueMethod{};
   });

  config_parser.register_filter_parser(
   "FALSE",
   [](const ConfigParser& config_parser,
      const AnyConditionMethod& condition_method) -> jsoncons::json {
     auto serialized_filter = jsoncons::json{};

     const auto false_filter =
      condition_method_cast<FalseMethod>(condition_method);
     if(false_filter) {
       serialized_filter["filter"] = "FALSE";
     }
     return serialized_filter;
   },
   [](ConfigParser::Parser, const jsoncons::json&) -> AnyConditionMethod {
     return FalseMethod{};
   });

  config_parser.register_filter_parser(
   "AND",
   [](const ConfigParser& config_parser,
      const AnyConditionMethod& condition_method) -> jsoncons::json {
     return serialize_binary_function_filter<AndMethod>(
      config_parser, condition_method, "firstCondition", "secondCondition");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     return parse_binary_function_filter<AndMethod>(
      config_parser, parameters, "firstCondition", "secondCondition");
   });

  config_parser.register_filter_parser(
   "OR",
   [](const ConfigParser& config_parser,
      const AnyConditionMethod& condition_method) -> jsoncons::json {
     return serialize_binary_function_filter<OrMethod>(
      config_parser, condition_method, "firstCondition", "secondCondition");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     return parse_binary_function_filter<OrMethod>(
      config_parser, parameters, "firstCondition", "secondCondition");
   });

  config_parser.register_filter_parser(
   "NOT",
   [](const ConfigParser& config_parser,
      const AnyConditionMethod& condition_method) -> jsoncons::json {
     return serialize_unary_function_filter<NotMethod>(
      config_parser, condition_method, "condition");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     return parse_unary_function_filter<NotMethod>(
      config_parser, parameters, "condition");
   });

  config_parser.register_filter_parser(
   "XOR",
   [](const ConfigParser& config_parser,
      const AnyConditionMethod& condition_method) -> jsoncons::json {
     return serialize_binary_function_filter<XorMethod>(
      config_parser, condition_method, "firstCondition", "secondCondition");
   },
   [](ConfigParser::Parser config_parser, const jsoncons::json& parameters) {
     return parse_binary_function_filter<XorMethod>(
      config_parser, parameters, "firstCondition", "secondCondition");
   });

  return config_parser;
}

} // namespace pludux
