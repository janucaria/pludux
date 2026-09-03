module;

#include <cctype>
#include <cmath>
#include <cstdint>
#include <istream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <jsoncons/json.hpp>
#include <jsoncons/reflect/json_conv_traits.hpp>

export module pludux.backtest:model_parser;

import pludux;

import :risk_distance_node;
import :execution_model;
import :model;
import :plot_method_parser;
import :config_parser;

namespace {

template<typename T>
auto strict_as(const jsoncons::ojson& json) -> T
{
  if constexpr(std::is_same_v<T, bool>) {
    if(!json.is_bool()) {
      throw std::invalid_argument{"Expected a JSON boolean"};
    }
    return json.as_bool();
  } else if constexpr(std::is_same_v<T, std::string>) {
    if(!json.is_string()) {
      throw std::invalid_argument{"Expected a JSON string"};
    }
    return json.as_string();
  } else if constexpr(std::is_floating_point_v<T>) {
    if(!json.is_number()) {
      throw std::invalid_argument{"Expected a JSON number"};
    }
    const auto value = json.as<T>();
    if(!std::isfinite(value)) {
      throw std::invalid_argument{"Expected a finite JSON number"};
    }
    return value;
  } else if constexpr(std::is_integral_v<T>) {
    if(!json.is_number()) {
      throw std::invalid_argument{"Expected a JSON number"};
    }
    if(json.is_uint64()) {
      const auto value = json.as<std::uint64_t>();
      if(value > static_cast<std::uint64_t>(std::numeric_limits<T>::max())) {
        throw std::out_of_range{"JSON integer is out of range"};
      }
      return static_cast<T>(value);
    }
    if(json.is_int64()) {
      const auto value = json.as<std::int64_t>();
      if constexpr(std::is_unsigned_v<T>) {
        if(value < 0 || static_cast<std::uint64_t>(value) >
                         std::numeric_limits<T>::max()) {
          throw std::out_of_range{"JSON integer is out of range"};
        }
      } else if(value < static_cast<std::int64_t>(std::numeric_limits<T>::min()) ||
                value > static_cast<std::int64_t>(std::numeric_limits<T>::max())) {
        throw std::out_of_range{"JSON integer is out of range"};
      }
      return static_cast<T>(value);
    }
    const auto value = json.as_double();
    if(!std::isfinite(value) || std::trunc(value) != value ||
       std::abs(value) > 9'007'199'254'740'991.0) {
      throw std::invalid_argument{"Expected an in-range integral JSON number"};
    }
    if constexpr(std::is_unsigned_v<T>) {
      if(value < 0.0 || static_cast<long double>(value) >
                         static_cast<long double>(
                          std::numeric_limits<T>::max())) {
        throw std::out_of_range{"JSON integer is out of range"};
      }
    } else if(static_cast<long double>(value) <
               static_cast<long double>(std::numeric_limits<T>::min()) ||
              static_cast<long double>(value) >
               static_cast<long double>(std::numeric_limits<T>::max())) {
      throw std::out_of_range{"JSON integer is out of range"};
    }
    return static_cast<T>(value);
  } else {
    return json.as<T>();
  }
}

template<typename T>
auto value_or(const jsoncons::ojson& json,
              std::string_view key,
              const T& default_value) -> T
{
  return json.contains(key) ? strict_as<T>(json.at(key)) : default_value;
}

} // namespace

export namespace pludux::backtest {

auto parse_intrabar_path(std::string_view value) -> IntrabarPath
{
  if(value == "LOW_FIRST") {
    return IntrabarPath::LowFirst;
  }
  if(value == "HIGH_FIRST") {
    return IntrabarPath::HighFirst;
  }
  if(value == "CANDLE_DIRECTION") {
    return IntrabarPath::CandleDirection;
  }
  throw std::runtime_error{"Invalid intrabar path"};
}

auto serialize_intrabar_path(IntrabarPath path) -> std::string
{
  switch(path) {
  case IntrabarPath::LowFirst:
    return "LOW_FIRST";
  case IntrabarPath::HighFirst:
    return "HIGH_FIRST";
  case IntrabarPath::CandleDirection:
    return "CANDLE_DIRECTION";
  }
  return "CANDLE_DIRECTION";
}

auto parse_exit_activation(std::string_view value) -> ExitActivation
{
  if(value == "SIMULTANEOUS") {
    return ExitActivation::Simultaneous;
  }
  if(value == "AFTER_PREVIOUS") {
    return ExitActivation::AfterPrevious;
  }
  throw std::runtime_error{"Invalid exit activation"};
}

auto serialize_exit_activation(ExitActivation activation) -> std::string
{
  return activation == ExitActivation::AfterPrevious ? "AFTER_PREVIOUS"
                                                     : "SIMULTANEOUS";
}

auto parse_signal_timing(std::string_view value) -> SignalTiming
{
  if(value == "CURRENT_CLOSE") {
    return SignalTiming::CurrentClose;
  }
  if(value == "NEXT_OPEN") {
    return SignalTiming::NextOpen;
  }
  throw std::runtime_error{"Invalid signal timing"};
}

auto serialize_signal_timing(SignalTiming timing) -> std::string
{
  return timing == SignalTiming::CurrentClose ? "CURRENT_CLOSE" : "NEXT_OPEN";
}

auto parse_pyramiding_retrigger(std::string_view value) -> PyramidingRetrigger
{
  if(value == "EVERY_EVALUATION") {
    return PyramidingRetrigger::EveryEvaluation;
  }
  if(value == "AFTER_FALSE") {
    return PyramidingRetrigger::AfterFalse;
  }
  throw std::runtime_error{"Invalid pyramiding retrigger"};
}

auto serialize_pyramiding_retrigger(PyramidingRetrigger retrigger)
 -> std::string
{
  return retrigger == PyramidingRetrigger::AfterFalse ? "AFTER_FALSE"
                                                      : "EVERY_EVALUATION";
}

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

auto parse_reduce(const jsoncons::ojson& config, double default_value) -> double
{
  const auto reduce = value_or<double>(config, "reduce", default_value);
  if(!std::isfinite(reduce) || reduce <= 0.0 || reduce > 1.0) {
    throw std::runtime_error{"Invalid exit reduce value: expected (0, 1]"};
  }
  return reduce;
}

auto parse_model_position(const jsoncons::ojson& position_json,
                          auto& config_parser) -> Model::Position
{
  auto position = Model::Position{};

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
    if(!entry_json.is_object()) {
      throw std::runtime_error{"Invalid entry configuration in model JSON"};
    }
    const auto defaults = Model::Entry{};
    position.entry(Model::Entry{
     entry_json.contains("signal")
      ? config_parser.parse_node(entry_json.at("signal"))
      : defaults.signal(),
     entry_json.contains("timing")
      ? parse_signal_timing(strict_as<std::string>(entry_json.at("timing")))
      : defaults.timing()});
  }

  if(position_json.contains("exits")) {
    const auto& exits_json = position_json.at("exits");
    if(!exits_json.is_object()) {
      throw std::runtime_error{"Invalid exits configuration in strategy JSON"};
    }

    if(exits_json.contains("activation")) {
      position.exits_activation(parse_exit_activation(
       strict_as<std::string>(exits_json.at("activation"))));
    }
    auto exits = std::vector<Model::Exit>{};
    const auto exit_rules_json = exits_json.contains("rules")
                                  ? exits_json.at("rules")
                                  : jsoncons::ojson::array();
    if(!exit_rules_json.is_array()) {
      throw std::runtime_error{"Invalid exits rules in strategy JSON"};
    }
    exits.reserve(exit_rules_json.size());
    for(const auto& exit_json : exit_rules_json.array_range()) {
      if(!exit_json.is_object()) {
        throw std::runtime_error{"Invalid exits item in strategy JSON"};
      }
      const auto defaults = Model::Exit{};
      exits.emplace_back(
       value_or<bool>(exit_json, "enabled", defaults.enabled()),
       exit_json.contains("signal")
        ? config_parser.parse_node(exit_json.at("signal"))
        : defaults.signal(),
       exit_json.contains("timing")
        ? parse_signal_timing(strict_as<std::string>(exit_json.at("timing")))
        : defaults.timing(),
       parse_reduce(exit_json, defaults.reduce()));
    }
    position.exits(std::move(exits));
  }

  if(position_json.contains("pyramiding")) {
    const auto& pyramiding_json = position_json.at("pyramiding");
    if(!pyramiding_json.is_object()) {
      throw std::runtime_error{"Invalid pyramiding configuration in model JSON"};
    }
    auto pyramiding = Model::Pyramiding{};
    if(pyramiding_json.contains("signal")) {
      pyramiding.signal(config_parser.parse_node(pyramiding_json.at("signal")));
    }
    if(pyramiding_json.contains("timing")) {
      pyramiding.timing(parse_signal_timing(
       strict_as<std::string>(pyramiding_json.at("timing"))));
    }
    if(pyramiding_json.contains("retrigger")) {
      pyramiding.retrigger(parse_pyramiding_retrigger(
       strict_as<std::string>(pyramiding_json.at("retrigger"))));
    }
    if(pyramiding_json.contains("cooldown")) {
      pyramiding.cooldown(
       strict_as<std::size_t>(pyramiding_json.at("cooldown")));
    }
    if(pyramiding_json.contains("maxLayers")) {
      pyramiding.max_layers(
       strict_as<std::size_t>(pyramiding_json.at("maxLayers")));
    }
    if(pyramiding_json.contains("stopTargetReference")) {
      const auto& stop_target_reference_json =
       pyramiding_json.at("stopTargetReference");
      if(!stop_target_reference_json.is_object()) {
        throw std::runtime_error{"Invalid stop/target reference configuration"};
      }
      if(stop_target_reference_json.contains("favorable")) {
        pyramiding.favorable_stop_target_reference(
         parse_stop_target_reference_price(strict_as<std::string>(
          stop_target_reference_json.at("favorable"))));
      }
      if(stop_target_reference_json.contains("unfavorable")) {
        pyramiding.unfavorable_stop_target_reference(
         parse_stop_target_reference_price(strict_as<std::string>(
          stop_target_reference_json.at("unfavorable"))));
      }
    }
    position.pyramiding(std::move(pyramiding));
  }

  if(position_json.contains("riskDistance")) {
    const auto& risk_distance_json = position_json.at("riskDistance");
    if(!risk_distance_json.is_object()) {
      throw std::runtime_error{
       "Invalid riskDistance configuration in strategy JSON: expected an "
       "explicit method object"};
    }
    auto risk_distance = config_parser.parse_node(risk_distance_json);
    if(!node_cast<RiskDistanceAmountNode<BacktestMethodContext>>(
         risk_distance) &&
       !node_cast<RiskDistancePercentNode<BacktestMethodContext>>(
        risk_distance) &&
       !node_cast<RiskDistanceAtrNode<BacktestMethodContext>>(risk_distance)) {
      throw std::runtime_error{
       "Invalid riskDistance configuration in strategy JSON: expected an "
       "R_DISTANCE_* method"};
    }
    position.risk_distance(std::move(risk_distance));
  }

  if(position_json.contains("stopLosses")) {
    const auto& stop_losses_json = position_json.at("stopLosses");
    if(!stop_losses_json.is_object()) {
      throw std::runtime_error{
       "Invalid stopLosses configuration in strategy JSON"};
    }
    if(stop_losses_json.contains("activation")) {
      position.stop_losses_activation(parse_exit_activation(
       strict_as<std::string>(stop_losses_json.at("activation"))));
    }
    if(stop_losses_json.contains("rules")) {
      const auto& stop_loss_rules_json = stop_losses_json.at("rules");
      if(!stop_loss_rules_json.is_array()) {
        throw std::runtime_error{"Invalid stopLosses rules in strategy JSON"};
      }
      auto stop_losses = std::vector<Model::StopLoss>{};
      stop_losses.reserve(stop_loss_rules_json.size());
      for(const auto& stop_loss_json : stop_loss_rules_json.array_range()) {
        if(!stop_loss_json.is_object()) {
          throw std::runtime_error{"Invalid stopLosses item in strategy JSON"};
        }
        const auto defaults = Model::StopLoss{};
        stop_losses.emplace_back(
         value_or<bool>(stop_loss_json, "enabled", defaults.enabled()),
         stop_loss_json.contains("stopPrice")
          ? config_parser.parse_node(stop_loss_json.at("stopPrice"))
          : defaults.stop_price(),
         value_or<bool>(stop_loss_json, "trailing", defaults.trailing()),
         parse_reduce(stop_loss_json, defaults.reduce()));
      }
      position.stop_losses(std::move(stop_losses));
    }
  }

  if(position_json.contains("takeProfits")) {
    const auto& take_profits_json = position_json.at("takeProfits");
    if(!take_profits_json.is_object()) {
      throw std::runtime_error{
       "Invalid takeProfits configuration in strategy JSON"};
    }

    if(take_profits_json.contains("activation")) {
      position.take_profits_activation(parse_exit_activation(
       strict_as<std::string>(take_profits_json.at("activation"))));
    }
    auto take_profits = std::vector<Model::TakeProfit>{};
    const auto take_profit_rules_json = take_profits_json.contains("rules")
                                        ? take_profits_json.at("rules")
                                        : jsoncons::ojson::array();
    if(!take_profit_rules_json.is_array()) {
      throw std::runtime_error{"Invalid takeProfits rules in strategy JSON"};
    }
    take_profits.reserve(take_profit_rules_json.size());
    for(const auto& take_profit_json : take_profit_rules_json.array_range()) {
      if(!take_profit_json.is_object()) {
        throw std::runtime_error{"Invalid takeProfits item in strategy JSON"};
      }
      const auto defaults = Model::TakeProfit{};
      take_profits.emplace_back(
       value_or<bool>(take_profit_json, "enabled", defaults.enabled()),
       take_profit_json.contains("targetPrice")
        ? config_parser.parse_node(take_profit_json.at("targetPrice"))
        : defaults.target_price(),
       parse_reduce(take_profit_json, defaults.reduce()));
    }
    position.take_profits(std::move(take_profits));
  }

  return position;
}

auto serialize_model_position(const Model::Position& position,
                              auto& config_parser) -> jsoncons::ojson
{
  auto position_json = jsoncons::ojson{};

  position_json["entry"] = jsoncons::ojson{};
  position_json["entry"]["signal"] =
   config_parser.serialize_node(position.entry().signal());
  position_json["entry"]["timing"] =
   serialize_signal_timing(position.entry().timing());

  position_json["exits"] = jsoncons::ojson{};
  position_json["exits"]["activation"] =
   serialize_exit_activation(position.exits_activation());
  position_json["exits"]["rules"] = jsoncons::ojson::array();
  for(const auto& exit : position.exits()) {
    auto exit_json = jsoncons::ojson{};
    exit_json["enabled"] = exit.enabled();
    exit_json["signal"] = config_parser.serialize_node(exit.signal());
    exit_json["timing"] = serialize_signal_timing(exit.timing());
    exit_json["reduce"] = exit.reduce();
    position_json["exits"]["rules"].push_back(std::move(exit_json));
  }

  position_json["pyramiding"] = jsoncons::ojson{};
  position_json["pyramiding"]["signal"] =
   config_parser.serialize_node(position.pyramiding().signal());
  position_json["pyramiding"]["timing"] =
   serialize_signal_timing(position.pyramiding().timing());
  position_json["pyramiding"]["retrigger"] =
   serialize_pyramiding_retrigger(position.pyramiding().retrigger());
  position_json["pyramiding"]["cooldown"] = position.pyramiding().cooldown();
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

  position_json["stopLosses"] = jsoncons::ojson{};
  position_json["stopLosses"]["activation"] =
   serialize_exit_activation(position.stop_losses_activation());
  position_json["stopLosses"]["rules"] = jsoncons::ojson::array();
  for(const auto& stop_loss : position.stop_losses()) {
    auto stop_loss_json = jsoncons::ojson{};
    stop_loss_json["enabled"] = stop_loss.enabled();
    stop_loss_json["trailing"] = stop_loss.trailing();
    stop_loss_json["stopPrice"] =
     config_parser.serialize_node(stop_loss.stop_price());
    stop_loss_json["reduce"] = stop_loss.reduce();
    position_json["stopLosses"]["rules"].push_back(std::move(stop_loss_json));
  }

  position_json["takeProfits"] = jsoncons::ojson{};
  position_json["takeProfits"]["activation"] =
   serialize_exit_activation(position.take_profits_activation());
  position_json["takeProfits"]["rules"] = jsoncons::ojson::array();
  for(const auto& take_profit : position.take_profits()) {
    auto take_profit_json = jsoncons::ojson{};
    take_profit_json["enabled"] = take_profit.enabled();
    take_profit_json["targetPrice"] =
     config_parser.serialize_node(take_profit.target_price());
    take_profit_json["reduce"] = take_profit.reduce();
    position_json["takeProfits"]["rules"].push_back(
     std::move(take_profit_json));
  }

  return position_json;
}

auto parse_model_config_json(std::string_view model_name,
                             const jsoncons::ojson& model_json)
 -> backtest::Model
{
  auto config_parser = make_backtest_model_config_parser();

  if(!model_json.is_object()) {
    throw std::runtime_error(
     "Invalid strategy JSON: expected an object at the root");
  }

  if(!model_json.contains("version")) {
    throw std::runtime_error{"Invalid model JSON: missing version"};
  }
  const auto version = strict_as<int>(model_json.at("version"));
  if(version != 1) {
    throw std::runtime_error("Unsupported model JSON version: " +
                             std::to_string(version));
  }

  auto model = Model{};
  model.name(std::string{model_name});

  if(model_json.contains("execution")) {
    const auto& execution_json = model_json.at("execution");
    if(!execution_json.is_object()) {
      throw std::runtime_error{"Invalid model JSON: execution must be an object"};
    }
    if(execution_json.contains("intrabarPath")) {
      model.intrabar_path(parse_intrabar_path(
       strict_as<std::string>(execution_json.at("intrabarPath"))));
    }
  }

  auto series_nodes = OrderedNamedRegistry<ModelNode>{};
  if(model_json.contains("series")) {
    const auto& series_json = model_json.at("series");
    if(!series_json.is_object()) {
      throw std::runtime_error{"Invalid model JSON: series must be an object"};
    }
    for(const auto& [series_name, series_config] : series_json.object_range()) {
      series_nodes.set(series_name, config_parser.parse_node(series_config));
    }
  }

  auto long_position = Model::Position{};
  auto short_position = Model::Position{};

  if(model_json.contains("positions")) {
    const auto& positions_json = model_json.at("positions");
    if(!positions_json.is_object()) {
      throw std::runtime_error{"Invalid model JSON: positions must be an object"};
    }

    if(positions_json.contains("long")) {
      long_position =
        parse_model_position(positions_json.at("long"), config_parser);
    }

    if(positions_json.contains("short")) {
      short_position =
        parse_model_position(positions_json.at("short"), config_parser);
    }
  }

  auto plots = std::vector<PlotGroup>{};
  if(model_json.contains("plots")) {
    const auto& plots_json = model_json.at("plots");
    if(!plots_json.is_array()) {
      throw std::runtime_error{"Invalid model JSON: plots must be an array"};
    }
    auto plot_method_parser = make_default_registered_plot_method_parser();

    for(const auto& plot_group_json : plots_json.array_range()) {
      if(!plot_group_json.is_object()) {
        throw std::runtime_error{"Invalid model JSON: plot group must be an object"};
      }
      const auto defaults = PlotGroup{};
      const auto label =
       value_or<std::string>(plot_group_json, "label", defaults.name());
      const auto overlay =
       value_or<bool>(plot_group_json, "overlay", defaults.is_overlay());
      auto plot_items =
       std::vector<ErasedPlotMethod<ErasedPlotMethodContext>>{};

      if(plot_group_json.contains("items")) {
        const auto& items_json = plot_group_json.at("items");
        if(!items_json.is_array()) {
          throw std::runtime_error{"Invalid model JSON: plot items must be an array"};
        }
        for(const auto& item_json : items_json.array_range()) {
          auto plot_method = plot_method_parser.deserialize_method(item_json);
          plot_items.push_back(std::move(plot_method));
        }
      }

      plots.emplace_back(PlotGroup{label, overlay, std::move(plot_items)});
    }
  }

  model.series_nodes(std::move(series_nodes));
  model.long_position(std::move(long_position));
  model.short_position(std::move(short_position));
  model.plots(std::move(plots));
  return model;
}

auto parse_model(std::string_view model_name, std::istream& json_model_stream)
 -> backtest::Model
{
  auto model_json = jsoncons::ojson::parse(
    json_model_stream, jsoncons::json_options{}.allow_comments(true));

  return parse_model_config_json(model_name, model_json);
}

auto parse_model(std::string_view model_name, const std::string& json_model)
 -> backtest::Model
{
  auto json_model_stream = std::istringstream{json_model};
  return parse_model(model_name, json_model_stream);
}

auto serialize_model_config_json(const backtest::Model& model)
 -> jsoncons::ojson
{
  auto config_parser = make_backtest_model_config_parser();

  auto model_json = jsoncons::ojson{};

  model_json["version"] = 1;
  model_json["execution"] = jsoncons::ojson{};
  model_json["execution"]["intrabarPath"] =
   serialize_intrabar_path(model.intrabar_path());

  auto series_json = jsoncons::ojson{};
  for(const auto& [series_name, series_node] : model.series_nodes()) {
    series_json[series_name] = config_parser.serialize_node(series_node);
  }
  model_json["series"] = std::move(series_json);

  auto positions_json = jsoncons::ojson{};

  positions_json["long"] =
    serialize_model_position(model.long_position(), config_parser);
  positions_json["short"] =
    serialize_model_position(model.short_position(), config_parser);

  model_json["positions"] = std::move(positions_json);

  auto plot_method_parser = make_default_registered_plot_method_parser();
  auto plots_json = jsoncons::ojson::array();
  for(const auto& plot_group : model.plots()) {
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
  model_json["plots"] = std::move(plots_json);

  return model_json;
}

auto stringify_model(const backtest::Model& model)
 -> std::string
{
  const auto model_json = serialize_model_config_json(model);
  return model_json.to_string();
}

} // namespace pludux::backtest

export namespace jsoncons::reflect {

template<typename Json>
struct json_conv_traits<Json, pludux::backtest::Model> {
  using value_type = pludux::backtest::Model;
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
       const auto model_json = jsoncons::ojson::parse(json.to_string());
       return result_type{
        pludux::backtest::parse_model_config_json("", model_json)};
    } catch(...) {
      return result_type{jsoncons::unexpect,
                         jsoncons::conv_errc::conversion_failed};
    }
  }

  template<typename Alloc, typename TempAlloc>
   static Json to_json(const jsoncons::allocator_set<Alloc, TempAlloc>&,
                       const value_type& model)
  {
     const auto model_json =
      pludux::backtest::serialize_model_config_json(model);
     return Json::parse(model_json.to_string());
  }
};

} // namespace jsoncons::reflect
