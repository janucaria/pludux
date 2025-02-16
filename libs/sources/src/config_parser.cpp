#ifndef PLUDUX_FILTER_PARSER_HPP
#define PLUDUX_FILTER_PARSER_HPP

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
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

static auto get_param_or_named_method(ConfigParser::Parser& config_parser,
                                      const nlohmann::json& parameters,
                                      const std::string& key,
                                      const std::string& named_method_key)
 -> std::optional<screener::ScreenerMethod>
{
  const auto is_param_exists = parameters.contains(key);

  if(parameters.contains(key)) {
    return config_parser.parse_method(parameters[key]);
  }

  if(config_parser.contains_named_method(named_method_key)) {
    return config_parser.parse_method(named_method_key);
  }

  return std::nullopt;
}

template<typename T>
static auto parse_ta_with_period_method(ConfigParser::Parser config_parser,
                                        const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto period = parameters["period"].get<int>();

  const auto offset =
   parameters.contains("offset") ? parameters["offset"].get<int>() : 0;

  const auto input_method =
   get_param_or_named_method(config_parser, parameters, "input", "close");

  if(!input_method.has_value()) {
    throw std::invalid_argument{"Target method is not found"};
  }

  const auto ta_method = T{period, input_method.value(), offset};
  return ta_method;
}

static auto parse_value_method(ConfigParser::Parser config_parser,
                               const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto value = parameters["value"].get<double>();
  return screener::ValueMethod{value};
}

static auto parse_data_method(ConfigParser::Parser config_parser,
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

static auto parse_atr_method(ConfigParser::Parser config_parser,
                             const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto period = parameters["period"].get<std::size_t>();
  const auto multiplier = get_param_or(parameters, "multiplier", 1.0);
  const auto offset = get_param_or(parameters, "offset", std::size_t{0});

  auto high_result =
   get_param_or_named_method(config_parser, parameters, "high", "high");
  if(!high_result.has_value()) {
    throw std::invalid_argument{"High method is not found"};
  }

  auto low_result =
   get_param_or_named_method(config_parser, parameters, "low", "low");
  if(!low_result.has_value()) {
    throw std::invalid_argument{"Low method is not found"};
  }

  auto close_result =
   get_param_or_named_method(config_parser, parameters, "close", "close");
  if(!close_result.has_value()) {
    throw std::invalid_argument{"Close method is not found"};
  }

  const auto atr_method = screener::AtrMethod{high_result.value(),
                                              low_result.value(),
                                              close_result.value(),
                                              period,
                                              multiplier,
                                              offset};
  return atr_method;
}

static auto parse_abs_diff_method(ConfigParser::Parser config_parser,
                                  const nlohmann::json& parameters)
 -> screener::ScreenerMethod
{
  const auto operand1 = config_parser.parse_method(parameters["operand1"]);
  const auto operand2 = config_parser.parse_method(parameters["operand2"]);

  const auto abs_diff_method = screener::AbsDiffMethod{operand1, operand2};
  return abs_diff_method;
}

static auto parse_kc_method(ConfigParser::Parser config_parser,
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
static auto parse_binary_function_method(ConfigParser::Parser config_parser,
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
static auto parse_unary_function_method(ConfigParser::Parser config_parser,
                                        const nlohmann::json& parameters,
                                        const std::string& operand_key)
 -> screener::ScreenerMethod
{
  const auto operand = config_parser.parse_method(parameters[operand_key]);
  const auto unary_function_method = T{operand};
  return unary_function_method;
}

template<typename T>
static auto parse_divergence_method(ConfigParser::Parser config_parser,
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
static auto parse_comparison_filter(ConfigParser::Parser config_parser,
                                    const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto target = config_parser.parse_method(parameters["target"]);
  auto threshold = config_parser.parse_method(parameters["threshold"]);
  return T{target, threshold};
}

static auto parse_all_of_filter(ConfigParser::Parser config_parser,
                                const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto filters = std::vector<screener::ScreenerFilter>{};
  for(const auto& subFilter : parameters["filters"]) {
    filters.push_back(config_parser.parse_filter(subFilter));
  }
  return screener::AllOfFilter{filters};
}

static auto parse_any_of_filter(ConfigParser::Parser config_parser,
                                const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto filters = std::vector<screener::ScreenerFilter>{};
  for(const auto& subFilter : parameters["filters"]) {
    filters.push_back(config_parser.parse_filter(subFilter));
  }
  return screener::AnyOfFilter{filters};
}

static auto parse_crossunder_filter(ConfigParser::Parser config_parser,
                                    const nlohmann::json& parameters)
 -> screener::ScreenerFilter
{
  auto signal = config_parser.parse_method(parameters["signal"]);
  auto reference = config_parser.parse_method(parameters["reference"]);
  return screener::CrossunderFilter{signal, reference};
}

static auto parse_crossover_filter(ConfigParser::Parser config_parser,
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
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
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
   "BB",
   [](ConfigParser::Parser config_parser,
      const nlohmann::json& parameters) -> screener::ScreenerMethod {
     auto output = BbOutput::middle;
     const auto param_output =
      get_param_or<std::string>(parameters, "output", "middle");

     if(param_output == "middle") {
       output = BbOutput::middle;
     } else if(param_output == "upper") {
       output = BbOutput::upper;
     } else if(param_output == "lower") {
       output = BbOutput::lower;
     } else {
       const auto error_message =
        std::format("Error BB.output: Unknown output {}", param_output);
       throw std::invalid_argument{error_message};
     }

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

     const auto input =
      get_param_or_named_method(config_parser, parameters, "input", "close");

     if(!input.has_value()) {
       throw std::invalid_argument{"Input method is not found"};
     }

     const auto period = get_param_or(parameters, "period", std::size_t{20});
     const auto stddev = get_param_or(parameters, "stddev", 2.0);

     const auto bb_method =
      screener::BbMethod{output, ma_type, input.value(), period, stddev};

     return bb_method;
   });

  register_method_parser(
   "MACD",
   [](ConfigParser::Parser config_parser,
      const nlohmann::json& parameters) -> screener::ScreenerMethod {
     const auto output_param =
      get_param_or<std::string>(parameters, "output", "macd");

     auto output = MacdOutput::macd;
     if(output_param == "macd") {
       output = MacdOutput::macd;
     } else if(output_param == "signal") {
       output = MacdOutput::signal;
     } else if(output_param == "histogram") {
       output = MacdOutput::histogram;
     } else {
       const auto error_message =
        std::format("Error MACD.output: Unknown output {}", output_param);
       throw std::invalid_argument{error_message};
     }

     const auto fast = get_param_or<std::size_t>(parameters, "fast", 12);
     const auto slow = get_param_or<std::size_t>(parameters, "slow", 26);
     const auto signal = get_param_or<std::size_t>(parameters, "signal", 9);
     const auto offset = get_param_or<std::size_t>(parameters, "offset", 0);
     const auto input =
      get_param_or_named_method(config_parser, parameters, "input", "close");

     if(!input.has_value()) {
       throw std::invalid_argument{"MACD.input method is not found"};
     }

     const auto macd_method =
      screener::MacdMethod{input.value(), output, fast, slow, signal, offset};
     return macd_method;
   });

  register_method_parser(
   "STOCH",
   [](ConfigParser::Parser config_parser,
      const nlohmann::json& parameters) -> screener::ScreenerMethod {
     const auto high =
      get_param_or_named_method(config_parser, parameters, "high", "high");
     if(!high.has_value()) {
       throw std::invalid_argument{"STOCH.high method is not found"};
     }

     const auto low =
      get_param_or_named_method(config_parser, parameters, "low", "low");
     if(!low.has_value()) {
       throw std::invalid_argument{"STOCH.low method is not found"};
     }

     const auto close =
      get_param_or_named_method(config_parser, parameters, "close", "close");
     if(!close.has_value()) {
       throw std::invalid_argument{"STOCH.close method is not found"};
     }

     const auto output_param =
      get_param_or<std::string>(parameters, "output", "k");

     auto output = StochOutput::k;
     if(output_param == "k") {
       output = StochOutput::k;
     } else if(output_param == "d") {
       output = StochOutput::d;
     } else {
       const auto error_message =
        std::format("STOCH.output: Unknown output {}", output_param);
       throw std::invalid_argument{error_message};
     }

     const auto k_period = get_param_or<std::size_t>(parameters, "kPeriod", 5);
     const auto k_smooth = get_param_or<std::size_t>(parameters, "kSmooth", 3);
     const auto d_period = get_param_or<std::size_t>(parameters, "dPeriod", 3);

     const auto stoch_method = screener::StochMethod{high.value(),
                                                     low.value(),
                                                     close.value(),
                                                     output,
                                                     k_period,
                                                     k_smooth,
                                                     d_period};
     return stoch_method;
   });

  register_method_parser(
   "ADD",
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::AddMethod>(
      config_parser, parameters, "augend", "addend");
   });
  register_method_parser(
   "SUBTRACT",
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::SubtractMethod>(
      config_parser, parameters, "minuend", "subtrahend");
   });
  register_method_parser(
   "MULTIPLY",
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::MultiplyMethod>(
      config_parser, parameters, "multiplicand", "multiplier");
   });
  register_method_parser(
   "DIVIDE",
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
     return parse_binary_function_method<screener::DivideMethod>(
      config_parser, parameters, "dividend", "divisor");
   });
  register_method_parser(
   "NEGATE",
   [](ConfigParser::Parser config_parser, const nlohmann::json& parameters) {
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

auto ConfigParser::parser() -> Parser
{
  return Parser{*this};
}

auto ConfigParser::parse_filter(const nlohmann::json& config)
 -> screener::ScreenerFilter
{
  return parser().parse_filter(config);
}

auto ConfigParser::parse_method(const nlohmann::json& config)
 -> screener::ScreenerMethod
{
  return parser().parse_method(config);
}

auto ConfigParser::parse_named_method(const std::string& name)
 -> screener::ScreenerMethod
{
  if(!named_config_methods_.contains(name)) {
    const auto error_message = std::format("Unknown named method: {}", name);
    throw std::invalid_argument{error_message};
  }

  return parser().parse_method(named_config_methods_.at(name));
}

auto ConfigParser::get_named_methods()
 -> std::unordered_map<std::string, screener::ScreenerMethod>
{
  auto named_methods =
   std::unordered_map<std::string, screener::ScreenerMethod>{};
  for(const auto& [name, config] : named_config_methods_) {
    named_methods.emplace(name, parser().parse_method(config));
  }
  return named_methods;
}

ConfigParser::Parser::Parser(ConfigParser& config_parser)
: config_parser_{config_parser}
{
}

auto ConfigParser::Parser::parse_method(const nlohmann::json& config)
 -> screener::ScreenerMethod
{
  if(config.is_number()) {
    return screener::ValueMethod{config.get<double>()};
  }

  auto& named_config_methods = config_parser_.named_config_methods_;

  if(config.is_string()) {
    const auto named_method = config.get<std::string>();
    if(!named_config_methods.contains(named_method)) {
      const auto error_message =
       std::format("Unknown named method: {}", named_method);
      throw std::invalid_argument{error_message};
    }

    const auto named_config_method = named_config_methods.at(named_method);
    return parse_method(named_config_method);
  }

  auto config_method = config;
  const auto method = config_method["method"].get<std::string>();

  if(config_method.contains("extends")) {
    const auto extends = config_method["extends"].get<std::string>();
    if(!named_config_methods.contains(extends)) {
      const auto error_message =
       std::format("Unknown base method: {}", extends);
      throw std::invalid_argument{error_message};
    }
    const auto base_config = named_config_methods.at(extends);
    const auto base_method = base_config["method"].get<std::string>();

    if(base_method != method) {
      const auto error_message = std::format(
       "Base method {} is not the same as method {}", base_method, method);
      throw std::invalid_argument{error_message};
    }

    config_method.erase("extends");
    for(const auto& [key, value] : base_config.items()) {
      if(!config_method.contains(key)) {
        config_method[key] = value;
      }
    }
  }

  auto& method_parsers = config_parser_.method_parsers_;
  if(!method_parsers.contains(method)) {
    const auto error_message = std::format("Unknown method: {}", method);
    throw std::invalid_argument{error_message};
  }

  const auto method_parser = method_parsers.at(method);
  const auto method_result = method_parser(*this, config_method);

  if(config_method.contains("name")) {
    const auto name = config_method["name"].get<std::string>();
    config_method.erase("name");
    named_config_methods.emplace(name, config_method);
  }

  return method_result;
}

auto ConfigParser::Parser::parse_filter(const nlohmann::json& config)
 -> screener::ScreenerFilter
{
  const auto& filter_parsers = config_parser_.filter_parsers_;
  const auto filter = config["filter"].get<std::string>();

  if(filter_parsers.contains(filter)) {
    return filter_parsers.at(filter)(*this, config);
  }

  const auto error_message = std::format("Unknown filter: {}", filter);
  throw std::invalid_argument{error_message};
}

auto ConfigParser::Parser::contains_named_method(
 const std::string& name) const noexcept -> bool
{
  return config_parser_.named_config_methods_.contains(name);
}

} // namespace pludux

#endif