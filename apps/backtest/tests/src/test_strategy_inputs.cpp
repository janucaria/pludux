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
  EXPECT_EQ(Strategy::Pyramiding{}.retrigger(),
            PyramidingRetrigger::EveryEvaluation);
  EXPECT_EQ(Strategy::Pyramiding{}.cooldown(), 0);
}

TEST(PyramidingCooldownTest, DefaultsWhenOmitted)
{
  auto strategy_json = jsoncons::ojson::parse(stringify_strategy(Strategy{}));
  strategy_json.at("positions").at("long").at("pyramiding").erase("cooldown");

  const auto parsed = parse_strategy("Test", strategy_json.to_string());
  EXPECT_EQ(parsed.long_position().pyramiding().cooldown(),
            Strategy::Pyramiding{}.cooldown());
}

TEST(StrategyInputsTest, CollectsNumericInputsInStrategyTraversalOrder)
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

  auto long_pyramiding = Strategy::Pyramiding{};
  long_pyramiding.signal(NumericInputNode{
   "Long Pyramid", NumericInputNode::ValueRepresentation::Decimal, 3.5});

  auto long_position = Strategy::Position{};
  long_position.entry(Strategy::Entry{
   NumericInputNode{
    "Long Entry", NumericInputNode::ValueRepresentation::UnsignedInteger, 4.8},
   OpenNode{},
   1,
   PriceExitsStart::CurrentBar});
  long_position.exits({Strategy::Exit{
   true,
   NumericInputNode{
    "Exit Signal", NumericInputNode::ValueRepresentation::Decimal, 6.0},
   OpenNode{},
   1}});
  long_position.pyramiding(std::move(long_pyramiding));
  long_position.risk_distance(
   RiskDistanceAmountNode<BacktestMethodContext>{NumericInputNode{
    "Risk Distance", NumericInputNode::ValueRepresentation::Decimal, 10.0}});
  long_position.stop_losses({Strategy::StopLoss{
   true,
   NumericInputNode{
    "Stop Price", NumericInputNode::ValueRepresentation::Decimal, 95.0},
   false}});
  long_position.take_profits({Strategy::TakeProfit{
   true,
   NumericInputNode{
    "Target Price", NumericInputNode::ValueRepresentation::Decimal, 120.0}}});

  const auto strategy = Strategy{"Test",
                              std::move(series_nodes),
                              std::move(long_position),
                              Strategy::Position{},
                              {}};

  const auto inputs = collect_strategy_inputs(strategy);

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

TEST(StrategyInputsTest, CollectsInputFromPositionRMultipleSource)
{
  auto series_nodes = OrderedNamedRegistry<ErasedNode<BacktestMethodContext>>{};
  series_nodes.set(
   "current_r",
   PositionRMultipleNode<BacktestMethodContext>{NumericInputNode{
    "R Source", NumericInputNode::ValueRepresentation::Decimal, 125.0}});
  const auto strategy = Strategy{
   "Test", std::move(series_nodes), Strategy::Position{}, Strategy::Position{}, {}};

  const auto inputs = collect_strategy_inputs(strategy);

  ASSERT_EQ(inputs.size(), 1U);
  EXPECT_EQ(inputs.front().label(), "R Source");
  EXPECT_DOUBLE_EQ(inputs.front().value(), 125.0);
}

TEST(StrategyInputsTest, CollectsOffsetComparisonOperandsInEvaluationOrder)
{
  auto document = jsoncons::ojson::parse(R"({
    "version": 1,
    "series": {
      "absolute": {
        "method": "COMPARISON.ABOVE_BY",
        "params": {
          "target": {"method": "INPUT.NUMERIC", "params": {"label": "Target", "value": 115}},
          "reference": {"method": "INPUT.NUMERIC", "params": {"label": "Reference", "value": 100}},
          "gap": {"method": "INPUT.NUMERIC", "params": {"label": "Gap", "value": 15}}
        }
      },
      "relative": {
        "method": "COMPARISON.ABOVE_BY_PERCENT",
        "params": {
          "target": 115,
          "reference": 100,
          "percent": {"method": "INPUT.NUMERIC", "params": {"label": "Percent", "value": 15}}
        }
      }
    }
  })");

  const auto methods = StrategyMethodRegistry{};
  auto results = SeriesEvaluationResults{};
  const auto account = BacktestAccountState{1'000.0, 0.0, 1'000.0, 1'000.0};
  const auto context = BacktestMethodContext{methods, results, 0, account, 0};
  const auto history = AssetHistory{{"Close", {100.0}}};
  const auto snapshot = AssetSnapshot{history};

  for(const auto direction :
      {ComparisonDirection::Above, ComparisonDirection::Below}) {
    const auto below = direction == ComparisonDirection::Below;
    if(below) {
      auto& series = document.at("series");
      series.at("absolute")["method"] = "COMPARISON.BELOW_BY";
      series.at("relative")["method"] = "COMPARISON.BELOW_BY_PERCENT";
      series.at("relative").at("params")["target"] = 85;
    }
    const auto strategy = parse_strategy("Offset inputs", document.to_string());
    const auto inputs = collect_strategy_inputs(strategy);
    ASSERT_EQ(inputs.size(), 4U);
    EXPECT_EQ(inputs[0].label(), "Target");
    EXPECT_EQ(inputs[1].label(), "Reference");
    EXPECT_EQ(inputs[2].label(), "Gap");
    EXPECT_EQ(inputs[3].label(), "Percent");

    const auto values = std::vector{below ? 166.0 : 234.0, 200.0, 34.0, 16.0};
    auto conversion = NodeToErasedMethodContext{values};
    const auto absolute = node_to_erased_method<BacktestMethodContext>(
     strategy.series_nodes().get("absolute").value(), conversion);
    const auto relative = node_to_erased_method<BacktestMethodContext>(
     strategy.series_nodes().get("relative").value(), conversion);

    EXPECT_TRUE(evaluate_signal_method(absolute, snapshot, context));
    EXPECT_FALSE(evaluate_signal_method(relative, snapshot, context));
  }
}

TEST(StrategyParserTest, EvaluatesAboveByAtAndAroundTheSignedThreshold)
{
  auto parser = make_backtest_strategy_config_parser();
  const auto configurations = {
   R"({"method":"COMPARISON.ABOVE_BY","params":{"target":"MARKET_DATA.CLOSE","reference":10,"gap":0.1}})",
   R"({"method":"COMPARISON.ABOVE_BY_PERCENT","params":{"target":"MARKET_DATA.CLOSE","reference":10,"percent":1}})"};
  const auto history = AssetHistory{{"Close", {9.9, 10.09, 10.1, 10.11}}};
  const auto snapshot = AssetSnapshot{history};
  const auto methods = StrategyMethodRegistry{};
  auto results = SeriesEvaluationResults{};
  const auto account = BacktestAccountState{1'000.0, 0.0, 1'000.0, 1'000.0};
  const auto context = BacktestMethodContext{methods, results, 0, account, 0};

  for(const auto* configuration : configurations) {
    SCOPED_TRACE(configuration);
    auto conversion = NodeToErasedMethodContext{};
    const auto method = node_to_erased_method<BacktestMethodContext>(
     parser.parse_node(jsoncons::ojson::parse(configuration)), conversion);
    EXPECT_FALSE(evaluate_signal_method(method, snapshot[3], context));
    EXPECT_FALSE(evaluate_signal_method(method, snapshot[2], context));
    EXPECT_TRUE(evaluate_signal_method(method, snapshot[1], context));
    EXPECT_TRUE(evaluate_signal_method(method, snapshot[0], context));
  }

  auto conversion = NodeToErasedMethodContext{};
  const auto tiny_gap = node_to_erased_method<BacktestMethodContext>(
   parser.parse_node(jsoncons::ojson::parse(
    R"({"method":"COMPARISON.ABOVE_BY","params":{"target":10,"reference":10,"gap":1e-20}})")),
   conversion);
  EXPECT_FALSE(evaluate_signal_method(tiny_gap, snapshot, context));
}

TEST(StrategyParserTest, EvaluatesBelowByAtAndAroundTheSignedThreshold)
{
  auto parser = make_backtest_strategy_config_parser();
  const auto history = AssetHistory{{"Close", {9.89, 9.9, 9.91, 10.1}}};
  const auto snapshot = AssetSnapshot{history};
  const auto methods = StrategyMethodRegistry{};
  auto results = SeriesEvaluationResults{};
  const auto account = BacktestAccountState{1'000.0, 0.0, 1'000.0, 1'000.0};
  const auto context = BacktestMethodContext{methods, results, 0, account, 0};
  const auto compile = [&](const char* config) {
    auto conversion = NodeToErasedMethodContext{};
    return node_to_erased_method<BacktestMethodContext>(
     parser.parse_node(jsoncons::ojson::parse(config)), conversion);
  };

  for(
   const auto* config :
   {R"({"method":"COMPARISON.BELOW_BY","params":{"target":"MARKET_DATA.CLOSE","reference":10,"gap":0.1}})",
    R"({"method":"COMPARISON.BELOW_BY_PERCENT","params":{"target":"MARKET_DATA.CLOSE","reference":10,"percent":1}})"}) {
    SCOPED_TRACE(config);
    const auto method = compile(config);
    EXPECT_TRUE(evaluate_signal_method(method, snapshot[3], context));
    EXPECT_TRUE(evaluate_signal_method(method, snapshot[2], context));
    EXPECT_FALSE(evaluate_signal_method(method, snapshot[1], context));
    EXPECT_FALSE(evaluate_signal_method(method, snapshot[0], context));
  }

  const auto tiny_gap = compile(
   R"({"method":"COMPARISON.BELOW_BY","params":{"target":10,"reference":10,"gap":1e-20}})");
  EXPECT_FALSE(evaluate_signal_method(tiny_gap, snapshot, context));
  const auto signed_reference = compile(
   R"({"method":"COMPARISON.BELOW_BY_PERCENT","params":{"target":-9,"reference":-10,"percent":10}})");
  EXPECT_TRUE(evaluate_signal_method(signed_reference, snapshot, context));
}

TEST(StrategyParserTest, EvaluatesOffsetComparisonsWithIndicatorsAfterWarmup)
{
  auto parser = make_backtest_strategy_config_parser();
  const auto configurations = {
   R"({
     "method": "COMPARISON.ABOVE_BY",
     "params": {
       "target": 115,
       "reference": 100,
       "gap": {
         "method": "OPERATOR.MULTIPLY",
         "params": {
           "multiplicand": {"method": "INDICATOR.ATR", "params": {"period": 2}},
           "multiplier": 1.5
         }
       }
     }
   })",
   R"({
     "method": "COMPARISON.ABOVE_BY_PERCENT",
     "params": {
       "target": 115,
       "reference": {"method": "INDICATOR.SMA", "params": {"period": 2}},
       "percent": 15
     }
   })"};
  const auto history = AssetHistory{
   {"Close", {100.0, 100.0}}, {"High", {105.0, 105.0}}, {"Low", {95.0, 95.0}}};
  const auto snapshot = AssetSnapshot{history};
  const auto methods = StrategyMethodRegistry{};
  auto results = SeriesEvaluationResults{};
  const auto account = BacktestAccountState{1'000.0, 0.0, 1'000.0, 1'000.0};
  const auto context = BacktestMethodContext{methods, results, 0, account, 0};

  for(const auto* configuration : configurations) {
    SCOPED_TRACE(configuration);
    auto document = jsoncons::ojson::parse(configuration);
    for(const auto direction :
        {ComparisonDirection::Above, ComparisonDirection::Below}) {
      if(direction == ComparisonDirection::Below) {
        auto& params = document.at("params");
        const auto has_gap = params.contains("gap");
        params["target"] = 85;
        document["method"] =
         has_gap ? "COMPARISON.BELOW_BY" : "COMPARISON.BELOW_BY_PERCENT";
      }
      auto conversion = NodeToErasedMethodContext{};
      const auto method = node_to_erased_method<BacktestMethodContext>(
       parser.parse_node(document), conversion);
      EXPECT_FALSE(evaluate_signal_method(method, snapshot[1], context));
      EXPECT_TRUE(evaluate_signal_method(method, snapshot[0], context));
    }
  }
}

TEST(StrategyInputsTest, AssigningStrategyReplacesSetupInputsWithDefaults)
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
  const auto strategy = Strategy{"Replacement",
                           std::move(series_nodes),
                           Strategy::Position{},
                           Strategy::Position{},
                           {}};
  const auto strategy_handle = StrategyStoreHandle{4, 2};

  auto setup = StrategySetup{};
  setup.strategy_handle(StrategyStoreHandle{1, 1});
  setup.inputs({NumericInputNode{
   "Old Input", NumericInputNode::ValueRepresentation::UnsignedInteger, 99.0}});

  assign_setup_strategy(setup, strategy_handle, strategy);

  EXPECT_EQ(setup.strategy_handle(), strategy_handle);
  ASSERT_EQ(setup.inputs().size(), 2);
  EXPECT_EQ(setup.inputs()[0].label(), "Replacement Decimal");
  EXPECT_EQ(setup.inputs()[0].representation(),
            NumericInputNode::ValueRepresentation::Decimal);
  EXPECT_DOUBLE_EQ(setup.inputs()[0].value(), 1.25);
  EXPECT_EQ(setup.inputs()[1].label(), "Replacement Integer");
  EXPECT_EQ(setup.inputs()[1].representation(),
            NumericInputNode::ValueRepresentation::SignedInteger);
  EXPECT_DOUBLE_EQ(setup.inputs()[1].value(), -3.0);
}

TEST(StrategyInputsTest, AssigningStrategyWithoutInputsClearsSetupInputs)
{
  auto setup = StrategySetup{};
  setup.inputs({NumericInputNode{
   "Old Input", NumericInputNode::ValueRepresentation::Decimal, 99.0}});
  const auto strategy_handle = StrategyStoreHandle{5, 3};

  assign_setup_strategy(setup, strategy_handle, Strategy{});

  EXPECT_EQ(setup.strategy_handle(), strategy_handle);
  EXPECT_TRUE(setup.inputs().empty());
}

TEST(StrategyInputsTest,
     SharedStrategyCompilesIndependentlyForStrategyInputsAndNamedSeriesResults)
{
  auto series_nodes = OrderedNamedRegistry<StrategyNode>{};
  series_nodes.set(
   "configured",
   NumericInputNode{
    "Configured", NumericInputNode::ValueRepresentation::Decimal, 1.0});
  const auto shared_strategy =
   Strategy{"Shared", std::move(series_nodes), {}, {}, {}};

  const auto compile_for_input = [&shared_strategy](double input) {
    const auto inputs = std::vector{input};
    auto conversion_context = NodeToErasedMethodContext{inputs};
    auto methods = StrategyMethodRegistry{};
    for(const auto& [name, node] : shared_strategy.series_nodes()) {
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

TEST(StrategyParserTest, ParsesPerSideStopLossAndTakeProfit)
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

  const auto strategy = parse_strategy("Test", strategy_json);

  ASSERT_EQ(strategy.long_position().stop_losses().size(), 2);
  EXPECT_TRUE(strategy.long_position().stop_losses()[0].enabled());
  EXPECT_TRUE(node_cast<RiskDistancePercentNode<BacktestMethodContext>>(
   strategy.long_position().risk_distance()));
  const auto& entry = strategy.long_position().entry();
  EXPECT_NE(node_cast<CloseNode>(entry.price()), nullptr);
  EXPECT_EQ(entry.delay(), 0U);
  EXPECT_EQ(entry.price_exits_start(), PriceExitsStart::NextBar);
  ASSERT_EQ(strategy.long_position().exits().size(), 2);
  EXPECT_TRUE(strategy.long_position().exits()[0].enabled());
  EXPECT_EQ(strategy.long_position().exits_activation(),
            ExitActivation::AfterPrevious);
  EXPECT_NE(node_cast<CloseNode>(strategy.long_position().exits()[0].price()),
            nullptr);
  EXPECT_EQ(strategy.long_position().exits()[0].delay(), 0U);
  EXPECT_DOUBLE_EQ(strategy.long_position().exits()[0].reduce(), 0.25);
  EXPECT_FALSE(strategy.long_position().exits()[1].enabled());
  EXPECT_NE(node_cast<CloseNode>(strategy.long_position().pyramiding().price()),
            nullptr);
  EXPECT_EQ(strategy.long_position().pyramiding().delay(), 0U);
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
   parse_strategy("Round Trip", stringify_strategy(strategy));
  EXPECT_TRUE(round_tripped.equivalent_rules(strategy));
}

TEST(StrategyParserTest, RejectsMissingAndVersionTwoStrategyJson)
{
  auto strategy_json = serialize_strategy_config_json(Strategy{});
  strategy_json.erase("version");
  EXPECT_THROW(parse_strategy_config_json("Missing Version", strategy_json),
               std::runtime_error);

  strategy_json = serialize_strategy_config_json(Strategy{});
  strategy_json["version"] = 2;
  EXPECT_THROW(parse_strategy_config_json("Version Two", strategy_json),
               std::runtime_error);
}

TEST(StrategyParserTest, StringifiesPositionObjectsWithPerSideLevels)
{
  auto long_position = Strategy::Position{};
  long_position.entry(
   Strategy::Entry{TrueNode{}, CloseNode{}, 0, PriceExitsStart::NextBar});
  long_position.exits(
   {Strategy::Exit{true, FalseNode{}, CloseNode{}, 0, 0.25}});
  long_position.exits_activation(ExitActivation::AfterPrevious);
  auto pyramiding = Strategy::Pyramiding{};
  pyramiding.price(CloseNode{});
  pyramiding.delay(0);
  pyramiding.retrigger(PyramidingRetrigger::AfterFalse);
  pyramiding.cooldown(4);
  pyramiding.favorable_stop_target_reference(
   StopTargetReferencePrice::InitialEntryPrice);
  pyramiding.unfavorable_stop_target_reference(
   StopTargetReferencePrice::LatestEntryPrice);
  long_position.pyramiding(pyramiding);
  long_position.risk_distance(
   RiskDistanceAmountNode<BacktestMethodContext>{10.0});
  long_position.stop_losses({Strategy::StopLoss{true, OpenNode{}, false, 0.5}});
  long_position.stop_losses_activation(ExitActivation::AfterPrevious);
  long_position.take_profits({Strategy::TakeProfit{true, ValueNode{120.0}, 0.75}});

  const auto strategy =
   Strategy{"Test", {}, std::move(long_position), Strategy::Position{}, {}};

  const auto strategy_json = jsoncons::ojson::parse(stringify_strategy(strategy));

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
  const auto& entry = strategy_json.at("positions").at("long").at("entry");
  EXPECT_FALSE(entry.contains("timing"));
  EXPECT_EQ(entry.at("price").at("method").as<std::string>(),
            "MARKET_DATA.CLOSE");
  EXPECT_EQ(entry.at("delay").as<unsigned>(), 0U);
  EXPECT_EQ(entry.at("priceExitsStart").as<std::string>(), "NEXT_BAR");
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
  const auto& exit =
   strategy_json.at("positions").at("long").at("exits").at("rules").at(0);
  EXPECT_FALSE(exit.contains("timing"));
  EXPECT_EQ(exit.at("price").at("method").as<std::string>(),
            "MARKET_DATA.CLOSE");
  EXPECT_EQ(exit.at("delay").as<unsigned>(), 0U);
  const auto& pyramid =
   strategy_json.at("positions").at("long").at("pyramiding");
  EXPECT_FALSE(pyramid.contains("timing"));
  EXPECT_EQ(pyramid.at("price").at("method").as<std::string>(),
            "MARKET_DATA.CLOSE");
  EXPECT_EQ(pyramid.at("delay").as<unsigned>(), 0U);
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
  EXPECT_EQ(strategy_json.at("positions")
             .at("long")
             .at("exits")
             .at("activation")
             .as<std::string>(),
            "AFTER_PREVIOUS");
}

TEST(StrategyParserTest, JsonconsConvTraitsRoundTripSchemaConfig)
{
  auto long_position = Strategy::Position{};
  long_position.entry(
   Strategy::Entry{TrueNode{}, CloseNode{}, 0, PriceExitsStart::NextBar});
  long_position.stop_losses({Strategy::StopLoss{true, OpenNode{}, false}});

  const auto strategy =
   Strategy{"Config Name", {}, std::move(long_position), Strategy::Position{}, {}};

  auto strategy_json_str = std::string{};
  jsoncons::encode_json(strategy, strategy_json_str);

  const auto strategy_json = jsoncons::ojson::parse(strategy_json_str);
  EXPECT_EQ(strategy_json.at("version").as<int>(), 1);
  EXPECT_FALSE(strategy_json.contains("name"));

  const auto decoded_strategy = jsoncons::decode_json<Strategy>(strategy_json_str);

  EXPECT_TRUE(decoded_strategy.name().empty());
  EXPECT_TRUE(decoded_strategy.equivalent_rules(strategy));
}

TEST(StrategyParserTest, DefaultStopLossAndEmptyExitCollections)
{
  const auto strategy = Strategy{};

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

TEST(StrategyParserTest, MinimalDocumentUsesFreshStrategyDefaultsAndIgnoresUnknownKeys)
{
  const auto parsed = parse_strategy(
   "Minimal", R"({"version":1,"executionPath":"legacy","unknown":42})");
  auto expected = Strategy{};
  expected.name("Minimal");
  EXPECT_EQ(parsed, expected);
}

TEST(StrategyParserTest, CommentsRemainSupportedAndTrailingCommasAreRejected)
{
  auto expected = Strategy{};
  expected.name("Commented");
  EXPECT_EQ(parse_strategy("Commented", "{/* comment */\"version\":1}"),
            expected);
  EXPECT_THROW(parse_strategy("Trailing", R"({"version":1,})"), std::exception);
}

TEST(StrategyParserTest, AcceptsLegacyIntrabarPathAndOmitsItFromExports)
{
  auto legacy_json = serialize_strategy_config_json(Strategy{});
  legacy_json["execution"] = jsoncons::ojson::object();
  legacy_json["execution"]["intrabarPath"] = "HIGH_FIRST";

  const auto strategy = parse_strategy_config_json("Legacy", legacy_json);
  auto expected = Strategy{};
  expected.name("Legacy");
  EXPECT_EQ(strategy, expected);

  const auto saved = jsoncons::ojson::parse(stringify_strategy(strategy));
  EXPECT_EQ(saved.at("version").as<int>(), 1);
  EXPECT_FALSE(saved.contains("execution"));
}

TEST(StrategyParserTest, RequiresCanonicalMethodAndLosslessIntegralValues)
{
  EXPECT_THROW(parse_strategy("No Method",
                           R"({"version":1,"series":{"value":{}}})"),
               std::exception);
  EXPECT_THROW(
   parse_strategy(
    "Null Params",
    R"({"version":1,"series":{"value":{"method":"INDICATOR.SMA","params":null}}})"),
   std::exception);

  const auto integral = parse_strategy(
   "Integral",
   R"({"version":1,"positions":{"long":{"pyramiding":{"cooldown":2.0}}}})");
  EXPECT_EQ(integral.long_position().pyramiding().cooldown(), 2U);
  for(const auto* cooldown :
      {"2.5", "-1", "4294967296", "1e100", "9007199254740993.0", "\"2\""}) {
    EXPECT_THROW(
     parse_strategy(
      "Invalid",
      std::string{R"({"version":1,"positions":{"long":{"pyramiding":{"cooldown":)"} +
       cooldown + R"(}}}})"),
     std::exception);
  }
}

TEST(StrategyParserTest, MigratesLegacyNextOpenToCanonicalSignalOrders)
{
  const auto strategy = parse_strategy("Legacy", R"({
    "version": 1,
    "positions": {"long": {
      "entry": {"timing": "NEXT_OPEN"},
      "exits": {"rules": [{"timing": "NEXT_OPEN"}]},
      "pyramiding": {"timing": "NEXT_OPEN"}
    }, "short": {}}
  })");
  const auto saved = serialize_strategy_config_json(strategy);
  const auto check_open = [](const jsoncons::ojson& order) {
    EXPECT_FALSE(order.contains("timing"));
    EXPECT_EQ(order.at("price").at("method").as<std::string>(),
              "MARKET_DATA.OPEN");
    EXPECT_EQ(order.at("delay").as<unsigned>(), 1U);
  };
  for(const auto* side : {"long", "short"}) {
    const auto& entry = saved.at("positions").at(side).at("entry");
    check_open(entry);
    EXPECT_EQ(entry.at("priceExitsStart").as<std::string>(), "CURRENT_BAR");
  }
  const auto& long_side = saved.at("positions").at("long");
  check_open(long_side.at("exits").at("rules").at(0));
  check_open(long_side.at("pyramiding"));
  EXPECT_EQ(parse_strategy_config_json("Legacy", saved), strategy);
}

TEST(StrategyParserTest, RoundTripsCustomEntryPriceAndCollectsItsInputs)
{
  const auto strategy = parse_strategy("Custom price", R"({
    "version": 1,
    "positions": {"long": {"entry": {
      "signal": true,
      "price": {"method":"OPERATOR.MULTIPLY","params":{
        "multiplicand":"MARKET_DATA.CLOSE",
        "multiplier":{"method":"INPUT.NUMERIC","params":{
          "label":"Entry price multiplier","value":0.95
        }}
      }},
      "delay": 2,
      "priceExitsStart": "NEXT_BAR"
    }}}
  })");
  const auto saved = serialize_strategy_config_json(strategy);
  const auto& entry = saved.at("positions").at("long").at("entry");
  EXPECT_FALSE(entry.contains("timing"));
  EXPECT_EQ(entry.at("price").at("method").as<std::string>(),
            "OPERATOR.MULTIPLY");
  EXPECT_EQ(entry.at("delay").as<unsigned>(), 2U);
  EXPECT_EQ(entry.at("priceExitsStart").as<std::string>(), "NEXT_BAR");
  EXPECT_EQ(parse_strategy_config_json("Custom price", saved), strategy);

  const auto inputs = collect_strategy_inputs(strategy);
  ASSERT_EQ(inputs.size(), 1U);
  EXPECT_EQ(inputs.front().label(), "Entry price multiplier");
  EXPECT_DOUBLE_EQ(inputs.front().value(), 0.95);
}

TEST(StrategyParserTest, RoundTripsCustomSignalExitAndPyramidPrices)
{
  const auto strategy = parse_strategy("Custom orders", R"({
    "version": 1,
    "positions": {"long": {
      "exits": {"rules": [{
        "enabled": true,
        "signal": true,
        "price": {"method":"INPUT.NUMERIC","params":{
          "label":"Exit price","value":105
        }},
        "delay": 2,
        "reduce": 0.5
      }]},
      "pyramiding": {
        "signal": true,
        "price": {"method":"INPUT.NUMERIC","params":{
          "label":"Pyramid price","value":110
        }},
        "delay": 3,
        "maxLayers": 2
      }
    }}
  })");
  const auto saved = serialize_strategy_config_json(strategy);
  const auto& side = saved.at("positions").at("long");
  const auto& exit = side.at("exits").at("rules").at(0);
  const auto& pyramid = side.at("pyramiding");
  EXPECT_FALSE(exit.contains("timing"));
  EXPECT_FALSE(pyramid.contains("timing"));
  EXPECT_EQ(exit.at("price").at("method").as<std::string>(), "INPUT.NUMERIC");
  EXPECT_EQ(pyramid.at("price").at("method").as<std::string>(),
            "INPUT.NUMERIC");
  EXPECT_EQ(exit.at("delay").as<unsigned>(), 2U);
  EXPECT_EQ(pyramid.at("delay").as<unsigned>(), 3U);
  EXPECT_EQ(parse_strategy_config_json("Custom orders", saved), strategy);

  const auto inputs = collect_strategy_inputs(strategy);
  ASSERT_EQ(inputs.size(), 2U);
  EXPECT_EQ(inputs[0].label(), "Exit price");
  EXPECT_EQ(inputs[1].label(), "Pyramid price");
}

TEST(StrategyParserTest, RejectsInvalidEntryPriceDelayAndExitStart)
{
  for(const auto* settings : {R"("delay": -1)",
                              R"("delay": 0.5)",
                              R"("delay": 1e100)",
                              R"("delay": "2")",
                              R"("price": null)",
                              R"("price": true)",
                              R"("price": "MARKET_DATA.UNKNOWN")",
                              R"("priceExitsStart": "UNKNOWN")"}) {
    SCOPED_TRACE(settings);
    EXPECT_THROW(
     parse_strategy(
      "Invalid",
      std::string{R"({"version":1,"positions":{"long":{"entry":{)"} + settings +
       R"(}}}})"),
     std::exception);
  }
}

TEST(StrategyParserTest, MissingStopLossesPreserveDefaultsAndEmptyDisablesStops)
{
  const auto risk_distance =
   R"("riskDistance":{"method":"RISK_DISTANCE.AMOUNT","params":{"amount":10}})";
  const auto missing = parse_strategy(
   "Missing",
   std::string{
    R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{)"} +
    risk_distance + R"(},"short":false}})");
  const auto empty = parse_strategy(
   "Empty",
   std::string{
    R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{)"} +
    risk_distance +
    R"(,"stopLosses":{"activation":"SIMULTANEOUS","rules":[]}},"short":false}})");
  const auto partial = parse_strategy(
   "Partial",
   std::string{
    R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{)"} +
    risk_distance +
    R"(,"stopLosses":{"activation":"AFTER_PREVIOUS"}},"short":false}})");

  EXPECT_EQ(missing.long_position().stop_losses(),
            Strategy::Position{}.stop_losses());
  EXPECT_TRUE(empty.long_position().stop_losses().empty());
  EXPECT_EQ(partial.long_position().stop_losses(),
            Strategy::Position{}.stop_losses());
  EXPECT_EQ(partial.long_position().stop_losses_activation(),
            ExitActivation::AfterPrevious);
}

TEST(StrategyParserTest, RejectsInvalidStopLosses)
{
  const auto risk_distance =
   R"("riskDistance":{"method":"RISK_DISTANCE.AMOUNT","params":{"amount":10}})";
  EXPECT_THROW(
   parse_strategy(
    "Not Array",
    std::string{
     R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{)"} +
     risk_distance + R"(,"stopLosses":{"rules":{}}},"short":false}})"),
   std::runtime_error);
  EXPECT_THROW(
   parse_strategy(
    "Bad Item",
    std::string{
     R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{)"} +
     risk_distance +
     R"(,"stopLosses":{"activation":"SIMULTANEOUS","rules":[null]}},"short":false}})"),
   std::runtime_error);
}

TEST(StrategyParserTest, DefaultsMissingAndRejectsInvalidRiskDistance)
{
  const auto missing = parse_strategy(
   "Missing", R"({"version":1,"positions":{"long":{},"short":false}})");
  EXPECT_EQ(missing.long_position().risk_distance(),
            Strategy::Position{}.risk_distance());
  EXPECT_THROW(
   parse_strategy(
    "Numeric",
    R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{"riskDistance":10},"short":false}})"),
   std::runtime_error);
  EXPECT_THROW(
   parse_strategy(
    "WrongMethod",
    R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"positions":{"long":{"riskDistance":{"method":"VALUE.CONSTANT","params":{"value":10}}},"short":false}})"),
   std::runtime_error);
}

TEST(ConfigParserTest, RoundTripsExplicitRiskDistanceAndRStopMethods)
{
  auto parser = make_backtest_strategy_config_parser();
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

TEST(StrategyParserTest, LoadsEveryBundledStrategySample)
{
  const auto sample_directory =
   std::filesystem::path{PLUDUX_BACKTEST_SAMPLE_DIR};
  auto schema_stream =
   std::ifstream{sample_directory.parent_path() / "schemas" /
                 "pludux.backtest.strategy.v1-draft.json"};
  auto schema = jsoncons::ojson::parse(schema_stream);
  EXPECT_EQ(schema.at("$id").as<std::string>(),
            "https://pludux.com/schemas/pludux.backtest.strategy.v1-draft.json");
  EXPECT_EQ(schema.at("title").as<std::string>(),
            "Pludux Backtest Strategy Schema");
  EXPECT_EQ(schema.at("properties").at("version").at("title").as<std::string>(),
            "Strategy JSON format version 1");
  EXPECT_EQ(
   schema.at("properties").at("version").at("description").as<std::string>(),
   "Version of the Pludux Strategy JSON format. Must be 1.");
  EXPECT_EQ(schema.at("properties").at("version").at("const").as<int>(), 1);
  const auto compiled_schema =
   jsoncons::jsonschema::make_json_schema(std::move(schema));
  EXPECT_NO_THROW(
   compiled_schema.validate(jsoncons::ojson::parse(R"({"version":1})")));
  EXPECT_NO_THROW(compiled_schema.validate(
   jsoncons::ojson::parse(R"({"version":1,"unknown":true,"plots":[{}, {"items":[]}]})")));
  EXPECT_THROW(compiled_schema.validate(jsoncons::ojson::parse(R"({})")),
               std::exception);
  EXPECT_THROW(compiled_schema.validate(jsoncons::ojson::parse(
                R"({"version":1,"series":{"value":{}}})")),
               std::exception);
  EXPECT_NO_THROW(
   compiled_schema.validate(jsoncons::ojson::parse(stringify_strategy(Strategy{}))));
  auto operator_document = jsoncons::ojson::parse(stringify_strategy(Strategy{}));
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
    EXPECT_NO_THROW(parse_strategy(entry.path().stem().string(), json.str()))
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
   parse_strategy("Position R-Multiple", r_multiple_strategy.to_string()));

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
  EXPECT_NO_THROW(compiled_schema.validate(legacy_signal_fields));
  const auto legacy_fields_strategy =
   parse_strategy("Legacy fields", legacy_signal_fields.to_string());
  const auto& legacy_entry = legacy_fields_strategy.long_position().entry();
  EXPECT_NE(node_cast<OpenNode>(legacy_entry.price()), nullptr);
  EXPECT_EQ(legacy_entry.delay(), 1U);
  EXPECT_ANY_THROW(compiled_schema.validate(legacy_exit_array));
  EXPECT_GT(sample_count, 0);
}
