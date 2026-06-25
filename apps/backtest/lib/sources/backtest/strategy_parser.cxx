module;

#include <cctype>
#include <cstdint>
#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <jsoncons/json.hpp>

export module pludux.backtest:strategy_parser;

import pludux;

import :strategy;
import :plot_method_parser;

export namespace pludux::backtest {

auto parse_backtest_strategy_json(std::string_view strategy_name,
                                  std::istream& json_strategy_stream)
 -> backtest::Strategy
{
  auto config_parser = make_default_registered_config_parser();

  auto strategy_json = jsoncons::ojson::parse(
   json_strategy_stream, jsoncons::json_options{}.allow_comments(true));

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

  auto long_entry_node = ErasedNode{FalseNode{}};
  auto long_exit_node = ErasedNode{FalseNode{}};
  auto position = Strategy::Positions{};

  auto short_entry_node = ErasedNode{FalseNode{}};
  auto short_exit_node = ErasedNode{FalseNode{}};

  if(strategy_json.contains("positions")) {
    const auto positions_json = strategy_json.at("positions");

    auto long_position_side = Strategy::PositionSide{};
    if(positions_json.contains("long")) {
      const auto& long_position_json = positions_json.at("long");

      if(long_position_json.contains("entry")) {
        const auto& entry_json = long_position_json.at("entry");
        long_entry_node = config_parser.parse_node(entry_json.at("signal"));
      }
      if(long_position_json.contains("exit")) {
        const auto& exit_json = long_position_json.at("exit");
        long_exit_node = config_parser.parse_node(exit_json.at("signal"));
      }
      if(long_position_json.contains("pyramiding")) {
        const auto& pyramiding_json = long_position_json.at("pyramiding");
        auto pyramiding = Strategy::Pyramiding{};
        if(pyramiding_json.contains("signal")) {
          pyramiding.signal(
           config_parser.parse_node(pyramiding_json.at("signal")));
        }
        if(pyramiding_json.contains("maxLayers")) {
          pyramiding.max_layers(
           pyramiding_json.at("maxLayers").as<std::size_t>());
        }
        long_position_side.pyramiding(std::move(pyramiding));
      }
    }

    auto short_position_side = Strategy::PositionSide{};
    if(positions_json.contains("short")) {
      const auto& short_position_json = positions_json.at("short");

      if(short_position_json.contains("entry")) {
        const auto& entry_json = short_position_json.at("entry");
        short_entry_node = config_parser.parse_node(entry_json.at("signal"));
      }
      if(short_position_json.contains("exit")) {
        const auto& exit_json = short_position_json.at("exit");
        short_exit_node = config_parser.parse_node(exit_json.at("signal"));
      }
      if(short_position_json.contains("pyramiding")) {
        const auto& pyramiding_json = short_position_json.at("pyramiding");
        auto pyramiding = Strategy::Pyramiding{};
        if(pyramiding_json.contains("signal")) {
          pyramiding.signal(
           config_parser.parse_node(pyramiding_json.at("signal")));
        }
        if(pyramiding_json.contains("maxLayers")) {
          pyramiding.max_layers(
           pyramiding_json.at("maxLayers").as<std::size_t>());
        }
        short_position_side.pyramiding(std::move(pyramiding));
      }
    }

    position.long_side(std::move(long_position_side));
    position.short_side(std::move(short_position_side));
  }

  auto is_take_profit_enabled = false;
  auto take_profit_r_multiple = 1.0;
  if(strategy_json.contains("takeProfit")) {
    const auto take_profit_config = strategy_json.at("takeProfit");
    if(take_profit_config.is_bool()) {
      is_take_profit_enabled = take_profit_config.as_bool();
    } else if(take_profit_config.is_object()) {
      is_take_profit_enabled =
       take_profit_config.get_value_or<bool>("enabled", true);
      take_profit_r_multiple =
       take_profit_config.get_value_or<double>("rMultiple", 1.0);
    } else {
      throw std::runtime_error(
       "Invalid take profit configuration in strategy JSON");
    }
  }

  auto is_trailing_stop_loss = false;
  auto is_stop_loss_enabled = false;
  if(strategy_json.contains("stopLoss")) {
    const auto stop_loss_config = strategy_json.at("stopLoss");
    if(stop_loss_config.is_bool()) {
      is_stop_loss_enabled = stop_loss_config.as_bool();
    } else if(stop_loss_config.is_object()) {
      is_trailing_stop_loss =
       stop_loss_config.get_value_or<bool>("trailing", false);
      is_stop_loss_enabled =
       stop_loss_config.get_value_or<bool>("enabled", true);
    } else {
      throw std::runtime_error(
       "Invalid stop loss configuration in strategy JSON");
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
                  std::move(long_entry_node),
                  std::move(long_exit_node),
                  std::move(short_entry_node),
                  std::move(short_exit_node),
                  std::move(position),
                  is_stop_loss_enabled,
                  is_trailing_stop_loss,
                  is_take_profit_enabled,
                  take_profit_r_multiple,
                  plots};
}

auto parse_backtest_strategy_json(std::string_view strategy_name,
                                  const std::string& json_strategy_str)
 -> backtest::Strategy
{
  auto json_strategy_stream = std::istringstream{json_strategy_str};
  return parse_backtest_strategy_json(strategy_name, json_strategy_stream);
}

auto stringify_backtest_strategy(const backtest::Strategy& strategy)
 -> std::string
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

  auto long_position_json = jsoncons::ojson{};
  long_position_json["entry"] = jsoncons::ojson{};
  long_position_json["entry"]["signal"] =
   config_parser.serialize_node(strategy.long_entry_node());
  long_position_json["exit"] = jsoncons::ojson{};
  long_position_json["exit"]["signal"] =
   config_parser.serialize_node(strategy.long_exit_node());
  {
    long_position_json["pyramiding"] = jsoncons::ojson{};
    long_position_json["pyramiding"]["signal"] = config_parser.serialize_node(
     strategy.positions().long_side().pyramiding().signal());
    long_position_json["pyramiding"]["maxLayers"] =
     strategy.positions().long_side().pyramiding().max_layers();
  }
  positions_json["long"] = std::move(long_position_json);

  auto short_position_json = jsoncons::ojson{};
  short_position_json["entry"] = jsoncons::ojson{};
  short_position_json["entry"]["signal"] =
   config_parser.serialize_node(strategy.short_entry_node());
  short_position_json["exit"] = jsoncons::ojson{};
  short_position_json["exit"]["signal"] =
   config_parser.serialize_node(strategy.short_exit_node());
  {
    short_position_json["pyramiding"] = jsoncons::ojson{};
    short_position_json["pyramiding"]["signal"] = config_parser.serialize_node(
     strategy.positions().short_side().pyramiding().signal());
    short_position_json["pyramiding"]["maxLayers"] =
     strategy.positions().short_side().pyramiding().max_layers();
  }
  positions_json["short"] = std::move(short_position_json);

  strategy_json["positions"] = std::move(positions_json);

  strategy_json["stopLoss"] = jsoncons::ojson{};
  strategy_json["stopLoss"]["enabled"] = strategy.stop_loss_enabled();
  strategy_json["stopLoss"]["trailing"] = strategy.stop_loss_trailing_enabled();

  strategy_json["takeProfit"] = jsoncons::ojson{};
  strategy_json["takeProfit"]["enabled"] = strategy.take_profit_enabled();
  strategy_json["takeProfit"]["rMultiple"] = strategy.take_profit_r_multiple();

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

  return strategy_json.to_string();
}

} // namespace pludux::backtest
