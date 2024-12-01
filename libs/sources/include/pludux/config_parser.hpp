#ifndef PLUDUX_PLUDUX_CONFIG_PARSER_HPP
#define PLUDUX_PLUDUX_CONFIG_PARSER_HPP

#include <nlohmann/json.hpp>

#include <pludux/screener.hpp>

namespace pludux {

auto parse_screener_method(const nlohmann::json& config) -> screener::ScreenerMethod;

auto parse_screener_filter(const nlohmann::json& config) -> screener::ScreenerFilter;

}

#endif