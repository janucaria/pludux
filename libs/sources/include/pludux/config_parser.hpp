#ifndef PLUDUX_PLUDUX_CONFIG_PARSER_HPP
#define PLUDUX_PLUDUX_CONFIG_PARSER_HPP

#include <functional>
#include <string>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include <pludux/screener.hpp>

namespace pludux {

class ConfigParser {
public:
  using FilterParser = std::function<screener::ScreenerFilter(
   const ConfigParser&, const nlohmann::json&)>;

  using MethodParser = std::function<screener::ScreenerMethod(
   const ConfigParser&, const nlohmann::json&)>;

  ConfigParser();

  void register_default_parsers();

  void register_filter_parser(const std::string& filter_name,
                              const FilterParser& filter_parser);

  void register_method_parser(const std::string& method_name,
                              const MethodParser& method_parser);

  auto parse_filter(const nlohmann::json& config) const
   -> screener::ScreenerFilter;

  auto parse_method(const nlohmann::json& config) const
   -> screener::ScreenerMethod;

private:
  std::unordered_map<std::string, FilterParser> filter_parsers_;
  std::unordered_map<std::string, MethodParser> method_parsers_;
};

} // namespace pludux

#endif