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

  using MethodParser = std::function<screener::ScreenerMethod(
   ConfigParser::Parser, const nlohmann::json&)>;

  ConfigParser();

  void register_default_parsers();

  void register_filter_parser(const std::string& filter_name,
                              const FilterParser& filter_parser);

  void register_method_parser(const std::string& method_name,
                              const MethodParser& method_parser);

  auto parser() -> Parser;

  auto parse_filter(const nlohmann::json& config) -> screener::ScreenerFilter;

  auto parse_method(const nlohmann::json& config) -> screener::ScreenerMethod;

  auto parse_named_method(const std::string& name) -> screener::ScreenerMethod;

  auto get_named_methods()
   -> std::unordered_map<std::string, screener::ScreenerMethod>;

private:
  std::unordered_map<std::string, FilterParser> filter_parsers_;
  std::unordered_map<std::string, MethodParser> method_parsers_;

  std::unordered_map<std::string, nlohmann::json> named_config_methods_;
};

} // namespace pludux

#endif