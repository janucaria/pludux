module;

#include <cctype>
#include <cmath>
#include <cstdint>
#include <istream>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include <jsoncons/json.hpp>
#include <jsoncons/reflect/json_conv_traits.hpp>

export module pludux.backtest:strategy_parser;

import pludux;

import :risk_distance_node;
import :strategy;
import :plot_method_parser;
import :config_parser;

export namespace pludux::backtest {

auto parse_stop_target_reference_price(std::string_view value)
 -> StopTargetReferencePrice
{
  if(value == "LATEST_ENTRY_PRICE") {
    return StopTargetReferencePrice::LatestEntryPrice;
  }
  if(value == "AVERAGE_PRICE") {
    return StopTargetReferencePrice::AveragePrice;
  }
  if(value == "INITIAL_ENTRY_PRICE") {
    return StopTargetReferencePrice::InitialEntryPrice;
  }
  throw std::runtime_error{"Invalid pyramiding stop/target reference price"};
}

auto serialize_stop_target_reference_price(StopTargetReferencePrice reference)
 -> std::string
{
  switch(reference) {
  case StopTargetReferencePrice::LatestEntryPrice:
    return "LATEST_ENTRY_PRICE";
  case StopTargetReferencePrice::AveragePrice:
    return "AVERAGE_PRICE";
  case StopTargetReferencePrice::InitialEntryPrice:
    return "INITIAL_ENTRY_PRICE";
  }
  return "AVERAGE_PRICE";
}

auto parse_reduce(const jsoncons::ojson& config) -> double
{
  const auto reduce = config.get_value_or<double>("reduce", 1.0);
  if(!std::isfinite(reduce) || reduce <= 0.0 || reduce > 1.0) {
    throw std::runtime_error{"Invalid exit reduce value: expected (0, 1]"};
  }
  return reduce;
}

auto parse_strategy_position(const jsoncons::ojson& position_json,
                             auto& config_parser) -> Strategy::Position
{
  auto position = Strategy::Position{};

  if(position_json.is_bool()) {
    if(!position_json.as_bool()) {
      return position;
    }

    throw std::runtime_error{
     "Invalid position configuration in strategy JSON: expected false or an "
     "object"};
  }

  if(!position_json.is_object()) {
    throw std::runtime_error{
     "Invalid position configuration in strategy JSON: expected false or an "
     "object"};
  }

  if(position_json.contains("entry")) {
    const auto& entry_json = position_json.at("entry");
    position.entry(
     Strategy::Entry{config_parser.parse_node(entry_json.at("signal")),
                     entry_json.at("signalDelay").as<std::size_t>(),
                     config_parser.parse_node(entry_json.at("price"))});
  }

  if(position_json.contains("exit")) {
    throw std::runtime_error{
     "Invalid position configuration in strategy JSON: 'exit' was replaced "
     "by the 'exits' array"};
  }

  if(position_json.contains("exits")) {
    const auto& exits_json = position_json.at("exits");
    if(!exits_json.is_array()) {
      throw std::runtime_error{
       "Invalid exits configuration in strategy JSON: expected an array"};
    }

    auto exits = std::vector<Strategy::Exit>{};
    exits.reserve(exits_json.size());
    for(const auto& exit_json : exits_json.array_range()) {
      if(!exit_json.is_object() || !exit_json.contains("signal") ||
         !exit_json.contains("signalDelay") || !exit_json.contains("price")) {
        throw std::runtime_error{"Invalid exits item in strategy JSON"};
      }
      exits.emplace_back(exit_json.get_value_or<bool>("enabled", false),
                         config_parser.parse_node(exit_json.at("signal")),
                         exit_json.at("signalDelay").as<std::size_t>(),
                         config_parser.parse_node(exit_json.at("price")),
                         parse_reduce(exit_json));
    }
    position.exits(std::move(exits));
  }

  if(position_json.contains("pyramiding")) {
    const auto& pyramiding_json = position_json.at("pyramiding");
    auto pyramiding = Strategy::Pyramiding{};
    if(pyramiding_json.contains("signal")) {
      pyramiding.signal(config_parser.parse_node(pyramiding_json.at("signal")));
    }
    pyramiding.signal_delay(
     pyramiding_json.at("signalDelay").as<std::size_t>());
    pyramiding.price(config_parser.parse_node(pyramiding_json.at("price")));
    if(pyramiding_json.contains("maxLayers")) {
      pyramiding.max_layers(pyramiding_json.at("maxLayers").as<std::size_t>());
    }
    if(pyramiding_json.contains("stopTargetReference")) {
      const auto& stop_target_reference_json =
       pyramiding_json.at("stopTargetReference");
      pyramiding.favorable_stop_target_reference(
       parse_stop_target_reference_price(
        stop_target_reference_json.get_value_or<std::string>("favorable",
                                                             "AVERAGE_PRICE")));
      pyramiding.unfavorable_stop_target_reference(
       parse_stop_target_reference_price(
        stop_target_reference_json.get_value_or<std::string>("unfavorable",
                                                             "AVERAGE_PRICE")));
    }
    position.pyramiding(std::move(pyramiding));
  }

  if(!position_json.contains("riskDistance")) {
    throw std::runtime_error{
     "Invalid position configuration in strategy JSON: missing "
     "riskDistance"};
  }
  const auto& risk_distance_json = position_json.at("riskDistance");
  if(!risk_distance_json.is_object()) {
    throw std::runtime_error{
     "Invalid riskDistance configuration in strategy JSON: expected an "
     "explicit method object"};
  }
  auto risk_distance = config_parser.parse_node(risk_distance_json);
  if(!node_cast<RiskDistanceAmountNode>(risk_distance) &&
     !node_cast<RiskDistancePercentNode>(risk_distance) &&
     !node_cast<RiskDistanceAtrNode>(risk_distance)) {
    throw std::runtime_error{
     "Invalid riskDistance configuration in strategy JSON: expected an "
     "R_DISTANCE_* method"};
  }
  position.risk_distance(std::move(risk_distance));

  auto stop_losses = std::vector<Strategy::StopLoss>{};
  if(position_json.contains("stopLosses")) {
    const auto& stop_losses_json = position_json.at("stopLosses");
    if(!stop_losses_json.is_array()) {
      throw std::runtime_error{
       "Invalid stopLosses configuration in strategy JSON: expected an "
       "array"};
    }
    stop_losses.reserve(stop_losses_json.size());
    for(const auto& stop_loss_json : stop_losses_json.array_range()) {
      if(!stop_loss_json.is_object() || !stop_loss_json.contains("stopPrice")) {
        throw std::runtime_error{"Invalid stopLosses item in strategy JSON"};
      }
      stop_losses.emplace_back(
       stop_loss_json.get_value_or<bool>("enabled", false),
       config_parser.parse_node(stop_loss_json.at("stopPrice")),
       stop_loss_json.get_value_or<bool>("trailing", false),
       parse_reduce(stop_loss_json));
    }
  }
  position.stop_losses(std::move(stop_losses));

  if(position_json.contains("takeProfit")) {
    throw std::runtime_error{
     "Invalid position configuration in strategy JSON: 'takeProfit' was "
     "replaced by the 'takeProfits' array"};
  }

  if(position_json.contains("takeProfits")) {
    const auto& take_profits_json = position_json.at("takeProfits");
    if(!take_profits_json.is_array()) {
      throw std::runtime_error{
       "Invalid takeProfits configuration in strategy JSON: expected an "
       "array"};
    }

    auto take_profits = std::vector<Strategy::TakeProfit>{};
    take_profits.reserve(take_profits_json.size());
    for(const auto& take_profit_json : take_profits_json.array_range()) {
      if(!take_profit_json.is_object() ||
         !take_profit_json.contains("targetPrice")) {
        throw std::runtime_error{"Invalid takeProfits item in strategy JSON"};
      }
      take_profits.emplace_back(
       take_profit_json.get_value_or<bool>("enabled", false),
       config_parser.parse_node(take_profit_json.at("targetPrice")),
       parse_reduce(take_profit_json));
    }
    position.take_profits(std::move(take_profits));
  }

  return position;
}

auto serialize_strategy_position(const Strategy::Position& position,
                                 auto& config_parser) -> jsoncons::ojson
{
  auto position_json = jsoncons::ojson{};

  position_json["entry"] = jsoncons::ojson{};
  position_json["entry"]["signal"] =
   config_parser.serialize_node(position.entry().signal());
  position_json["entry"]["signalDelay"] = position.entry().signal_delay();
  position_json["entry"]["price"] =
   config_parser.serialize_node(position.entry().price());

  position_json["exits"] = jsoncons::ojson::array();
  for(const auto& exit : position.exits()) {
    auto exit_json = jsoncons::ojson{};
    exit_json["enabled"] = exit.enabled();
    exit_json["signal"] = config_parser.serialize_node(exit.signal());
    exit_json["signalDelay"] = exit.signal_delay();
    exit_json["price"] = config_parser.serialize_node(exit.price());
    exit_json["reduce"] = exit.reduce();
    position_json["exits"].push_back(std::move(exit_json));
  }

  position_json["pyramiding"] = jsoncons::ojson{};
  position_json["pyramiding"]["signal"] =
   config_parser.serialize_node(position.pyramiding().signal());
  position_json["pyramiding"]["signalDelay"] =
   position.pyramiding().signal_delay();
  position_json["pyramiding"]["price"] =
   config_parser.serialize_node(position.pyramiding().price());
  position_json["pyramiding"]["maxLayers"] = position.pyramiding().max_layers();
  position_json["pyramiding"]["stopTargetReference"] = jsoncons::ojson{};
  position_json["pyramiding"]["stopTargetReference"]["favorable"] =
   serialize_stop_target_reference_price(
    position.pyramiding().favorable_stop_target_reference());
  position_json["pyramiding"]["stopTargetReference"]["unfavorable"] =
   serialize_stop_target_reference_price(
    position.pyramiding().unfavorable_stop_target_reference());

  position_json["riskDistance"] =
   config_parser.serialize_node(position.risk_distance());

  position_json["stopLosses"] = jsoncons::ojson::array();
  for(const auto& stop_loss : position.stop_losses()) {
    auto stop_loss_json = jsoncons::ojson{};
    stop_loss_json["enabled"] = stop_loss.enabled();
    stop_loss_json["trailing"] = stop_loss.trailing();
    stop_loss_json["stopPrice"] =
     config_parser.serialize_node(stop_loss.stop_price());
    stop_loss_json["reduce"] = stop_loss.reduce();
    position_json["stopLosses"].push_back(std::move(stop_loss_json));
  }

  position_json["takeProfits"] = jsoncons::ojson::array();
  for(const auto& take_profit : position.take_profits()) {
    auto take_profit_json = jsoncons::ojson{};
    take_profit_json["enabled"] = take_profit.enabled();
    take_profit_json["targetPrice"] =
     config_parser.serialize_node(take_profit.target_price());
    take_profit_json["reduce"] = take_profit.reduce();
    position_json["takeProfits"].push_back(std::move(take_profit_json));
  }

  return position_json;
}

auto parse_backtest_strategy_config_json(std::string_view strategy_name,
                                         const jsoncons::ojson& strategy_json)
 -> backtest::Strategy
{
  auto config_parser = make_default_registered_config_parser();

  if(!strategy_json.is_object()) {
    throw std::runtime_error(
     "Invalid strategy JSON: expected an object at the root");
  }

  if(strategy_json.contains("version")) {
    const auto version = strategy_json.at("version").as<int>();
    if(version != 2) {
      throw std::runtime_error("Unsupported strategy JSON version: " +
                               std::to_string(version));
    }
  }

  auto series_nodes = OrderedNamedRegistry<ErasedNode>{};
  if(strategy_json.contains("series")) {
    const auto& series_json = strategy_json.at("series");
    for(const auto& [series_name, series_config] : series_json.object_range()) {
      series_nodes.set(series_name, config_parser.parse_node(series_config));
    }
  }

  auto long_position = Strategy::Position{};
  auto short_position = Strategy::Position{};

  if(strategy_json.contains("positions")) {
    const auto positions_json = strategy_json.at("positions");

    if(positions_json.contains("long")) {
      long_position =
       parse_strategy_position(positions_json.at("long"), config_parser);
    }

    if(positions_json.contains("short")) {
      short_position =
       parse_strategy_position(positions_json.at("short"), config_parser);
    }
  }

  auto plots = std::vector<PlotGroup>{};
  if(strategy_json.contains("plots")) {
    const auto& plots_json = strategy_json.at("plots");
    auto plot_method_parser = make_default_registered_plot_method_parser();

    for(const auto& plot_group_json : plots_json.array_range()) {
      const auto label =
       plot_group_json.get_value_or<std::string>("label", "Unnamed");
      const auto overlay = plot_group_json.get_value_or<bool>("overlay", true);
      auto plot_items = std::vector<AnyPlotMethod>{};

      if(plot_group_json.contains("items")) {
        const auto& items_json = plot_group_json.at("items");
        for(const auto& item_json : items_json.array_range()) {
          auto plot_method = plot_method_parser.deserialize_method(item_json);
          plot_items.push_back(std::move(plot_method));
        }
      }

      plots.emplace_back(PlotGroup{label, overlay, std::move(plot_items)});
    }
  }

  return Strategy{std::string{strategy_name},
                  std::move(series_nodes),
                  std::move(long_position),
                  std::move(short_position),
                  plots};
}

auto parse_backtest_strategy_json(std::string_view strategy_name,
                                  std::istream& json_strategy_stream)
 -> backtest::Strategy
{
  auto strategy_json = jsoncons::ojson::parse(
   json_strategy_stream, jsoncons::json_options{}.allow_comments(true));

  return parse_backtest_strategy_config_json(strategy_name, strategy_json);
}

auto parse_backtest_strategy_json(std::string_view strategy_name,
                                  const std::string& json_strategy_str)
 -> backtest::Strategy
{
  auto json_strategy_stream = std::istringstream{json_strategy_str};
  return parse_backtest_strategy_json(strategy_name, json_strategy_stream);
}

auto serialize_backtest_strategy_config_json(const backtest::Strategy& strategy)
 -> jsoncons::ojson
{
  auto config_parser = make_default_registered_config_parser();

  auto strategy_json = jsoncons::ojson{};

  strategy_json["version"] = 2;

  auto series_json = jsoncons::ojson{};
  for(const auto& [series_name, series_node] : strategy.series_nodes()) {
    series_json[series_name] = config_parser.serialize_node(series_node);
  }
  strategy_json["series"] = std::move(series_json);

  auto positions_json = jsoncons::ojson{};

  positions_json["long"] =
   serialize_strategy_position(strategy.long_position(), config_parser);
  positions_json["short"] =
   serialize_strategy_position(strategy.short_position(), config_parser);

  strategy_json["positions"] = std::move(positions_json);

  auto plot_method_parser = make_default_registered_plot_method_parser();
  auto plots_json = jsoncons::ojson::array();
  for(const auto& plot_group : strategy.plots()) {
    auto plot_group_json = jsoncons::ojson{};
    plot_group_json["label"] = plot_group.name();
    plot_group_json["overlay"] = plot_group.is_overlay();

    auto items_json = jsoncons::ojson::array();
    for(const auto& item : plot_group.items()) {
      items_json.push_back(plot_method_parser.serialize_method(item));
    }
    plot_group_json["items"] = std::move(items_json);

    plots_json.push_back(std::move(plot_group_json));
  }
  strategy_json["plots"] = std::move(plots_json);

  return strategy_json;
}

auto stringify_backtest_strategy(const backtest::Strategy& strategy)
 -> std::string
{
  const auto strategy_json = serialize_backtest_strategy_config_json(strategy);
  return strategy_json.to_string();
}

} // namespace pludux::backtest

export namespace jsoncons::reflect {

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::Strategy> {
  using value_type = pludux::backtest::Strategy;
  using result_type = jsoncons::conversion_result<value_type>;

  static constexpr bool is_compatible = true;

  static constexpr bool is(const Json& json) noexcept
  {
    return json.is_object();
  }

  template<typename Alloc, typename TempAlloc>
  static result_type try_as(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                            const Json& json)
  {
    try {
      const auto strategy_json = jsoncons::ojson::parse(json.to_string());
      return result_type{pludux::backtest::parse_backtest_strategy_config_json(
       "", strategy_json)};
    } catch(...) {
      return result_type{jsoncons::unexpect,
                         jsoncons::conv_errc::conversion_failed};
    }
  }

  template<typename Alloc, typename TempAlloc>
  static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                      const value_type& strategy)
  {
    const auto strategy_json =
     pludux::backtest::serialize_backtest_strategy_config_json(strategy);
    return Json::parse(strategy_json.to_string());
  }
};

} // namespace jsoncons::reflect
