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

    auto method_registry() const noexcept
     -> std::shared_ptr<const screener::MethodRegistry>;

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

  ConfigParser();

  ConfigParser(std::shared_ptr<screener::MethodRegistry> method_registry);

  void register_default_parsers();

  void register_filter_parser(const std::string& filter_name,
                              const FilterDeserialize& filter_deserialize);

  void register_filter_parser(const std::string& filter_name,
                              const FilterSerialize& filter_serialize,
                              const FilterDeserialize& filter_deserialize);

  void register_method_parser(const std::string& method_name,
                              const MethodDeserialize& method_deserialize);

  void register_method_parser(const std::string& method_name,
                              const MethodSerialize& method_serialize,
                              const MethodDeserialize& method_deserialize);

  auto parser() -> Parser;

  auto parse_filter(const nlohmann::json& config) -> screener::ScreenerFilter;

  auto serialize_filter(const screener::ScreenerFilter& filter) const
   -> nlohmann::json;

  auto parse_method(const nlohmann::json& config) -> screener::ScreenerMethod;

  auto serialize_method(const screener::ScreenerMethod& method) const
   -> nlohmann::json;

private:
  std::unordered_map<std::string, std::pair<FilterSerialize, FilterDeserialize>>
   filter_parsers_;
  std::unordered_map<std::string, std::pair<MethodSerialize, MethodDeserialize>>
   method_parsers_;

  std::shared_ptr<screener::MethodRegistry> method_registry_;
};

} // namespace pludux

#endif