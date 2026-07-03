module;

#include <cctype>
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

import :strategy;
import :plot_method_parser;

export namespace pludux::backtest {

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
    const auto& exit_json = position_json.at("exit");
    position.exit(
     Strategy::Exit{config_parser.parse_node(exit_json.at("signal")),
                    exit_json.at("signalDelay").as<std::size_t>(),
                    config_parser.parse_node(exit_json.at("price"))});
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
    position.pyramiding(std::move(pyramiding));
  }

  if(!position_json.contains("stopLoss")) {
    throw std::runtime_error{
     "Invalid position configuration in strategy JSON: missing stopLoss"};
  }

  const auto& stop_loss_json = position_json.at("stopLoss");
  if(!stop_loss_json.is_object() || !stop_loss_json.contains("stopPrice")) {
    throw std::runtime_error{"Invalid stopLoss configuration in strategy JSON"};
  }

  position.stop_loss(
   Strategy::StopLoss{stop_loss_json.get_value_or<bool>("enabled", false),
                      config_parser.parse_node(stop_loss_json.at("stopPrice")),
                      stop_loss_json.get_value_or<bool>("trailing", false)});

  if(position_json.contains("takeProfit")) {
    const auto& take_profit_json = position_json.at("takeProfit");
    if(!take_profit_json.is_object() ||
       !take_profit_json.contains("targetPrice")) {
      throw std::runtime_error{
       "Invalid takeProfit configuration in strategy JSON"};
    }

    position.take_profit(Strategy::TakeProfit{
     take_profit_json.get_value_or<bool>("enabled", false),
     config_parser.parse_node(take_profit_json.at("targetPrice"))});
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

  position_json["exit"] = jsoncons::ojson{};
  position_json["exit"]["signal"] =
   config_parser.serialize_node(position.exit().signal());
  position_json["exit"]["signalDelay"] = position.exit().signal_delay();
  position_json["exit"]["price"] =
   config_parser.serialize_node(position.exit().price());

  position_json["pyramiding"] = jsoncons::ojson{};
  position_json["pyramiding"]["signal"] =
   config_parser.serialize_node(position.pyramiding().signal());
  position_json["pyramiding"]["signalDelay"] =
   position.pyramiding().signal_delay();
  position_json["pyramiding"]["price"] =
   config_parser.serialize_node(position.pyramiding().price());
  position_json["pyramiding"]["maxLayers"] = position.pyramiding().max_layers();

  position_json["stopLoss"] = jsoncons::ojson{};
  position_json["stopLoss"]["enabled"] = position.stop_loss().enabled();
  position_json["stopLoss"]["trailing"] = position.stop_loss().trailing();
  position_json["stopLoss"]["stopPrice"] =
   config_parser.serialize_node(position.stop_loss().stop_price());

  if(position.take_profit().enabled()) {
    position_json["takeProfit"] = jsoncons::ojson{};
    position_json["takeProfit"]["enabled"] = true;
    position_json["takeProfit"]["targetPrice"] =
     config_parser.serialize_node(position.take_profit().target_price());
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
