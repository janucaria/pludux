#ifndef PLUDUX_PLUDUX_CONFIG_PARSER_HPP
#define PLUDUX_PLUDUX_CONFIG_PARSER_HPP

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

#include <nlohmann/json.hpp>

#include <pludux/screener.hpp>

namespace pludux {

class ConfigParser {
public:
  class Parser {
  public:
    Parser(ConfigParser& config_parser);

    auto parse_method(const nlohmann::json& config) -> screener::ScreenerMethod;

    auto parse_filter(const nlohmann::json& config) -> screener::ScreenerFilter;

    auto contains_named_method(const std::string& name) const noexcept -> bool;

  private:
    ConfigParser& config_parser_;
  };

  friend Parser;

  using FilterParser = std::function<screener::ScreenerFilter(
   ConfigParser::Parser, const nlohmann::json&)>;

  using MethodSerialize = std::function<
   auto(const ConfigParser&, const screener::ScreenerMethod&)->nlohmann::json>;

  using MethodDeserialize = std::function<
   auto(ConfigParser::Parser, const nlohmann::json&)->screener::ScreenerMethod>;

  ConfigParser();

  void register_default_parsers();

  void register_filter_parser(const std::string& filter_name,
                              const FilterParser& filter_parser);

  void register_method_parser(const std::string& method_name,
                              const MethodDeserialize& method_deserialize);

  void register_method_parser(const std::string& method_name,
                              const MethodSerialize& method_serialize,
                              const MethodDeserialize& method_deserialize);

  auto parser() -> Parser;

  auto parse_filter(const nlohmann::json& config) -> screener::ScreenerFilter;

  auto parse_method(const nlohmann::json& config) -> screener::ScreenerMethod;

  auto serialize_method(const screener::ScreenerMethod& method) const
   -> nlohmann::json;

  auto parse_named_method(const std::string& name) -> screener::ScreenerMethod;

  auto get_named_methods()
   -> std::unordered_map<std::string, screener::ScreenerMethod>;

private:
  std::unordered_map<std::string, FilterParser> filter_parsers_;
  std::unordered_map<std::string, std::pair<MethodSerialize, MethodDeserialize>>
   method_parsers_;

  std::unordered_map<std::string, nlohmann::json> named_config_methods_;
};

} // namespace pludux

#endif