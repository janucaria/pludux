#include <gtest/gtest.h>

#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonschema/jsonschema.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;

TEST(PyramidingRetriggerTest, ParsesSerializesAndRejectsInvalidValues)
{
  EXPECT_EQ(parse_pyramiding_retrigger("EVERY_EVALUATION"),
            PyramidingRetrigger::EveryEvaluation);
  EXPECT_EQ(parse_pyramiding_retrigger("AFTER_FALSE"),
            PyramidingRetrigger::AfterFalse);
  EXPECT_EQ(
   serialize_pyramiding_retrigger(PyramidingRetrigger::EveryEvaluation),
   "EVERY_EVALUATION");
  EXPECT_EQ(serialize_pyramiding_retrigger(PyramidingRetrigger::AfterFalse),
            "AFTER_FALSE");
  EXPECT_THROW(parse_pyramiding_retrigger("LOGIC.ALWAYS"), std::runtime_error);
  EXPECT_EQ(Model::Pyramiding{}.retrigger(),
            PyramidingRetrigger::EveryEvaluation);
  EXPECT_EQ(Model::Pyramiding{}.cooldown(), 0);
}

TEST(PyramidingCooldownTest, IsRequiredWhenPyramidingIsPresent)
{
  auto strategy_json = jsoncons::ojson::parse(stringify_model(Model{}));
  strategy_json.at("positions").at("long").at("pyramiding").erase("cooldown");

  EXPECT_THROW(parse_model("Test", strategy_json.to_string()), std::exception);
}

TEST(ModelInputsTest, CollectsNumericInputsInModelTraversalOrder)
{
  auto series_nodes = OrderedNamedRegistry<ErasedNode<BacktestMethodContext>>{};
  series_nodes.set(
   "spread",
   AddNode<BacktestMethodContext>{
    NumericInputNode{
     "Duplicate", NumericInputNode::ValueRepresentation::Decimal, 1.5},
    NumericInputNode{
     "Duplicate", NumericInputNode::ValueRepresentation::SignedInteger, 2.8}});
  series_nodes.set(
   "channel",
   KcNode<BacktestMethodContext>{
    CloseNode{},
    NumericInputNode{
     "KC Period", NumericInputNode::ValueRepresentation::UnsignedInteger, 20.0},
    NumericInputNode{
     "KC Multiplier", NumericInputNode::ValueRepresentation::Decimal, 1.5},
    NumericInputNode{"KC Band ATR",
                     NumericInputNode::ValueRepresentation::UnsignedInteger,
                     14.0}});

  auto long_pyramiding = Model::Pyramiding{};
  long_pyramiding.signal(NumericInputNode{
   "Long Pyramid", NumericInputNode::ValueRepresentation::Decimal, 3.5});

  auto long_position = Model::Position{};
  long_position.entry(Model::Entry{
   NumericInputNode{
    "Long Entry", NumericInputNode::ValueRepresentation::UnsignedInteger, 4.8},
   SignalTiming::NextOpen});
  long_position.exits({Model::Exit{
   true,
   NumericInputNode{
    "Exit Signal", NumericInputNode::ValueRepresentation::Decimal, 6.0},
   SignalTiming::NextOpen}});
  long_position.pyramiding(std::move(long_pyramiding));
  long_position.risk_distance(
   RiskDistanceAmountNode<BacktestMethodContext>{NumericInputNode{
    "Risk Distance", NumericInputNode::ValueRepresentation::Decimal, 10.0}});
  long_position.stop_losses({Model::StopLoss{
   true,
   NumericInputNode{
    "Stop Price", NumericInputNode::ValueRepresentation::Decimal, 95.0},
   false}});
  long_position.take_profits({Model::TakeProfit{
   true,
   NumericInputNode{
    "Target Price", NumericInputNode::ValueRepresentation::Decimal, 120.0}}});

  const auto strategy = Model{"Test",
                              std::move(series_nodes),
                              std::move(long_position),
                              Model::Position{},
                              {}};

  const auto inputs = collect_model_inputs(strategy);

  ASSERT_EQ(inputs.size(), 11);
  EXPECT_EQ(inputs[0].label(), "Duplicate");
  EXPECT_DOUBLE_EQ(inputs[0].value(), 1.5);
  EXPECT_EQ(inputs[1].label(), "Duplicate");
  EXPECT_DOUBLE_EQ(inputs[1].value(), 2.8);
  EXPECT_EQ(inputs[2].label(), "KC Period");
  EXPECT_DOUBLE_EQ(inputs[2].value(), 20.0);
  EXPECT_EQ(inputs[3].label(), "KC Multiplier");
  EXPECT_DOUBLE_EQ(inputs[3].value(), 1.5);
  EXPECT_EQ(inputs[4].label(), "KC Band ATR");
  EXPECT_DOUBLE_EQ(inputs[4].value(), 14.0);
  EXPECT_EQ(inputs[5].label(), "Long Entry");
  EXPECT_DOUBLE_EQ(inputs[5].value(), 4.8);
  EXPECT_EQ(inputs[6].label(), "Exit Signal");
  EXPECT_DOUBLE_EQ(inputs[6].value(), 6.0);
  EXPECT_EQ(inputs[7].label(), "Long Pyramid");
  EXPECT_DOUBLE_EQ(inputs[7].value(), 3.5);
  EXPECT_EQ(inputs[8].label(), "Risk Distance");
  EXPECT_DOUBLE_EQ(inputs[8].value(), 10.0);
  EXPECT_EQ(inputs[9].label(), "Stop Price");
  EXPECT_DOUBLE_EQ(inputs[9].value(), 95.0);
  EXPECT_EQ(inputs[10].label(), "Target Price");
  EXPECT_DOUBLE_EQ(inputs[10].value(), 120.0);
}

TEST(ModelInputsTest, CollectsInputFromPositionRMultipleSource)
{
  auto series_nodes = OrderedNamedRegistry<ErasedNode<BacktestMethodContext>>{};
  series_nodes.set(
   "current_r",
   PositionRMultipleNode<BacktestMethodContext>{NumericInputNode{
    "R Source", NumericInputNode::ValueRepresentation::Decimal, 125.0}});
  const auto strategy = Model{
   "Test", std::move(series_nodes), Model::Position{}, Model::Position{}, {}};

  const auto inputs = collect_model_inputs(strategy);

  ASSERT_EQ(inputs.size(), 1U);
  EXPECT_EQ(inputs.front().label(), "R Source");
  EXPECT_DOUBLE_EQ(inputs.front().value(), 125.0);
}

TEST(ModelInputsTest, AssigningModelReplacesStrategyInputsWithDefaults)
{
  auto series_nodes = OrderedNamedRegistry<ErasedNode<BacktestMethodContext>>{};
  series_nodes.set(
   "replacement",
   AddNode<BacktestMethodContext>{
    NumericInputNode{"Replacement Decimal",
                     NumericInputNode::ValueRepresentation::Decimal,
                     1.25},
    NumericInputNode{"Replacement Integer",
                     NumericInputNode::ValueRepresentation::SignedInteger,
                     -3.0}});
  const auto model = Model{"Replacement",
                           std::move(series_nodes),
                           Model::Position{},
                           Model::Position{},
                           {}};
  const auto model_handle = ModelStoreHandle{4, 2};

  auto strategy = Strategy{};
  strategy.model_handle(ModelStoreHandle{1, 1});
  strategy.inputs({NumericInputNode{
   "Old Input", NumericInputNode::ValueRepresentation::UnsignedInteger, 99.0}});

  assign_strategy_model(strategy, model_handle, model);

  EXPECT_EQ(strategy.model_handle(), model_handle);
  ASSERT_EQ(strategy.inputs().size(), 2);
  EXPECT_EQ(strategy.inputs()[0].label(), "Replacement Decimal");
  EXPECT_EQ(strategy.inputs()[0].representation(),
            NumericInputNode::ValueRepresentation::Decimal);
  EXPECT_DOUBLE_EQ(strategy.inputs()[0].value(), 1.25);
  EXPECT_EQ(strategy.inputs()[1].label(), "Replacement Integer");
  EXPECT_EQ(strategy.inputs()[1].representation(),
            NumericInputNode::ValueRepresentation::SignedInteger);
  EXPECT_DOUBLE_EQ(strategy.inputs()[1].value(), -3.0);
}

TEST(ModelInputsTest, AssigningModelWithoutInputsClearsStrategyInputs)
{
  auto strategy = Strategy{};
  strategy.inputs({NumericInputNode{
   "Old Input", NumericInputNode::ValueRepresentation::Decimal, 99.0}});
  const auto model_handle = ModelStoreHandle{5, 3};

  assign_strategy_model(strategy, model_handle, Model{});

  EXPECT_EQ(strategy.model_handle(), model_handle);
  EXPECT_TRUE(strategy.inputs().empty());
}

TEST(ModelInputsTest,
     SharedModelCompilesIndependentlyForStrategyInputsAndNamedSeriesResults)
{
  auto series_nodes = OrderedNamedRegistry<ModelNode>{};
  series_nodes.set(
   "configured",
   NumericInputNode{
    "Configured", NumericInputNode::ValueRepresentation::Decimal, 1.0});
  const auto shared_model =
   Model{"Shared", std::move(series_nodes), {}, {}, {}};

  const auto compile_for_input = [&shared_model](double input) {
    const auto inputs = std::vector{input};
    auto conversion_context = NodeToErasedMethodContext{inputs};
    auto methods = ModelMethodRegistry{};
    for(const auto& [name, node] : shared_model.series_nodes()) {
      methods.set(
       name,
       node_to_erased_method<BacktestMethodContext>(node, conversion_context));
    }
    return methods;
  };

  const auto first_methods = compile_for_input(2.0);
  const auto second_methods = compile_for_input(7.0);
  auto first_results = SeriesEvaluationResults{};
  auto second_results = SeriesEvaluationResults{};
  const auto account = BacktestAccountState{1'000.0, 0.0, 1'000.0, 1'000.0};
  const auto first_context =
   BacktestMethodContext{first_methods, first_results, 0, account, 0};
  const auto second_context =
   BacktestMethodContext{second_methods, second_results, 0, account, 0};
  const auto snapshot = AssetSnapshot{AssetHistory{{"Close", {100.0}}}};

  EXPECT_DOUBLE_EQ(first_context.call_series_method("configured", snapshot),
                   2.0);
  EXPECT_DOUBLE_EQ(second_context.call_series_method("configured", snapshot),
                   7.0);

  first_results.alias("configured", first_methods.get("configured").value());
  second_results.alias("configured", second_methods.get("configured").value());
  first_results.put(first_methods.get("configured").value(), 2.0);
  second_results.put(second_methods.get("configured").value(), 7.0);
  EXPECT_DOUBLE_EQ(
   first_results.results(std::string{"configured"})->get().front(), 2.0);
  EXPECT_DOUBLE_EQ(
   second_results.results(std::string{"configured"})->get().front(), 7.0);
}

TEST(ModelParserTest, ParsesPerSideStopLossAndTakeProfit)
{
  const auto strategy_json = std::string{R"({
    "version": 1,
    "execution": { "intrabarPath": "HIGH_FIRST" },
    "positions": {
      "long": {
        "entry": {
          "timing": "CURRENT_CLOSE",
          "signal": true
        },
        "exits": {
          "activation": "AFTER_PREVIOUS",
          "rules": [
            {
              "enabled": true,
              "timing": "CURRENT_CLOSE",
              "signal": false,
              "reduce": 0.25
            },
            {
              "enabled": false,
              "timing": "NEXT_OPEN",
              "signal": true,
              "reduce": 0.5
            }
          ]
        },
        "riskDistance": {
          "method": "RISK_DISTANCE.PERCENT",
          "params": { "percentage": 5 }
        },
        "stopLosses": {
          "activation": "AFTER_PREVIOUS",
          "rules": [
            {
              "enabled": true,
              "trailing": true,
              "stopPrice": "MARKET_DATA.OPEN",
              "reduce": 0.5
            },
            {
              "enabled": false,
              "trailing": false,
              "stopPrice": 80,
              "reduce": 0.25
            }
          ]
        },
        "takeProfits": {
          "activation": "SIMULTANEOUS",
          "rules": [
            {
              "enabled": true,
              "targetPrice": 120,
              "reduce": 0.75
            },
            {
              "enabled": false,
              "targetPrice": 140,
              "reduce": 0.5
            }
          ]
        },
        "pyramiding": {
          "timing": "CURRENT_CLOSE",
          "retrigger": "AFTER_FALSE",
          "cooldown": 3,
          "signal": false,
          "maxLayers": 2,
          "stopTargetReference": {
            "favorable": "INITIAL_ENTRY_PRICE",
            "unfavorable": "LATEST_ENTRY_PRICE"
          }
        }
      },
      "short": false
    }
  })"};

  const auto strategy = parse_model("Test", strategy_json);

  ASSERT_EQ(strategy.long_position().stop_losses().size(), 2);
  EXPECT_TRUE(strategy.long_position().stop_losses()[0].enabled());
  EXPECT_TRUE(node_cast<RiskDistancePercentNode<BacktestMethodContext>>(
   strategy.long_position().risk_distance()));
  EXPECT_EQ(strategy.intrabar_path(), IntrabarPath::HighFirst);
  EXPECT_EQ(strategy.long_position().entry().timing(),
            SignalTiming::CurrentClose);
  ASSERT_EQ(strategy.long_position().exits().size(), 2);
  EXPECT_TRUE(strategy.long_position().exits()[0].enabled());
  EXPECT_EQ(strategy.long_position().exits_activation(),
            ExitActivation::AfterPrevious);
  EXPECT_EQ(strategy.long_position().exits()[0].timing(),
            SignalTiming::CurrentClose);
  EXPECT_DOUBLE_EQ(strategy.long_position().exits()[0].reduce(), 0.25);
  EXPECT_FALSE(strategy.long_position().exits()[1].enabled());
  EXPECT_EQ(strategy.long_position().pyramiding().timing(),
            SignalTiming::CurrentClose);
  EXPECT_EQ(strategy.long_position().pyramiding().retrigger(),
            PyramidingRetrigger::AfterFalse);
  EXPECT_EQ(strategy.long_position().pyramiding().cooldown(), 3);
  EXPECT_EQ(
   strategy.long_position().pyramiding().favorable_stop_target_reference(),
   StopTargetReferencePrice::InitialEntryPrice);
  EXPECT_EQ(
   strategy.long_position().pyramiding().unfavorable_stop_target_reference(),
   StopTargetReferencePrice::LatestEntryPrice);
  EXPECT_TRUE(strategy.long_position().stop_losses()[0].trailing());
  EXPECT_DOUBLE_EQ(strategy.long_position().stop_losses()[0].reduce(), 0.5);
  EXPECT_TRUE(
   node_cast<OpenNode>(strategy.long_position().stop_losses()[0].stop_price()));
  EXPECT_FALSE(strategy.long_position().stop_losses()[1].enabled());
  EXPECT_DOUBLE_EQ(strategy.long_position().stop_losses()[1].reduce(), 0.25);
  ASSERT_EQ(strategy.long_position().take_profits().size(), 2);
  EXPECT_TRUE(strategy.long_position().take_profits()[0].enabled());
  EXPECT_DOUBLE_EQ(strategy.long_position().take_profits()[0].reduce(), 0.75);
  EXPECT_FALSE(strategy.long_position().take_profits()[1].enabled());
  EXPECT_TRUE(node_cast<ValueNode>(
   strategy.long_position().take_profits()[0].target_price()));
  EXPECT_FALSE(strategy.short_position().entry().signal() ==
               strategy.long_position().entry().signal());

  const auto round_tripped =
   parse_model("Round Trip", stringify_model(strategy));
  EXPECT_TRUE(round_tripped.equivalent_rules(strategy));
}

TEST(ModelParserTest, RejectsMissingAndVersionTwoModelJson)
{
  auto model_json = serialize_model_config_json(Model{});
  model_json.erase("version");
  EXPECT_THROW(parse_model_config_json("Missing Version", model_json),
               std::runtime_error);

  model_json = serialize_model_config_json(Model{});
  model_json["version"] = 2;
  EXPECT_THROW(parse_model_config_json("Version Two", model_json),
               std::runtime_error);
}

TEST(ModelParserTest, StringifiesPositionObjectsWithPerSideLevels)
{
  auto long_position = Model::Position{};
  long_position.entry(Model::Entry{TrueNode{}, SignalTiming::CurrentClose});
  long_position.exits(
   {Model::Exit{true, FalseNode{}, SignalTiming::CurrentClose, 0.25}});
  long_position.exits_activation(ExitActivation::AfterPrevious);
  auto pyramiding = Model::Pyramiding{};
  pyramiding.timing(SignalTiming::CurrentClose);
  pyramiding.retrigger(PyramidingRetrigger::AfterFalse);
  pyramiding.cooldown(4);
  pyramiding.favorable_stop_target_reference(
   StopTargetReferencePrice::InitialEntryPrice);
  pyramiding.unfavorable_stop_target_reference(
   StopTargetReferencePrice::LatestEntryPrice);
  long_position.pyramiding(pyramiding);
  long_position.risk_distance(
   RiskDistanceAmountNode<BacktestMethodContext>{10.0});
  long_position.stop_losses({Model::StopLoss{true, OpenNode{}, false, 0.5}});
  long_position.stop_losses_activation(ExitActivation::AfterPrevious);
  long_position.take_profits({Model::TakeProfit{true, ValueNode{120.0}, 0.75}});

  const auto strategy =
   Model{"Test", {}, std::move(long_position), Model::Position{}, {}};

  const auto strategy_json = jsoncons::ojson::parse(stringify_model(strategy));

  EXPECT_TRUE(strategy_json.at("positions").at("long").is_object());
  EXPECT_TRUE(strategy_json.at("positions").at("short").is_object());
  EXPECT_TRUE(strategy_json.at("positions")
               .at("long")
               .at("riskDistance")
               .at("method")
               .as<std::string>() == "RISK_DISTANCE.AMOUNT");
  EXPECT_TRUE(strategy_json.at("positions")
               .at("long")
               .at("stopLosses")
               .at("rules")
               .at(0)
               .contains("stopPrice"));
  EXPECT_TRUE(strategy_json.at("positions")
               .at("long")
               .at("takeProfits")
               .at("rules")
               .at(0)
               .contains("targetPrice"));
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("entry")
             .at("timing")
             .as<std::string>(),
            "CURRENT_CLOSE");
  EXPECT_DOUBLE_EQ(strategy_json.at("positions")
                    .at("long")
                    .at("exits")
                    .at("rules")
                    .at(0)
                    .at("reduce")
                    .as<double>(),
                   0.25);
  EXPECT_DOUBLE_EQ(strategy_json.at("positions")
                    .at("long")
                    .at("stopLosses")
                    .at("rules")
                    .at(0)
                    .at("reduce")
                    .as<double>(),
                   0.5);
  EXPECT_DOUBLE_EQ(strategy_json.at("positions")
                    .at("long")
                    .at("takeProfits")
                    .at("rules")
                    .at(0)
                    .at("reduce")
                    .as<double>(),
                   0.75);
  EXPECT_FALSE(strategy_json.at("positions")
                .at("long")
                .at("exits")
                .at("rules")
                .at(0)
                .contains("reduceRounding"));
  EXPECT_FALSE(strategy_json.at("positions")
                .at("long")
                .at("stopLosses")
                .at("rules")
                .at(0)
                .contains("reduceRounding"));
  EXPECT_FALSE(strategy_json.at("positions")
                .at("long")
                .at("takeProfits")
                .at("rules")
                .at(0)
                .contains("reduceRounding"));
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("exits")
             .at("rules")
             .at(0)
             .at("timing")
             .as<std::string>(),
            "CURRENT_CLOSE");
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("pyramiding")
             .at("timing")
             .as<std::string>(),
            "CURRENT_CLOSE");
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("pyramiding")
             .at("retrigger")
             .as<std::string>(),
            "AFTER_FALSE");
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("pyramiding")
             .at("cooldown")
             .as<std::size_t>(),
            4);
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("pyramiding")
             .at("stopTargetReference")
             .at("favorable")
             .as<std::string>(),
            "INITIAL_ENTRY_PRICE");
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("pyramiding")
             .at("stopTargetReference")
             .at("unfavorable")
             .as<std::string>(),
            "LATEST_ENTRY_PRICE");
  EXPECT_EQ(strategy_json.at("execution").at("intrabarPath").as<std::string>(),
            "CANDLE_DIRECTION");
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("exits")
             .at("activation")
             .as<std::string>(),
            "AFTER_PREVIOUS");
}

TEST(ModelParserTest, JsonconsConvTraitsRoundTripSchemaConfig)
{
  auto long_position = Model::Position{};
  long_position.entry(Model::Entry{TrueNode{}, SignalTiming::CurrentClose});
  long_position.stop_losses({Model::StopLoss{true, OpenNode{}, false}});

  const auto strategy =
   Model{"Config Name", {}, std::move(long_position), Model::Position{}, {}};

  auto strategy_json_str = std::string{};
  jsoncons::encode_json(strategy, strategy_json_str);

  const auto strategy_json = jsoncons::ojson::parse(strategy_json_str);
  EXPECT_EQ(strategy_json.at("version").as<int>(), 1);
  EXPECT_FALSE(strategy_json.contains("name"));

  const auto decoded_strategy = jsoncons::decode_json<Model>(strategy_json_str);

  EXPECT_TRUE(decoded_strategy.name().empty());
  EXPECT_TRUE(decoded_strategy.equivalent_rules(strategy));
}

TEST(ModelParserTest, DefaultStopLossAndEmptyExitCollections)
{
  const auto strategy = Model{};

  ASSERT_EQ(strategy.long_position().stop_losses().size(), 1);
  EXPECT_TRUE(strategy.long_position().stop_losses()[0].enabled());
  const auto* stop_price =
   node_cast<Sl1RNode>(strategy.long_position().stop_losses()[0].stop_price());
  ASSERT_NE(stop_price, nullptr);

  const auto* risk_distance =
   node_cast<RiskDistanceAtrNode<BacktestMethodContext>>(
    strategy.long_position().risk_distance());
  ASSERT_NE(risk_distance, nullptr);
  const auto* risk_multiplier =
   node_cast<ValueNode>(risk_distance->multiplier());
  ASSERT_NE(risk_multiplier, nullptr);
  EXPECT_DOUBLE_EQ(risk_multiplier->value(), 2.0);

  EXPECT_TRUE(strategy.long_position().take_profits().empty());
  EXPECT_TRUE(strategy.long_position().exits().empty());
}

TEST(ModelParserTest, MissingAndEmptyStopLossesDisableExecutableStops)
{
  const auto risk_distance =
   R"("riskDistance":{"method":"RISK_DISTANCE.AMOUNT","params":{"amount":10}})";
  const auto missing = parse_model(
   "Missing",
   std::string{
    R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{)"} +
    risk_distance + R"(},"short":false}})");
  const auto empty = parse_model(
   "Empty",
   std::string{
    R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{)"} +
    risk_distance +
    R"(,"stopLosses":{"activation":"SIMULTANEOUS","rules":[]}},"short":false}})");

  EXPECT_TRUE(missing.long_position().stop_losses().empty());
  EXPECT_TRUE(empty.long_position().stop_losses().empty());
}

TEST(ModelParserTest, RejectsInvalidStopLosses)
{
  const auto risk_distance =
   R"("riskDistance":{"method":"RISK_DISTANCE.AMOUNT","params":{"amount":10}})";
  EXPECT_THROW(
   parse_model(
    "Not Array",
    std::string{
     R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{)"} +
     risk_distance + R"(,"stopLosses":{}},"short":false}})"),
   std::runtime_error);
  EXPECT_THROW(
   parse_model(
    "Bad Item",
    std::string{
     R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{)"} +
     risk_distance +
     R"(,"stopLosses":{"activation":"SIMULTANEOUS","rules":[{}]}},"short":false}})"),
   std::runtime_error);
}

TEST(ModelParserTest, RejectsMissingOrImplicitRiskDistance)
{
  EXPECT_THROW(
   parse_model(
    "Missing",
    R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{},"short":false}})"),
   std::runtime_error);
  EXPECT_THROW(
   parse_model(
    "Numeric",
    R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{"riskDistance":10},"short":false}})"),
   std::runtime_error);
  EXPECT_THROW(
   parse_model(
    "WrongMethod",
    R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{"riskDistance":{"method":"VALUE.CONSTANT","params":{"value":10}}},"short":false}})"),
   std::runtime_error);
}

TEST(ConfigParserTest, RoundTripsExplicitRiskDistanceAndRStopMethods)
{
  auto parser = make_backtest_model_config_parser();
  const auto configurations = std::vector<std::string>{
   R"({"method":"RISK_DISTANCE.AMOUNT","params":{"amount":{"method":"VALUE.CONSTANT","params":{"value":10.0}}}})",
   R"({"method":"RISK_DISTANCE.PERCENT","params":{"percentage":{"method":"VALUE.CONSTANT","params":{"value":5.0}}}})",
   R"({"method":"RISK_DISTANCE.ATR","params":{"period":{"method":"VALUE.CONSTANT","params":{"value":14.0}},"multiplier":{"method":"VALUE.CONSTANT","params":{"value":2.0}},"maSmoothingType":"RMA"}})",
   R"({"method":"STOP_LOSS.ONE_R"})",
   R"({"method":"STOP_LOSS.R_MULTIPLE","params":{"multiple":{"method":"VALUE.CONSTANT","params":{"value":2.0}}}})"};

  for(const auto& configuration : configurations) {
    const auto expected = jsoncons::ojson::parse(configuration);
    const auto node = parser.parse_node(expected);
    EXPECT_EQ(parser.serialize_node(node), expected);
  }
}

TEST(ModelParserTest, LoadsEveryBundledModelSample)
{
  const auto sample_directory =
   std::filesystem::path{PLUDUX_BACKTEST_SAMPLE_DIR};
  auto schema_stream =
   std::ifstream{sample_directory.parent_path() / "schemas" /
                 "pludux.backtest.model.v1-draft.json"};
  auto schema = jsoncons::ojson::parse(schema_stream);
  EXPECT_EQ(schema.at("$id").as<std::string>(),
            "https://pludux.com/schemas/pludux.backtest.model.v1-draft.json");
  EXPECT_EQ(schema.at("title").as<std::string>(),
            "Pludux Backtest Model Schema");
  EXPECT_EQ(schema.at("properties").at("version").at("title").as<std::string>(),
            "Model JSON format version 1");
  EXPECT_EQ(
   schema.at("properties").at("version").at("description").as<std::string>(),
   "Version of the Pludux Model JSON format. Must be 1.");
  EXPECT_EQ(schema.at("properties").at("version").at("const").as<int>(), 1);
  const auto compiled_schema =
   jsoncons::jsonschema::make_json_schema(std::move(schema));
  EXPECT_NO_THROW(
   compiled_schema.validate(jsoncons::ojson::parse(stringify_model(Model{}))));
  auto operator_document = jsoncons::ojson::parse(stringify_model(Model{}));
  operator_document["series"]["scalar_operators"] = jsoncons::ojson::parse(R"({
     "method":"OPERATOR.MAX",
     "params":{
       "left":{"method":"OPERATOR.ABS","params":{"operand":-1}},
       "right":{
         "method":"OPERATOR.MIN",
         "params":{
           "left":{"method":"OPERATOR.POSITIVE_PART","params":{"operand":1}},
           "right":{"method":"OPERATOR.NEGATIVE_PART","params":{"operand":-1}}
         }
       }
     }
   })");
  EXPECT_NO_THROW(compiled_schema.validate(operator_document));
  auto sample_count = std::size_t{0};
  for(const auto& entry :
      std::filesystem::directory_iterator{sample_directory}) {
    if(entry.path().extension() != ".json") {
      continue;
    }
    auto stream = std::ifstream{entry.path()};
    auto json = std::ostringstream{};
    json << stream.rdbuf();
    const auto document = jsoncons::ojson::parse(json.str());
    EXPECT_NO_THROW(compiled_schema.validate(document))
     << entry.path().string();
    EXPECT_NO_THROW(parse_model(entry.path().stem().string(), json.str()))
     << entry.path().string();
    ++sample_count;
  }

  const auto r_multiple_strategy = jsoncons::ojson::parse(R"({
    "version": 1,
    "execution": { "intrabarPath": "CANDLE_DIRECTION" },
    "positions": {
      "long": {
        "entry": { "signal": true, "timing": "CURRENT_CLOSE" },
        "exits": {
          "activation": "SIMULTANEOUS",
          "rules": [{
            "enabled": true,
            "signal": {
              "method": "COMPARISON.GREATER_EQUAL",
              "params": {
                "target": {
                  "method": "POSITION.R_MULTIPLE",
                  "params": { "source": "MARKET_DATA.CLOSE" }
                },
                "threshold": 2
              }
            },
            "timing": "CURRENT_CLOSE",
            "reduce": 1
          }]
        },
        "riskDistance": {
          "method": "RISK_DISTANCE.AMOUNT",
          "params": { "amount": 10 }
        },
        "stopLosses": {
          "activation": "SIMULTANEOUS",
          "rules": [{
            "enabled": true,
            "trailing": false,
            "stopPrice": {
              "method": "STOP_LOSS.R_MULTIPLE",
              "params": { "multiple": 2 }
            },
            "reduce": 1
          }]
        }
      },
      "short": false
    }
  })");
  EXPECT_NO_THROW(compiled_schema.validate(r_multiple_strategy));
  EXPECT_NO_THROW(
   parse_model("Position R-Multiple", r_multiple_strategy.to_string()));

  const auto legacy_signal_fields = jsoncons::ojson::parse(R"({
    "version": 1,
    "execution": { "intrabarPath": "CANDLE_DIRECTION" },
    "positions": {
      "long": {
        "entry": {
          "signal": true,
          "timing": "NEXT_OPEN",
          "signalDelay": 1,
          "price": "MARKET_DATA.OPEN"
        },
        "riskDistance": {
          "method": "RISK_DISTANCE.AMOUNT",
          "params": { "amount": 10 }
        }
      },
      "short": false
    }
  })");
  const auto legacy_exit_array = jsoncons::ojson::parse(R"({
    "version": 1,
    "execution": { "intrabarPath": "CANDLE_DIRECTION" },
    "positions": {
      "long": {
        "entry": { "signal": true, "timing": "CURRENT_CLOSE" },
        "exits": [],
        "riskDistance": {
          "method": "RISK_DISTANCE.AMOUNT",
          "params": { "amount": 10 }
        }
      },
      "short": false
    }
  })");
  EXPECT_ANY_THROW(compiled_schema.validate(legacy_signal_fields));
  EXPECT_ANY_THROW(compiled_schema.validate(legacy_exit_array));
  EXPECT_GT(sample_count, 0);
}
