module;

#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <nlohmann/json.hpp>

export module pludux:config_parser;

import :screener;
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

    auto parse_method(this auto& self, const nlohmann::json& config)
     -> screener::ScreenerMethod
    {
      return self.config_parser_.parse_method(config);
    }

    auto parse_filter(this auto& self, const nlohmann::json& config)
     -> screener::ScreenerFilter
    {
      return self.config_parser_.parse_filter(config);
    }

  private:
    ConfigParser& config_parser_;
  };

  friend Parser;

  using FilterSerialize = std::function<
   auto(const ConfigParser&, const screener::ScreenerFilter&)->nlohmann::json>;

  using FilterDeserialize = std::function<
   auto(ConfigParser::Parser, const nlohmann::json&)->screener::ScreenerFilter>;

  using MethodSerialize = std::function<
   auto(const ConfigParser&, const screener::ScreenerMethod&)->nlohmann::json>;

  using MethodDeserialize = std::function<
   auto(ConfigParser::Parser, const nlohmann::json&)->screener::ScreenerMethod>;

  ConfigParser()
  : filter_parsers_{}
  , method_parsers_{}
  {
  }

  void register_filter_parser(this auto& self,
                              const std::string& filter_name,
                              const FilterSerialize& filter_serialize,
                              const FilterDeserialize& filter_deserialize)
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

  auto parse_filter(this auto& self, const nlohmann::json& config)
   -> screener::ScreenerFilter
  {
    if(config.is_boolean()) {
      if(config.get<bool>()) {
        return screener::TrueFilter{};
      }

      return screener::FalseFilter{};
    }
    const auto& filter_parsers = self.filter_parsers_;
    const auto filter = config.at("filter").get<std::string>();

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

  auto serialize_filter(this const auto& self,
                        const screener::ScreenerFilter& filter)
   -> nlohmann::json
  {
    auto serialized_filter = nlohmann::json{};

    for(const auto& [filter_name, filter_parser] : self.filter_parsers_) {
      const auto& [filter_serialize, _] = filter_parser;
      serialized_filter = filter_serialize(self, filter);
      if(!serialized_filter.empty()) {
        if(!serialized_filter.is_object()) {
          const auto error_message = std::format(
           "Filter {} serialization must return an object", filter_name);
          throw std::invalid_argument{error_message};
        }

        if(!serialized_filter.contains("filter")) {
          serialized_filter["filter"] = filter_name;
        } else if(serialized_filter.at("filter").get<std::string>() !=
                  filter_name) {
          const auto error_message = std::format(
           "Filter {} serialization must return 'filter' key with value {}",
           filter_name,
           filter_name);
          throw std::invalid_argument{error_message};
        }
        break;
      }
    }

    return serialized_filter;
  }

  auto parse_method(this auto& self, const nlohmann::json& config)
   -> screener::ScreenerMethod
  {
    if(config.is_number()) {
      return screener::ValueMethod{config.get<double>()};
    }

    if(config.is_string()) {
      const auto named_method = config.get<std::string>();
      const auto expanded_method = nlohmann::json{{"method", named_method}};

      return self.parse_method(expanded_method);
    }

    auto config_method = config;
    const auto method = config.at("method").get<std::string>();

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

  auto serialize_method(this const auto& self,
                        const screener::ScreenerMethod& method)
   -> nlohmann::json
  {
    auto serialized_method = nlohmann::json{};

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
  std::unordered_map<std::string, std::pair<FilterSerialize, FilterDeserialize>>
   filter_parsers_;
  std::unordered_map<std::string, std::pair<MethodSerialize, MethodDeserialize>>
   method_parsers_;
};

auto make_default_registered_config_parser(
 std::shared_ptr<screener::MethodRegistry> method_registry = nullptr)
 -> ConfigParser;

} // namespace pludux

namespace pludux {

template<typename T>
static auto get_param_or(const nlohmann::json& parameters,
                         const std::string& key,
                         const T& default_value) -> T
{
  return parameters.contains(key) ? parameters.at(key).get<T>() : default_value;
}

static auto
parse_method_from_param_or(ConfigParser::Parser config_parser,
                           const nlohmann::json& parameters,
                           const std::string& key,
                           const screener::ScreenerMethod& default_value)
 -> screener::ScreenerMethod
{
  if(!parameters.contains(key)) {
    return default_value;
  }

  return config_parser.parse_method(parameters.at(key));
}

template<typename TMethod>
static auto parse_ta_with_period_method(ConfigParser::Parser config_parser,
                                        const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  auto ta_method = TMethod{};

  if(parameters.contains("period")) {
    ta_method.period(parameters.at("period").get<std::size_t>());
  }

  if(parameters.contains("input")) {
    ta_method.input(config_parser.parse_method(parameters.at("input")));
  }

  return ta_method;
}

template<typename TMethod>
static auto
serialize_ta_with_period_method(const ConfigParser& config_parser,
                                const screener::ScreenerMethod& method)
 -> nlohmann::json
{
  auto serialized_method = nlohmann::json{};

  auto ta_method = screener_method_cast<TMethod>(method);

  if(ta_method) {
    serialized_method["period"] = ta_method->period();
    serialized_method["input"] =
     config_parser.serialize_method(ta_method->input());
  }

  return serialized_method;
}

template<typename T>
static auto serialize_ohlcv_method(const ConfigParser& config_parser,
                                   const screener::ScreenerMethod& method)
 -> nlohmann::json
{
  auto serialized_method = nlohmann::json{};
  auto ohlcv_method = screener_method_cast<T>(method);
  if(ohlcv_method) {
    serialized_method = nlohmann::json::object();
  }
  return serialized_method;
};

template<typename T>
static auto parse_ohlcv_method(ConfigParser::Parser config_parser,
                               const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  auto ohlcv_method = T{};
  return ohlcv_method;
}

static auto serialize_value_method(const ConfigParser& config_parser,
                                   const screener::ScreenerMethod& method)
 -> nlohmann::json
{
  auto serialized_method = nlohmann::json{};

  auto value_method = screener_method_cast<screener::ValueMethod>(method);

  if(value_method) {
    serialized_method["value"] = value_method->value();
  }

  return serialized_method;
}

static auto deserialize_value_method(ConfigParser::Parser config_parser,
                                     const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto value = parameters.at("value").get<double>();
  return screener::ValueMethod{value};
}

static auto parse_data_method(ConfigParser::Parser config_parser,
                              const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto field = parameters.at("field").get<std::string>();

  const auto field_method = screener::DataMethod{field};

  return field_method;
}

static auto serialize_data_method(const ConfigParser& config_parser,
                                  const screener::ScreenerMethod& method)
 -> nlohmann::json
{
  auto serialized_method = nlohmann::json{};

  auto data_method = screener_method_cast<screener::DataMethod>(method);

  if(data_method) {
    serialized_method["field"] = data_method->field();
  }

  return serialized_method;
}

static auto parse_atr_method(ConfigParser::Parser config_parser,
                             const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  auto atr_method = screener::AtrMethod{};

  if(parameters.contains("period")) {
    atr_method.period(parameters.at("period").get<std::size_t>());
  }

  if(parameters.contains("multiplier")) {
    atr_method.multiplier(parameters.at("multiplier").get<double>());
  }

  if(parameters.contains("high")) {
    atr_method.high(config_parser.parse_method(parameters.at("high")));
  }

  if(parameters.contains("low")) {
    atr_method.low(config_parser.parse_method(parameters.at("low")));
  }

  if(parameters.contains("close")) {
    atr_method.close(config_parser.parse_method(parameters.at("close")));
  }

  return atr_method;
}

static auto serialize_atr_method(const ConfigParser& config_parser,
                                 const screener::ScreenerMethod& method)
 -> nlohmann::json
{
  auto serialized_method = nlohmann::json{};

  auto atr_method = screener_method_cast<screener::AtrMethod>(method);
  if(atr_method) {
    serialized_method["period"] = atr_method->period();
    serialized_method["multiplier"] = atr_method->multiplier();
    serialized_method["high"] =
     config_parser.serialize_method(atr_method->high());
    serialized_method["low"] =
     config_parser.serialize_method(atr_method->low());
    serialized_method["close"] =
     config_parser.serialize_method(atr_method->close());
  }

  return serialized_method;
}

static auto serialize_kc_method(const ConfigParser& config_parser,
                                const screener::ScreenerMethod& method)
 -> nlohmann::json
{
  auto serialized_method = nlohmann::json{};

  auto kc_method = screener_method_cast<screener::KcMethod>(method);
  if(kc_method) {
    serialized_method["ma"] = config_parser.serialize_method(kc_method->ma());
    serialized_method["range"] =
     config_parser.serialize_method(kc_method->range());
    serialized_method["multiplier"] = kc_method->multiplier();
  }

  return serialized_method;
}

static auto parse_kc_method(ConfigParser::Parser config_parser,
                            const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto ma_method = config_parser.parse_method(parameters.at("ma"));
  const auto range_method = config_parser.parse_method(parameters.at("range"));
  const auto multiplier = parameters.at("multiplier").get<double>();

  const auto kc_method =
   screener::KcMethod{ma_method, range_method, multiplier};
  return kc_method;
}

template<typename T>
static auto parse_binary_function_method(ConfigParser::Parser config_parser,
                                         const nlohmann::json& parameters,
                                         const std::string& first_operand_key,
                                         const std::string& second_operand_key)
 -> screener::ScreenerMethod
{
  const auto first_operand =
   config_parser.parse_method(parameters.at(first_operand_key));
  const auto second_operand =
   config_parser.parse_method(parameters.at(second_operand_key));

  const auto binary_function_method = T{first_operand, second_operand};
  return binary_function_method;
}

template<typename T>
static auto
serialize_binary_function_method(const ConfigParser& config_parser,
                                 const screener::ScreenerMethod& method,
                                 const std::string& first_operand_key,
                                 const std::string& second_operand_key)
 -> nlohmann::json
{
  auto serialized_method = nlohmann::json{};

  auto binary_function_method = screener_method_cast<T>(method);
  if(binary_function_method) {
    serialized_method[first_operand_key] =
     config_parser.serialize_method(binary_function_method->operand1());
    serialized_method[second_operand_key] =
     config_parser.serialize_method(binary_function_method->operand2());
  }

  return serialized_method;
}

template<typename T>
static auto parse_unary_function_method(ConfigParser::Parser config_parser,
                                        const nlohmann::json& parameters,
                                        const std::string& operand_key)
 -> screener::ScreenerMethod
{
  const auto operand = config_parser.parse_method(parameters.at(operand_key));
  const auto unary_function_method = T{operand};
  return unary_function_method;
}

template<typename T>
static auto
serialize_unary_function_method(const ConfigParser& config_parser,
                                const screener::ScreenerMethod& method,
                                const std::string& operand_key)
 -> nlohmann::json
{
  auto serialized_method = nlohmann::json{};

  auto unary_function_method = screener_method_cast<T>(method);
  if(unary_function_method) {
    serialized_method[operand_key] =
     config_parser.serialize_method(unary_function_method->operand());
  }

  return serialized_method;
}

template<typename T>
static auto parse_divergence_method(ConfigParser::Parser config_parser,
                                    const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  auto divergence_method = T{};

  if(parameters.contains("pivotRange")) {
    divergence_method.pivot_range(
     parameters.at("pivotRange").get<std::size_t>());
  }

  if(parameters.contains("lookbackRange")) {
    divergence_method.lookback_range(
     parameters.at("lookbackRange").get<std::size_t>());
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
                                        const screener::ScreenerMethod& method)
 -> nlohmann::json
{
  auto serialized_method = nlohmann::json{};

  auto divergence_method = screener_method_cast<T>(method);
  if(divergence_method) {
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
                                         const nlohmann::json& parameters,
                                         const std::string& first_operand_key,
                                         const std::string& second_operand_key)
 -> screener::ScreenerFilter
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
                                 const screener::ScreenerFilter& filter,
                                 const std::string& first_operand_key,
                                 const std::string& second_operand_key)
 -> nlohmann::json
{
  auto serialized_filter = nlohmann::json{};

  auto binary_function_filter = screener_filter_cast<T>(filter);
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
                                        const nlohmann::json& parameters,
                                        const std::string& operand_key)
 -> screener::ScreenerFilter
{
  const auto operand = config_parser.parse_filter(parameters.at(operand_key));
  const auto unary_function_filter = T{operand};
  return unary_function_filter;
}

template<typename T>
static auto
serialize_unary_function_filter(const ConfigParser& config_parser,
                                const screener::ScreenerFilter& filter,
                                const std::string& operand_key)
 -> nlohmann::json
{
  auto serialized_filter = nlohmann::json{};

  auto unary_function_filter = screener_filter_cast<T>(filter);
  if(unary_function_filter) {
    serialized_filter[operand_key] =
     config_parser.serialize_filter(unary_function_filter->other_condition());
  }

  return serialized_filter;
}

template<typename T>
static auto parse_comparison_filter(ConfigParser::Parser config_parser,
                                    const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto target = config_parser.parse_method(parameters.at("target"));
  auto threshold = config_parser.parse_method(parameters.at("threshold"));
  return T{target, threshold};
}

template<typename T>
static auto serialize_comparison_filter(const ConfigParser& config_parser,
                                        const screener::ScreenerFilter& filter)
 -> nlohmann::json
{
  auto serialized_filter = nlohmann::json{};

  auto comparison_filter = screener_filter_cast<T>(filter);
  if(comparison_filter) {
    serialized_filter["target"] =
     config_parser.serialize_method(comparison_filter->target());
    serialized_filter["threshold"] =
     config_parser.serialize_method(comparison_filter->threshold());
  }

  return serialized_filter;
}

static auto parse_all_of_filter(ConfigParser::Parser config_parser,
                                const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  if(!parameters.contains("conditions")) {
    throw std::invalid_argument{"ALL_OF: 'conditions' is not found"};
  }

  auto conditions = std::vector<screener::ScreenerFilter>{};
  for(const auto& filter : parameters.at("conditions")) {
    conditions.push_back(config_parser.parse_filter(filter));
  }
  return screener::AllOfFilter{conditions};
}

static auto serialize_all_of_filter(const ConfigParser& config_parser,
                                    const screener::ScreenerFilter& filter)
 -> nlohmann::json
{
  auto serialized_filter = nlohmann::json{};

  auto all_of_filter = screener_filter_cast<screener::AllOfFilter>(filter);
  if(all_of_filter) {
    auto conditions = nlohmann::json::array();
    for(const auto& condition : all_of_filter->conditions()) {
      conditions.push_back(config_parser.serialize_filter(condition));
    }
    serialized_filter["conditions"] = conditions;
  }

  return serialized_filter;
}

static auto parse_any_of_filter(ConfigParser::Parser config_parser,
                                const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  if(!parameters.contains("conditions")) {
    throw std::invalid_argument{"ANY_OF: 'conditions' is not found"};
  }
  auto conditions = std::vector<screener::ScreenerFilter>{};
  for(const auto& filter : parameters.at("conditions")) {
    conditions.push_back(config_parser.parse_filter(filter));
  }
  return screener::AnyOfFilter{conditions};
}

static auto serialize_any_of_filter(const ConfigParser& config_parser,
                                    const screener::ScreenerFilter& filter)
 -> nlohmann::json
{
  auto serialized_filter = nlohmann::json{};

  auto any_of_filter = screener_filter_cast<screener::AnyOfFilter>(filter);
  if(any_of_filter) {
    auto conditions = nlohmann::json::array();
    for(const auto& condition : any_of_filter->conditions()) {
      conditions.push_back(config_parser.serialize_filter(condition));
    }
    serialized_filter["conditions"] = conditions;
  }

  return serialized_filter;
}

static auto parse_crossunder_filter(ConfigParser::Parser config_parser,
                                    const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto signal = config_parser.parse_method(parameters.at("signal"));
  auto reference = config_parser.parse_method(parameters.at("reference"));
  return screener::CrossunderFilter{signal, reference};
}

static auto serialize_crossunder_filter(const ConfigParser& config_parser,
                                        const screener::ScreenerFilter& filter)
 -> nlohmann::json
{
  auto serialized_filter = nlohmann::json{};

  auto crossunder_filter =
   screener_filter_cast<screener::CrossunderFilter>(filter);
  if(crossunder_filter) {
    serialized_filter["signal"] =
     config_parser.serialize_method(crossunder_filter->signal());
    serialized_filter["reference"] =
     config_parser.serialize_method(crossunder_filter->reference());
  }

  return serialized_filter;
}

static auto parse_crossover_filter(ConfigParser::Parser config_parser,
                                   const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto signal = config_parser.parse_method(parameters.at("signal"));
  auto reference = config_parser.parse_method(parameters.at("reference"));
  return screener::CrossoverFilter{signal, reference};
}

static auto serialize_crossover_filter(const ConfigParser& config_parser,
                                       const screener::ScreenerFilter& filter)
 -> nlohmann::json
{
  auto serialized_filter = nlohmann::json{};

  auto crossover_filter =
   screener_filter_cast<screener::CrossoverFilter>(filter);
  if(crossover_filter) {
    serialized_filter["signal"] =
     config_parser.serialize_method(crossover_filter->signal());
    serialized_filter["reference"] =
     config_parser.serialize_method(crossover_filter->reference());
  }

  return serialized_filter;
}

auto make_default_registered_config_parser(
 std::shared_ptr<screener::MethodRegistry> method_registry) -> ConfigParser
{
  ConfigParser config_parser;

  config_parser.register_method_parser(
   "VALUE", serialize_value_method, deserialize_value_method);

  config_parser.register_method_parser(
   "DATA", serialize_data_method, parse_data_method);

  config_parser.register_method_parser(
   "OPEN",
   serialize_ohlcv_method<screener::OpenMethod>,
   parse_ohlcv_method<screener::OpenMethod>);

  config_parser.register_method_parser(
   "HIGH",
   serialize_ohlcv_method<screener::HighMethod>,
   parse_ohlcv_method<screener::HighMethod>);

  config_parser.register_method_parser(
   "LOW",
   serialize_ohlcv_method<screener::LowMethod>,
   parse_ohlcv_method<screener::LowMethod>);

  config_parser.register_method_parser(
   "CLOSE",
   serialize_ohlcv_method<screener::CloseMethod>,
   parse_ohlcv_method<screener::CloseMethod>);

  config_parser.register_method_parser(
   "VOLUME",
   serialize_ohlcv_method<screener::VolumeMethod>,
   parse_ohlcv_method<screener::VolumeMethod>);

  config_parser.register_method_parser(
   "CHANGES",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod screener_method) -> nlohmann::json {
     auto serialized_method = nlohmann::json{};

     auto changes_method =
      screener_method_cast<screener::ChangesMethod>(screener_method);
     if(changes_method) {
       serialized_method["input"] =
        config_parser.serialize_method(changes_method->input());
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     auto changes_method = screener::ChangesMethod{};

     if(parameters.contains("input")) {
       changes_method.input(config_parser.parse_method(parameters.at("input")));
     }

     return changes_method;
   });

  config_parser.register_method_parser(
   "SMA",
   serialize_ta_with_period_method<screener::SmaMethod>,
   parse_ta_with_period_method<screener::SmaMethod>);

  config_parser.register_method_parser(
   "EMA",
   serialize_ta_with_period_method<screener::EmaMethod>,
   parse_ta_with_period_method<screener::EmaMethod>);

  config_parser.register_method_parser(
   "WMA",
   serialize_ta_with_period_method<screener::WmaMethod>,
   parse_ta_with_period_method<screener::WmaMethod>);

  config_parser.register_method_parser(
   "RMA",
   serialize_ta_with_period_method<screener::RmaMethod>,
   parse_ta_with_period_method<screener::RmaMethod>);

  config_parser.register_method_parser(
   "HMA",
   serialize_ta_with_period_method<screener::HmaMethod>,
   parse_ta_with_period_method<screener::HmaMethod>);

  config_parser.register_method_parser(
   "RSI",
   serialize_ta_with_period_method<screener::RsiMethod>,
   parse_ta_with_period_method<screener::RsiMethod>);

  config_parser.register_method_parser(
   "ROC",
   serialize_ta_with_period_method<screener::RocMethod>,
   parse_ta_with_period_method<screener::RocMethod>);

  config_parser.register_method_parser(
   "RVOL",
   serialize_ta_with_period_method<screener::RvolMethod>,
   parse_ta_with_period_method<screener::RvolMethod>);

  config_parser.register_method_parser(
   "ATR", serialize_atr_method, parse_atr_method);

  config_parser.register_method_parser(
   "KC", serialize_kc_method, parse_kc_method);

  config_parser.register_method_parser(
   "REFERENCE",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod screener_method) -> nlohmann::json {
     auto serialized_method = nlohmann::json{};

     auto reference_method =
      screener_method_cast<screener::ReferenceMethod>(screener_method);
     if(reference_method) {
       serialized_method["name"] = reference_method->name();
     }

     return serialized_method;
   },
   [method_registry](ConfigParser::Parser config_parser,
                     const nlohmann::json& parameters) {
     const auto name = get_param_or<std::string>(parameters, "name", "");
     return screener::ReferenceMethod{method_registry, name};
   });

  config_parser.register_method_parser(
   "LOOKBACK",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod screener_method) -> nlohmann::json {
     auto serialized_method = nlohmann::json{};

     auto lookback_method =
      screener_method_cast<screener::LookbackMethod>(screener_method);
     if(lookback_method) {
       serialized_method["period"] = lookback_method->period();
       serialized_method["source"] =
        config_parser.serialize_method(lookback_method->source());
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     const auto period = parameters.at("period").get<std::size_t>();
     const auto source_method = parse_method_from_param_or(
      config_parser, parameters, "source", screener::CloseMethod{});
     return screener::LookbackMethod{source_method, period};
   });

  config_parser.register_method_parser(
   "OUTPUT_BY_NAME",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod screener_method) -> nlohmann::json {
     auto serialized_method = nlohmann::json{};

     auto output_by_name_method =
      screener_method_cast<screener::OutputByNameMethod>(screener_method);
     if(output_by_name_method) {
       serialized_method["name"] = [&]() -> std::string {
         switch(output_by_name_method->output()) {
         case OutputName::MacdLine:
           return "macd-macd";
         case OutputName::SignalLine:
           return "macd-signal";
         case OutputName::MacdHistogram:
           return "macd-histogram";
         case OutputName::StochasticK:
           return "percent-k";
         case OutputName::StochasticD:
           return "percent-d";
         case OutputName::MiddleBand:
           return "middle-band";
         case OutputName::UpperBand:
           return "upper-band";
         case OutputName::LowerBand:
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
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     const auto name = get_param_or<std::string>(parameters, "name", "default");
     const auto output = [&]() -> OutputName {
       if(name == "macd-macd") {
         return OutputName::MacdLine;
       } else if(name == "macd-signal") {
         return OutputName::SignalLine;
       } else if(name == "macd-histogram") {
         return OutputName::MacdHistogram;
       } else if(name == "percent-k") {
         return OutputName::StochasticK;
       } else if(name == "percent-d") {
         return OutputName::StochasticD;
       } else if(name == "middle-band") {
         return OutputName::MiddleBand;
       } else if(name == "upper-band") {
         return OutputName::UpperBand;
       } else if(name == "lower-band") {
         return OutputName::LowerBand;
       } else {
         return OutputName::Default;
       }
     }();

     const auto source_method = parse_method_from_param_or(
      config_parser, parameters, "source", screener::CloseMethod{});

     return screener::OutputByNameMethod{source_method, output};
   });

  config_parser.register_method_parser(
   "ABS_DIFF",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod screener_method) -> nlohmann::json {
     return serialize_binary_function_method<screener::AbsDiffMethod>(
      config_parser, screener_method, "minuend", "subtrahend");
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::AbsDiffMethod>(
      config_parser, parameters, "minuend", "subtrahend");
   });

  config_parser.register_method_parser(
   "BB",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod screener_method) -> nlohmann::json {
     auto serialized_method = nlohmann::json{};

     auto bb_method = screener_method_cast<screener::BbMethod>(screener_method);
     if(bb_method) {
       serialized_method["maType"] =
        [](screener::BbMethod::MaType ma_type) -> std::string {
         switch(ma_type) {
         case screener::BbMethod::MaType::sma:
           return "SMA";
         case screener::BbMethod::MaType::ema:
           return "EMA";
         case screener::BbMethod::MaType::wma:
           return "WMA";
         case screener::BbMethod::MaType::rma:
           return "RMA";
         case screener::BbMethod::MaType::hma:
           return "HMA";
         default:
           const auto error_message =
            std::format("Unknown BB.maType: {}", static_cast<int>(ma_type));
           throw std::invalid_argument{error_message};
         }
       }(bb_method->ma_type());

       serialized_method["input"] =
        config_parser.serialize_method(bb_method->input());
       serialized_method["period"] = bb_method->period();
       serialized_method["stddev"] = bb_method->stddev();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const nlohmann::json& parameters) -> screener::ScreenerMethod {
     auto ma_type = screener::BbMethod::MaType::sma;
     const auto param_ma_type =
      get_param_or<std::string>(parameters, "maType", "SMA");

     if(param_ma_type == "SMA") {
       ma_type = screener::BbMethod::MaType::sma;
     } else if(param_ma_type == "EMA") {
       ma_type = screener::BbMethod::MaType::ema;
     } else if(param_ma_type == "WMA") {
       ma_type = screener::BbMethod::MaType::wma;
     } else if(param_ma_type == "RMA") {
       ma_type = screener::BbMethod::MaType::rma;
     } else if(param_ma_type == "HMA") {
       ma_type = screener::BbMethod::MaType::hma;
     } else {
       const auto error_message =
        std::format("Error BB.maType: Unknown maType {}", param_ma_type);
       throw std::invalid_argument{error_message};
     }

     const auto input_method = parse_method_from_param_or(
      config_parser, parameters, "input", screener::CloseMethod{});
     const auto period = get_param_or(parameters, "period", std::size_t{20});
     const auto stddev = get_param_or(parameters, "stddev", 2.0);

     const auto bb_method =
      screener::BbMethod{ma_type, input_method, period, stddev};

     return bb_method;
   });

  config_parser.register_method_parser(
   "MACD",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod screener_method) -> nlohmann::json {
     auto serialized_method = nlohmann::json{};

     auto macd_method =
      screener_method_cast<screener::MacdMethod>(screener_method);
     if(macd_method) {
       serialized_method["fast"] = macd_method->fast_period();
       serialized_method["slow"] = macd_method->slow_period();
       serialized_method["signal"] = macd_method->signal_period();
       serialized_method["input"] =
        config_parser.serialize_method(macd_method->input());
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const nlohmann::json& parameters) -> screener::ScreenerMethod {
     const auto fast = get_param_or<std::size_t>(parameters, "fast", 12);
     const auto slow = get_param_or<std::size_t>(parameters, "slow", 26);
     const auto signal = get_param_or<std::size_t>(parameters, "signal", 9);
     const auto input_method = parse_method_from_param_or(
      config_parser, parameters, "input", screener::CloseMethod{});

     const auto macd_method =
      screener::MacdMethod{input_method, fast, slow, signal};
     return macd_method;
   });

  config_parser.register_method_parser(
   "STOCH",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod screener_method) -> nlohmann::json {
     auto serialized_method = nlohmann::json{};

     auto stoch_method =
      screener_method_cast<screener::StochMethod>(screener_method);
     if(stoch_method) {
       serialized_method["high"] =
        config_parser.serialize_method(stoch_method->high());
       serialized_method["low"] =
        config_parser.serialize_method(stoch_method->low());
       serialized_method["close"] =
        config_parser.serialize_method(stoch_method->close());
       serialized_method["kPeriod"] = stoch_method->k_period();
       serialized_method["kSmooth"] = stoch_method->k_smooth();
       serialized_method["dPeriod"] = stoch_method->d_period();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const nlohmann::json& parameters) -> screener::ScreenerMethod {
     const auto high_method = parse_method_from_param_or(
      config_parser, parameters, "high", screener::HighMethod{});

     const auto low_method = parse_method_from_param_or(
      config_parser, parameters, "low", screener::LowMethod{});

     const auto close_method = parse_method_from_param_or(
      config_parser, parameters, "close", screener::CloseMethod{});

     const auto k_period = get_param_or<std::size_t>(parameters, "kPeriod", 5);
     const auto k_smooth = get_param_or<std::size_t>(parameters, "kSmooth", 3);
     const auto d_period = get_param_or<std::size_t>(parameters, "dPeriod", 3);

     const auto stoch_method = screener::StochMethod{
      high_method, low_method, close_method, k_period, k_smooth, d_period};
     return stoch_method;
   });

  config_parser.register_method_parser(
   "STOCH_RSI",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod screener_method) -> nlohmann::json {
     auto serialized_method = nlohmann::json{};

     auto stoch_rsi_method =
      screener_method_cast<screener::StochRsiMethod>(screener_method);
     if(stoch_rsi_method) {
       serialized_method["rsiInput"] =
        config_parser.serialize_method(stoch_rsi_method->rsi_input());
       serialized_method["rsiPeriod"] = stoch_rsi_method->rsi_period();
       serialized_method["kPeriod"] = stoch_rsi_method->k_period();
       serialized_method["kSmooth"] = stoch_rsi_method->k_smooth();
       serialized_method["dPeriod"] = stoch_rsi_method->d_period();
     }

     return serialized_method;
   },
   [](ConfigParser::Parser config_parser,
      const nlohmann::json& parameters) -> screener::ScreenerMethod {
     const auto rsi_input_method = parse_method_from_param_or(
      config_parser, parameters, "rsiInput", screener::CloseMethod{});
     const auto rsi_period =
      get_param_or<std::size_t>(parameters, "rsiPeriod", 14);
     const auto k_period = get_param_or<std::size_t>(parameters, "kPeriod", 5);
     const auto k_smooth = get_param_or<std::size_t>(parameters, "kSmooth", 3);
     const auto d_period = get_param_or<std::size_t>(parameters, "dPeriod", 3);

     const auto stoch_rsi_method = screener::StochRsiMethod{
      rsi_input_method, rsi_period, k_period, k_smooth, d_period};
     return stoch_rsi_method;
   });

  config_parser.register_method_parser(
   "ADD",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod& screener_method) -> nlohmann::json {
     return serialize_binary_function_method<screener::AddMethod>(
      config_parser, screener_method, "augend", "addend");
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::AddMethod>(
      config_parser, parameters, "augend", "addend");
   });
  config_parser.register_method_parser(
   "SUBTRACT",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod& screener_method) -> nlohmann::json {
     return serialize_binary_function_method<screener::SubtractMethod>(
      config_parser, screener_method, "minuend", "subtrahend");
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::SubtractMethod>(
      config_parser, parameters, "minuend", "subtrahend");
   });
  config_parser.register_method_parser(
   "MULTIPLY",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod& screener_method) -> nlohmann::json {
     return serialize_binary_function_method<screener::MultiplyMethod>(
      config_parser, screener_method, "multiplicand", "multiplier");
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::MultiplyMethod>(
      config_parser, parameters, "multiplicand", "multiplier");
   });
  config_parser.register_method_parser(
   "DIVIDE",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod& screener_method) -> nlohmann::json {
     return serialize_binary_function_method<screener::DivideMethod>(
      config_parser, screener_method, "dividend", "divisor");
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::DivideMethod>(
      config_parser, parameters, "dividend", "divisor");
   });
  config_parser.register_method_parser(
   "NEGATE",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod& screener_method) -> nlohmann::json {
     return serialize_unary_function_method<screener::NegateMethod>(
      config_parser, screener_method, "operand");
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_unary_function_method<screener::NegateMethod>(
      config_parser, parameters, "operand");
   });
  config_parser.register_method_parser(
   "PERCENTAGE",
   [](const ConfigParser& config_parser,
      const screener::ScreenerMethod& screener_method) -> nlohmann::json {
     return serialize_binary_function_method<screener::PercentageMethod>(
      config_parser, screener_method, "total", "percent");
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::PercentageMethod>(
      config_parser, parameters, "total", "percent");
   });
  config_parser.register_method_parser(
   "BULLISH_DIVERGENCE",
   serialize_divergence_method<screener::BullishDivergenceMethod>,
   parse_divergence_method<screener::BullishDivergenceMethod>);
  config_parser.register_method_parser(
   "HIDDEN_BULLISH_DIVERGENCE",
   serialize_divergence_method<screener::HiddenBullishDivergenceMethod>,
   parse_divergence_method<screener::HiddenBullishDivergenceMethod>);

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
   serialize_comparison_filter<screener::GreaterThanFilter>,
   parse_comparison_filter<screener::GreaterThanFilter>);
  config_parser.register_filter_parser(
   "LESS_THAN",
   serialize_comparison_filter<screener::LessThanFilter>,
   parse_comparison_filter<screener::LessThanFilter>);
  config_parser.register_filter_parser(
   "GREATER_EQUAL",
   serialize_comparison_filter<screener::GreaterEqualFilter>,
   parse_comparison_filter<screener::GreaterEqualFilter>);
  config_parser.register_filter_parser(
   "LESS_EQUAL",
   serialize_comparison_filter<screener::LessEqualFilter>,
   parse_comparison_filter<screener::LessEqualFilter>);
  config_parser.register_filter_parser(
   "EQUAL",
   serialize_comparison_filter<screener::EqualFilter>,
   parse_comparison_filter<screener::EqualFilter>);
  config_parser.register_filter_parser(
   "NOT_EQUAL",
   serialize_comparison_filter<screener::NotEqualFilter>,
   parse_comparison_filter<screener::NotEqualFilter>);

  config_parser.register_filter_parser(
   "TRUE",
   [](const ConfigParser& config_parser,
      const screener::ScreenerFilter& screener_filter) -> nlohmann::json {
     auto serialized_filter = nlohmann::json{};

     const auto true_filter =
      screener_filter_cast<screener::TrueFilter>(screener_filter);

     if(true_filter) {
       serialized_filter["filter"] = "TRUE";
     }

     return serialized_filter;
   },
   [](ConfigParser::Parser, const nlohmann::json&) -> screener::ScreenerFilter {
     return screener::TrueFilter{};
   });

  config_parser.register_filter_parser(
   "FALSE",
   [](const ConfigParser& config_parser,
      const screener::ScreenerFilter& screener_filter) -> nlohmann::json {
     auto serialized_filter = nlohmann::json{};

     const auto false_filter =
      screener_filter_cast<screener::FalseFilter>(screener_filter);
     if(false_filter) {
       serialized_filter["filter"] = "FALSE";
     }
     return serialized_filter;
   },
   [](ConfigParser::Parser, const nlohmann::json&) -> screener::ScreenerFilter {
     return screener::FalseFilter{};
   });

  config_parser.register_filter_parser(
   "AND",
   [](const ConfigParser& config_parser,
      const screener::ScreenerFilter& screener_filter) -> nlohmann::json {
     return serialize_binary_function_filter<screener::AndFilter>(
      config_parser, screener_filter, "firstCondition", "secondCondition");
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_filter<screener::AndFilter>(
      config_parser, parameters, "firstCondition", "secondCondition");
   });

  config_parser.register_filter_parser(
   "OR",
   [](const ConfigParser& config_parser,
      const screener::ScreenerFilter& screener_filter) -> nlohmann::json {
     return serialize_binary_function_filter<screener::OrFilter>(
      config_parser, screener_filter, "firstCondition", "secondCondition");
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_filter<screener::OrFilter>(
      config_parser, parameters, "firstCondition", "secondCondition");
   });

  config_parser.register_filter_parser(
   "NOT",
   [](const ConfigParser& config_parser,
      const screener::ScreenerFilter& screener_filter) -> nlohmann::json {
     return serialize_unary_function_filter<screener::NotFilter>(
      config_parser, screener_filter, "condition");
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_unary_function_filter<screener::NotFilter>(
      config_parser, parameters, "condition");
   });

  config_parser.register_filter_parser(
   "XOR",
   [](const ConfigParser& config_parser,
      const screener::ScreenerFilter& screener_filter) -> nlohmann::json {
     return serialize_binary_function_filter<screener::XorFilter>(
      config_parser, screener_filter, "firstCondition", "secondCondition");
   },
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_filter<screener::XorFilter>(
      config_parser, parameters, "firstCondition", "secondCondition");
   });

  return config_parser;
}

} // namespace pludux
