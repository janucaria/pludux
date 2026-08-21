#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <jsoncons/json.hpp>

import pludux.backtest;

using namespace pludux;
using namespace pludux::backtest;
using ModelConfigParser = ConfigParser<BacktestMethodContext>;
using pludux::backtest::make_backtest_model_config_parser;
using pludux::backtest::make_requested_order_comparator_config_parser;
using json = jsoncons::ojson;

using SignalAllOfNode = pludux::AllOfNode<BacktestMethodContext>;
using SignalAnyOfNode = pludux::AnyOfNode<BacktestMethodContext>;
using SignalAlwaysNode = pludux::TrueNode;
using SignalNeverNode = pludux::FalseNode;
using SignalEqualNode = pludux::EqualNode<BacktestMethodContext>;
using SignalNotEqualNode = pludux::NotEqualNode<BacktestMethodContext>;
using SignalGreaterThanNode =
 pludux::GreaterThanNode<BacktestMethodContext>;
using SignalGreaterEqualNode =
 pludux::GreaterEqualNode<BacktestMethodContext>;
using SignalLessThanNode = pludux::LessThanNode<BacktestMethodContext>;
using SignalLessEqualNode = pludux::LessEqualNode<BacktestMethodContext>;
using SignalCrossoverNode =
 pludux::CrossoverNode<BacktestMethodContext>;
using SignalCrossunderNode =
 pludux::CrossunderNode<BacktestMethodContext>;
using SignalNotNode = pludux::LogicalNotNode<BacktestMethodContext>;
using SignalAndNode = pludux::LogicalAndNode<BacktestMethodContext>;
using SignalOrNode = pludux::LogicalOrNode<BacktestMethodContext>;
using SignalXorNode = pludux::LogicalXorNode<BacktestMethodContext>;

TEST(ConfigParserInfrastructureTest,
     TypedParserRegistersParsesAndSerializesValueAndBinaryOperator)
{
  using Context = pludux::backtest::BacktestMethodContext;
  using Parser = pludux::backtest::ConfigParser<Context>;
  using Node = ErasedNode<Context>;
  using AddNode = BinaryOperatorNode<std::plus<>, Context>;

  auto parser = Parser{};
  parser.register_node_parser(
   "VALUE",
   [](const Parser&, const Node& node) -> json {
     const auto* value = node_cast<ValueNode>(node);
     return value ? json::object{{"value", value->value()}} : json::null();
   },
   [](Parser::Parser, const json& params) -> Node {
     return ValueNode{params.at("value").as_double()};
   });
  parser.register_node_parser(
   "ADD",
   [](const Parser& parser, const Node& node) -> json {
     const auto* add = node_cast<AddNode>(node);
     return add ? json::object{{"augend", parser.serialize_node(add->left())},
                               {"addend", parser.serialize_node(add->right())}}
                : json::null();
   },
   [](Parser::Parser parser, const json& params) -> Node {
     return AddNode{parser.parse_node(params.at("augend")),
                    parser.parse_node(params.at("addend"))};
   });

  const auto config = json::parse(R"(
    {"method":"ADD","params":{"augend":{"method":"VALUE","params":{"value":2}},"addend":{"method":"VALUE","params":{"value":3}}}}
  )");
  const auto node = parser.parse_node(config);
  const auto* add = node_cast<AddNode>(node);
  ASSERT_NE(add, nullptr);
  const auto* augend = node_cast<ValueNode>(add->left());
  const auto* addend = node_cast<ValueNode>(add->right());
  ASSERT_NE(augend, nullptr);
  ASSERT_NE(addend, nullptr);
  EXPECT_EQ(augend->value(), 2.0);
  EXPECT_EQ(addend->value(), 3.0);
  EXPECT_EQ(parser.serialize_node(node), config);
}

TEST(ModelConfigParserTest, ParsesShorthandsDefaultsAndTypedNodes)
{
  auto parser = make_backtest_model_config_parser();
  const auto config = json::parse(R"(
    {"method":"SMA","params":{"source":{"method":"ADD","params":{"augend":"CLOSE","addend":2}}}}
  )");

  const auto node = parser.parse_node(config);
  const auto* sma = node_cast<SmaNode<backtest::BacktestMethodContext>>(node);
  ASSERT_NE(sma, nullptr);
  EXPECT_NE(node_cast<NumericInputNode>(sma->period()), nullptr);
  EXPECT_EQ(parser.parse_node(parser.serialize_node(node)), node);
}

TEST(ModelConfigParserTest, RejectsMethodsOutsideBacktestContext)
{
  auto parser = make_backtest_model_config_parser();
  for(const auto* comparator_only : {
       "REQUESTED_ORDER_PRICE", "REQUESTED_ORDER_DIRECTION",
       "IS_PYRAMIDING_ORDER", "RAW_REQUESTED_QUANTITY",
       "RAW_REQUESTED_QUANTITY_LIMIT", "DRAWDOWN_ADJUSTED_QUANTITY",
       "DRAWDOWN_ADJUSTED_QUANTITY_LIMIT", "REQUESTED_QUANTITY",
       "REQUESTED_NOTIONAL", "REQUESTED_COST", "ESTIMATED_ENTRY_FEE",
       "ESTIMATED_1R_EXIT_FEE", "REQUESTED_ORDER_RISK_DISTANCE",
       "REQUESTED_PRICE_RISK", "REQUESTED_RISK_WITH_FEES",
       "FROZEN_UNIT_QUANTITY"}) {
    SCOPED_TRACE(comparator_only);
    EXPECT_THROW(parser.parse_node(json::parse(std::string{"{\"method\":\""} +
                                               comparator_only + "\"}")),
                 std::invalid_argument);
  }
  EXPECT_THROW(parser.parse_node(json::parse(
                R"({"method":"MODEL_PERFORMANCE","params":{"metric":"WIN_RATE"}})")),
               std::invalid_argument);
}

TEST(ModelConfigParserTest, PreservesCanonicalCompositeDefaults)
{
  auto parser = make_backtest_model_config_parser();
  const auto config = json::parse(R"({"method":"MACD"})");

  const auto node = parser.parse_node(config);
  const auto* macd = node_cast<MacdNode<backtest::BacktestMethodContext>>(node);
  ASSERT_NE(macd, nullptr);
  EXPECT_NE(node_cast<CloseNode>(macd->source()), nullptr);
  EXPECT_NE(node_cast<NumericInputNode>(macd->fast_period()), nullptr);
  EXPECT_NE(node_cast<NumericInputNode>(macd->slow_period()), nullptr);
  EXPECT_NE(node_cast<NumericInputNode>(macd->signal_period()), nullptr);
  EXPECT_EQ(parser.parse_node(parser.serialize_node(node)), node);
}

TEST(ModelConfigParserTest, RoundTripsEveryCanonicalModelRegistration)
{
  auto parser = make_backtest_model_config_parser();
  const auto configurations = std::vector<std::string>{
   "0", "true", "false", "\"OPEN\"", "\"HIGH\"", "\"LOW\"",
   "\"CLOSE\"", "\"VOLUME\"", "\"EQUITY\"", "\"EQUITY_PERCENT\"",
   "\"DRAWDOWN\"", "\"PYRAMIDING_LAYER\"", "\"INITIAL_ENTRY_PRICE\"",
   "\"LATEST_ENTRY_PRICE\"", "\"AVERAGE_PRICE\"", "\"STOP_TARGET_REF_PRICE\"",
   "\"POSITION_DIRECTION\"", "\"SL_1R\"", "\"ALWAYS\"", "\"NEVER\"",
   R"({"method":"VALUE","params":{"value":1.25}})",
   R"({"method":"DATA","params":{"field":"custom"}})",
   R"({"method":"SERIES","params":{"name":"basis"}})",
   R"({"method":"INPUT","params":{"label":"Period","representation":"SignedInteger","value":-2}})",
   R"({"method":"CHANGE"})",
   R"({"method":"LOOKBACK","params":{"period":2}})",
   R"({"method":"SMA"})", R"({"method":"EMA"})", R"({"method":"WMA"})",
   R"({"method":"RMA"})", R"({"method":"HMA"})", R"({"method":"RSI"})",
   R"({"method":"HIGHEST"})", R"({"method":"LOWEST"})", R"({"method":"ROC"})",
   R"({"method":"STDDEV"})", R"({"method":"RVOL"})", R"({"method":"TR"})",
   R"({"method":"ATR","params":{"maSmoothingType":"EMA"}})",
   R"({"method":"KC"})", R"({"method":"DC"})",
   R"({"method":"BB"})", R"({"method":"MACD"})", R"({"method":"STOCH"})",
   R"({"method":"STOCH_RSI"})",
   R"({"method":"SELECT_OUTPUT","params":{"output":"upper-band","source":"CLOSE"}})",
   R"({"method":"PERCENTAGE"})", R"({"method":"POSITION_R_MULTIPLE"})",
   R"({"method":"SL_AMOUNT"})", R"({"method":"TP_AMOUNT"})",
   R"({"method":"SL_PERCENT"})", R"({"method":"TP_PERCENT"})",
   R"({"method":"SL_ATR"})", R"({"method":"TP_ATR"})",
   R"({"method":"R_DISTANCE_AMOUNT"})",
   R"({"method":"R_DISTANCE_PERCENTAGE"})", R"({"method":"R_DISTANCE_ATR"})",
   R"({"method":"SL_R_MULTIPLE"})", R"({"method":"TP_R_MULTIPLE"})",
   R"({"method":"ADD","params":{"augend":1,"addend":2}})",
   R"({"method":"SUBTRACT","params":{"minuend":1,"subtrahend":2}})",
   R"({"method":"MULTIPLY","params":{"multiplicand":1,"multiplier":2}})",
   R"({"method":"DIVIDE","params":{"dividend":1,"divisor":2}})",
   R"({"method":"ABS_DIFF","params":{"minuend":1,"subtrahend":2}})",
   R"({"method":"MAX","params":{"left":1,"right":2}})",
   R"({"method":"MIN","params":{"left":1,"right":2}})",
   R"({"method":"NEGATE","params":{"operand":1}})",
   R"({"method":"ABS","params":{"operand":-1}})",
   R"({"method":"SQRT","params":{"operand":4}})",
   R"({"method":"POSITIVE_PART","params":{"operand":1}})",
   R"({"method":"NEGATIVE_PART","params":{"operand":-1}})",
   R"({"method":"GREATER_THAN","params":{"target":2,"threshold":1}})",
   R"({"method":"GREATER_EQUAL","params":{"target":2,"threshold":1}})",
   R"({"method":"LESS_THAN","params":{"target":1,"threshold":2}})",
   R"({"method":"LESS_EQUAL","params":{"target":1,"threshold":2}})",
   R"({"method":"EQUAL","params":{"target":1,"threshold":1}})",
   R"({"method":"NOT_EQUAL","params":{"target":1,"threshold":2}})",
   R"({"method":"CROSSOVER","params":{"value":"CLOSE","baseline":"OPEN"}})",
   R"({"method":"CROSSUNDER","params":{"value":"CLOSE","baseline":"OPEN"}})",
   R"({"method":"AND","params":{"firstCondition":true,"secondCondition":false}})",
   R"({"method":"OR","params":{"firstCondition":true,"secondCondition":false}})",
   R"({"method":"XOR","params":{"firstCondition":true,"secondCondition":false}})",
   R"({"method":"NOT","params":{"condition":true}})",
   R"({"method":"ALL_OF","params":{"items":[true,false]}})",
   R"({"method":"ANY_OF","params":{"items":[true,false]}})"};

  for(const auto& configuration : configurations) {
    SCOPED_TRACE(configuration);
    const auto config = json::parse(configuration);
    const auto node = parser.parse_node(config);
    EXPECT_EQ(parser.parse_node(parser.serialize_node(node)), node);
  }
}

TEST(ModelConfigParserTest,
     RoundTripsScalarOperationsUsingBacktestMethodContextNodes)
{
  auto parser = make_backtest_model_config_parser();

  const auto absolute = parser.parse_node(
   json::parse(R"({"method":"ABS","params":{"operand":-1}})"));
  EXPECT_NE((node_cast<UnaryOperatorNode<Absolute<>, BacktestMethodContext>>(
              absolute)),
            nullptr);
  EXPECT_EQ(parser.parse_node(parser.serialize_node(absolute)), absolute);

  const auto maximum = parser.parse_node(
   json::parse(R"({"method":"MAX","params":{"left":1,"right":2}})"));
  EXPECT_NE((node_cast<BinaryOperatorNode<Maximum<>, BacktestMethodContext>>(
              maximum)),
            nullptr);
  EXPECT_EQ(parser.parse_node(parser.serialize_node(maximum)), maximum);

  const auto minimum = parser.parse_node(
   json::parse(R"({"method":"MIN","params":{"left":1,"right":2}})"));
  EXPECT_NE((node_cast<BinaryOperatorNode<Minimum<>, BacktestMethodContext>>(
              minimum)),
            nullptr);
  EXPECT_EQ(parser.parse_node(parser.serialize_node(minimum)), minimum);

  const auto positive_part = parser.parse_node(
   json::parse(R"({"method":"POSITIVE_PART","params":{"operand":1}})"));
  EXPECT_NE((node_cast<UnaryOperatorNode<PositivePart<>, BacktestMethodContext>>(
              positive_part)),
            nullptr);
  EXPECT_EQ(parser.parse_node(parser.serialize_node(positive_part)),
            positive_part);

  const auto negative_part = parser.parse_node(
   json::parse(R"({"method":"NEGATIVE_PART","params":{"operand":-1}})"));
  EXPECT_NE((node_cast<UnaryOperatorNode<NegativePart<>, BacktestMethodContext>>(
              negative_part)),
            nullptr);
  EXPECT_EQ(parser.parse_node(parser.serialize_node(negative_part)),
            negative_part);
}

TEST(RequestedOrderComparatorConfigParserTest,
     SupportsOnlyComparatorGrammarAndProducesTypedNodes)
{
  auto parser = make_requested_order_comparator_config_parser();
  const auto config = json::parse(R"(
    {"method":"DIVIDE","params":{"dividend":{"method":"REQUESTED_NOTIONAL"},"divisor":{"method":"LOOKBACK","params":{"period":2,"source":"CLOSE"}}}}
  )");
  const auto node = parser.parse_node(config);
  const auto* divide =
   node_cast<BinaryOperatorNode<std::divides<>,
                                backtest::RequestedOrderMethodContext>>(node);
  EXPECT_NE(divide, nullptr);
  const auto serialized = parser.serialize_node(node);
  EXPECT_EQ(parser.parse_node(serialized), node);
  EXPECT_THROW(parser.parse_node(json::parse(R"({"method":"SERIES","params":{"name":"x"}})")),
               std::invalid_argument);
  EXPECT_THROW(parser.parse_node(json::parse(R"({"method":"ATR"})")), std::invalid_argument);
  EXPECT_THROW(parser.parse_node(json::parse(R"({"method":"EQUITY"})")),
               std::invalid_argument);
  EXPECT_THROW(parser.parse_node(json::parse("true")), std::invalid_argument);
  EXPECT_THROW(parser.parse_node(json::parse("false")), std::invalid_argument);
}

TEST(RequestedOrderComparatorConfigParserTest,
     RoundTripsEveryCanonicalComparatorRegistrationAndRejectsModelOnlyNodes)
{
  auto parser = make_requested_order_comparator_config_parser();
  const auto configurations = std::vector<std::string>{
   "1", "\"OPEN\"", "\"HIGH\"", "\"LOW\"", "\"CLOSE\"", "\"VOLUME\"",
   R"({"method":"VALUE","params":{"value":1.25}})",
   R"({"method":"DATA","params":{"field":"custom"}})",
   R"({"method":"LOOKBACK","params":{"period":2}})",
   R"({"method":"REQUESTED_ORDER_PRICE"})",
   R"({"method":"REQUESTED_ORDER_DIRECTION"})",
   R"({"method":"IS_PYRAMIDING_ORDER"})",
   R"({"method":"RAW_REQUESTED_QUANTITY"})",
   R"({"method":"RAW_REQUESTED_QUANTITY_LIMIT"})",
   R"({"method":"DRAWDOWN_ADJUSTED_QUANTITY"})",
   R"({"method":"DRAWDOWN_ADJUSTED_QUANTITY_LIMIT"})",
   R"({"method":"REQUESTED_QUANTITY"})", R"({"method":"REQUESTED_NOTIONAL"})",
   R"({"method":"REQUESTED_COST"})", R"({"method":"ESTIMATED_ENTRY_FEE"})",
   R"({"method":"ESTIMATED_1R_EXIT_FEE"})",
   R"({"method":"REQUESTED_ORDER_RISK_DISTANCE"})",
   R"({"method":"REQUESTED_PRICE_RISK"})",
   R"({"method":"REQUESTED_RISK_WITH_FEES"})",
   R"({"method":"FROZEN_UNIT_QUANTITY"})",
   R"({"method":"ADD","params":{"augend":1,"addend":2}})",
   R"({"method":"SUBTRACT","params":{"minuend":1,"subtrahend":2}})",
   R"({"method":"MULTIPLY","params":{"multiplicand":1,"multiplier":2}})",
   R"({"method":"DIVIDE","params":{"dividend":1,"divisor":2}})",
   R"({"method":"ABS_DIFF","params":{"left":1,"right":2}})",
   R"({"method":"MAX","params":{"left":1,"right":2}})",
   R"({"method":"MIN","params":{"left":1,"right":2}})",
   R"({"method":"NEGATE","params":{"operand":1}})",
   R"({"method":"ABS","params":{"operand":1}})",
   R"({"method":"SQRT","params":{"operand":4}})",
   R"({"method":"POSITIVE_PART","params":{"operand":1}})",
   R"({"method":"NEGATIVE_PART","params":{"operand":-1}})"};

  for(const auto& configuration : configurations) {
    SCOPED_TRACE(configuration);
    const auto config = json::parse(configuration);
    const auto node = parser.parse_node(config);
    EXPECT_EQ(parser.parse_node(parser.serialize_node(node)), node);
  }

  for(const auto* model_only : {
       "EQUITY", "EQUITY_PERCENT", "DRAWDOWN", "CHANGE", "SMA", "EMA",
       "WMA", "RMA", "HMA", "RSI", "HIGHEST", "LOWEST", "ROC", "RVOL",
       "ATR", "TR", "KC", "DC", "SERIES", "INPUT", "SELECT_OUTPUT", "BB",
       "MACD", "STOCH", "STOCH_RSI", "PERCENTAGE", "SL_AMOUNT", "TP_AMOUNT",
       "SL_PERCENT", "TP_PERCENT", "SL_ATR", "TP_ATR", "R_DISTANCE_AMOUNT",
       "R_DISTANCE_PERCENTAGE", "R_DISTANCE_ATR", "SL_1R", "SL_R_MULTIPLE",
       "TP_R_MULTIPLE", "INITIAL_ENTRY_PRICE", "LATEST_ENTRY_PRICE",
       "AVERAGE_PRICE", "STOP_TARGET_REF_PRICE", "POSITION_DIRECTION",
       "PYRAMIDING_LAYER", "POSITION_R_MULTIPLE", "GREATER_THAN",
       "GREATER_EQUAL", "LESS_THAN", "LESS_EQUAL", "EQUAL", "NOT_EQUAL",
       "CROSSOVER", "CROSSUNDER", "ALWAYS", "NEVER", "AND", "OR", "NOT",
       "XOR", "ALL_OF", "ANY_OF"}) {
    SCOPED_TRACE(model_only);
    EXPECT_THROW(parser.parse_node(json::parse(std::string{"{\"method\":\""} +
                                               model_only + "\"}")),
                 std::invalid_argument);
  }
}

auto node_context = NodeToErasedMethodContext{};

struct HistogramRenderCapture {
  std::vector<std::uint32_t> colors;
};

class PlotMethodTestContext {
public:
  PlotMethodTestContext(std::vector<double> data,
                        std::shared_ptr<HistogramRenderCapture> capture)
  : data_{std::make_shared<std::vector<double>>(std::move(data))}
  , capture_{std::move(capture)}
  {
  }

  void render_plot_line(const std::vector<double>&, std::uint32_t)
  {
  }

  void render_plot_histogram(const std::vector<double>&, std::uint32_t)
  {
  }

  void render_plot_momentum_histogram(const std::vector<double>&,
                                      const std::vector<std::uint32_t>& colors)
  {
    capture_->colors = colors;
  }

  auto series_results(const std::string&) const
   -> std::optional<std::reference_wrapper<const std::vector<double>>>
  {
    return std::cref(*data_);
  }

  auto results_size() const -> std::size_t
  {
    return data_->size();
  }

private:
  std::shared_ptr<std::vector<double>> data_;
  std::shared_ptr<HistogramRenderCapture> capture_;
};

class ParsedConfigNodeMethod {
public:
  ParsedConfigNodeMethod(ErasedNode<BacktestMethodContext> node)
  : node_{std::move(node)}
  , method_{
     node_to_erased_method<BacktestMethodContext>(node_, node_context)}
  {
  }

  auto method(this const ParsedConfigNodeMethod& self) noexcept
   -> const ErasedSeriesMethod<BacktestMethodContext>&
  {
    return self.method_;
  }

  auto node(this const ParsedConfigNodeMethod& self) noexcept
   -> const ErasedNode<BacktestMethodContext>&
  {
    return self.node_;
  }

  auto operator==(this const ParsedConfigNodeMethod& self,
                  const ParsedConfigNodeMethod& other) noexcept -> bool
  {
    return self.method_ == other.method_;
  }

private:
  ErasedNode<BacktestMethodContext> node_;
  ErasedSeriesMethod<BacktestMethodContext> method_;
};

template<typename UMethod>
auto series_method_cast(
 const ParsedConfigNodeMethod& parsed_node_method) noexcept -> const UMethod*
{
  const auto& method = parsed_node_method.method();
  return series_method_cast<UMethod>(method);
}

auto value_method_value(
 const ErasedSeriesMethod<BacktestMethodContext>& method) noexcept -> double
{
  const auto* value_method = series_method_cast<ValueMethod>(method);
  EXPECT_NE(value_method, nullptr);
  return value_method == nullptr ? 0.0 : value_method->value();
}

class ConfigParserTest : public ::testing::Test {
protected:
  ModelConfigParser config_parser;

  auto parse_node_method(const jsoncons::ojson& config)
   -> ParsedConfigNodeMethod
  {
    return ParsedConfigNodeMethod{config_parser.parse_node(config)};
  }

  auto
  serialize_node_method(const ParsedConfigNodeMethod& parsed_node_method) const
   -> jsoncons::ojson
  {
    return config_parser.serialize_node(parsed_node_method.node());
  }

  void SetUp() override
  {
    config_parser = std::move(make_backtest_model_config_parser());

    parse_node_method(json::parse(R"(
      {
        "method": "DATA",
        "params": {
          "name": "high",
          "field": "high"
        }
      }
    )"));

    parse_node_method(json::parse(R"(
      {
        "method": "DATA",
        "params": {
          "name": "low",
          "field": "low"
        }
      }
    )"));

    parse_node_method(json::parse(R"(
      {
        "method": "DATA",
        "params": {
          "name": "close",
          "field": "close"
        }
      }
    )"));

    parse_node_method(json::parse(R"(
      {
        "method": "DATA",
        "params": {
          "name": "volume",
          "field": "volume"
        }
      }
    )"));
  }
};

TEST(PlotMethodParserTest, ParseAndSerializeMomentumHistogram)
{
  const auto config = json::parse(R"(
    {
      "version": 1,
      "plots": [{
        "items": [{
          "method": "MOMENTUM_HISTOGRAM",
          "params": {
            "source": {
              "method": "SERIES",
              "params": {
                "name": "histogram"
              }
            },
            "positiveRisingColor": "#26a69a",
            "positiveFallingColor": "#b2dfdb",
            "negativeFallingColor": "#ef5350",
            "negativeRisingColor": "#ffcdd2"
          }
        }]
      }],
      "execution": {
        "intrabarPath": "CANDLE_DIRECTION"
      }
    }
  )");

  const auto strategy =
    backtest::parse_model_config_json("Test", config);
  const auto& method = strategy.plots().at(0).items().at(0);
  using MomentumHistogram = backtest::MomentumHistogramPlotMethod<
   backtest::ErasedPlotSourceMethod<backtest::ErasedPlotMethodContext>>;
  const auto* histogram = plot_method_cast<MomentumHistogram>(method);
  ASSERT_NE(histogram, nullptr);
  EXPECT_EQ(histogram->positive_rising_color(), 0xFF9AA626);
  EXPECT_EQ(histogram->positive_falling_color(), 0xFFDBDFB2);
  EXPECT_EQ(histogram->negative_falling_color(), 0xFF5053EF);
  EXPECT_EQ(histogram->negative_rising_color(), 0xFFD2CDFF);

  const auto capture = std::make_shared<HistogramRenderCapture>();
  method(PlotMethodTestContext{{1.0, 2.0, 1.0, -1.0, -2.0, -1.0}, capture});
  EXPECT_EQ(
   capture->colors,
   (std::vector<std::uint32_t>{
    0xFF9AA626, 0xFF9AA626, 0xFFDBDFB2, 0xFF5053EF, 0xFF5053EF, 0xFFD2CDFF}));

  const auto serialized =
    backtest::serialize_model_config_json(strategy);
  const auto& serialized_params =
   serialized.at("plots").at(0).at("items").at(0).at("params");
  EXPECT_TRUE(serialized_params.contains("positiveRisingColor"));
  EXPECT_TRUE(serialized_params.contains("positiveFallingColor"));
  EXPECT_TRUE(serialized_params.contains("negativeFallingColor"));
  EXPECT_TRUE(serialized_params.contains("negativeRisingColor"));

  const auto round_trip =
    backtest::parse_model_config_json("Test", serialized);
  EXPECT_EQ(method, round_trip.plots().at(0).items().at(0));
}

TEST(PlotMethodParserTest, RejectMomentumHistogramWithMissingColor)
{
  const auto config = json::parse(R"(
    {
      "version": 1,
      "plots": [{
        "items": [{
          "method": "MOMENTUM_HISTOGRAM",
          "params": {
            "source": {
              "method": "CONSTANT",
              "params": {
                "value": 1
              }
            },
            "positiveRisingColor": "#26a69a",
            "positiveFallingColor": "#b2dfdb",
            "negativeFallingColor": "#ef5350"
          }
        }]
      }],
      "execution": {
        "intrabarPath": "CANDLE_DIRECTION"
      }
    }
  )");

   EXPECT_THROW(backtest::parse_model_config_json("Test", config),
               std::invalid_argument);
}

TEST_F(ConfigParserTest, ParseScreenerSeriesMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SERIES",
      "params": {
        "name": "close"
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto series_method = series_method_cast<SeriesMethod>(method);
  ASSERT_NE(series_method, nullptr);

  EXPECT_EQ(series_method->name(), "close");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerLookbackMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "LOOKBACK",
      "params": {
        "period": 3,
        "source": {
          "method": "CLOSE"
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto lookback_method = series_method_cast<
   LookbackMethod<ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(lookback_method, nullptr);

  EXPECT_EQ(lookback_method->period(), 3);
  const auto source_method =
   series_method_cast<CloseMethod>(lookback_method->source());
  ASSERT_NE(source_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseNumericInputNode)
{
  const auto config = json::parse(R"(
    {
      "method": "INPUT",
      "params": {
        "label": "Length",
        "representation": "UnsignedInteger",
        "value": 14.9
      }
    }
  )");

  const auto parsed_node = config_parser.parse_node(config);
  const auto* input_node = node_cast<NumericInputNode>(parsed_node);

  ASSERT_NE(input_node, nullptr);
  EXPECT_EQ(input_node->label(), "Length");
  EXPECT_EQ(input_node->representation(),
            NumericInputNode::ValueRepresentation::UnsignedInteger);
  EXPECT_DOUBLE_EQ(input_node->value(), 14.9);

  const auto serialized_config = config_parser.serialize_node(parsed_node);
  const auto deserialized_node = config_parser.parse_node(serialized_config);
  EXPECT_EQ(parsed_node, deserialized_node);
}

TEST_F(ConfigParserTest, ParseScreenerSelectOutputMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SELECT_OUTPUT",
      "params": {
        "output": "upper-band",
        "source": {
          "method": "MACD",
          "params": {
            "fastPeriod": 12,
            "slowPeriod": 26,
            "signalPeriod": 9,
            "input": {
              "method": "CLOSE"
            }
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto select_output_method = series_method_cast<
   SelectOutputMethod<ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(select_output_method, nullptr);
  EXPECT_EQ(select_output_method->output(), MethodOutput::UpperBand);

  const auto macd_method = series_method_cast<
   MacdMethod<ErasedSeriesMethod<BacktestMethodContext>,
              ErasedSeriesMethod<BacktestMethodContext>>>(
   select_output_method->source());
  ASSERT_NE(macd_method, nullptr);

  const auto source = series_method_cast<CloseMethod>(macd_method->source());
  EXPECT_NE(source, nullptr);
  EXPECT_EQ(value_method_value(macd_method->fast_period()), 12);
  EXPECT_EQ(value_method_value(macd_method->slow_period()), 26);
  EXPECT_EQ(value_method_value(macd_method->signal_period()), 9);

  const auto serialized_config = serialize_node_method(method);
  const auto expected_serialized_config = json::parse(R"(
    {
      "method": "SELECT_OUTPUT",
      "params": {
        "output": "upper-band",
        "source": {
          "method": "MACD",
          "params": {
            "fastPeriod": {"method": "VALUE", "params": {"value": 12}},
            "slowPeriod": {"method": "VALUE", "params": {"value": 26}},
            "signalPeriod": {"method": "VALUE", "params": {"value": 9}},
            "source": {"method": "CLOSE"}
          }
        }
      }
    }
  )");
  EXPECT_EQ(serialized_config, expected_serialized_config);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerOpenMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "OPEN"
    }
  )");

  const auto method = parse_node_method(config);

  const auto open_method = series_method_cast<OpenMethod>(method);
  ASSERT_NE(open_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerHighMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "HIGH"
    }
  )");

  const auto method = parse_node_method(config);

  const auto high_method = series_method_cast<HighMethod>(method);
  ASSERT_NE(high_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerLowMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "LOW"
    }
  )");

  const auto method = parse_node_method(config);

  const auto low_method = series_method_cast<LowMethod>(method);
  ASSERT_NE(low_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerCloseMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "CLOSE"
    }
  )");

  const auto method = parse_node_method(config);

  const auto close_method = series_method_cast<CloseMethod>(method);
  ASSERT_NE(close_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerVolumeMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "VOLUME"
    }
  )");

  const auto method = parse_node_method(config);

  const auto volume_method = series_method_cast<VolumeMethod>(method);
  ASSERT_NE(volume_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerSmaMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SMA",
      "params": {
        "period": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 14
          }
        },
        "source": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto sma_method =
   series_method_cast<SmaMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
  ASSERT_NE(sma_method, nullptr);

  EXPECT_EQ(value_method_value(sma_method->period()), 14);

  const auto source = series_method_cast<DataMethod>(sma_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "close");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerEmaMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "EMA",
      "params": {
        "period": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 10
          }
        },
        "source": {
          "method": "DATA",
          "params": {
            "field": "open"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto ema_method =
   series_method_cast<EmaMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
  ASSERT_NE(ema_method, nullptr);

  EXPECT_EQ(value_method_value(ema_method->period()), 10);

  const auto source = series_method_cast<DataMethod>(ema_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "open");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerWmaMethod)
{
  const auto config = json::parse(R"(
      {
        "method": "WMA",
        "params": {
          "period": {
            "method": "INPUT",
            "params": {
              "representation": "UnsignedInteger",
              "value": 20
            }
          },
          "source": {
            "method": "DATA",
            "params": {
              "field": "high"
            }
          }
        }
      }
    )");

  const auto method = parse_node_method(config);

  const auto wma_method =
   series_method_cast<WmaMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
  ASSERT_NE(wma_method, nullptr);

  EXPECT_EQ(value_method_value(wma_method->period()), 20);

  const auto source = series_method_cast<DataMethod>(wma_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "high");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerRmaMethod)
{
  const auto config = json::parse(R"(
      {
        "method": "RMA",
        "params": {
          "period": {
            "method": "INPUT",
            "params": {
              "representation": "UnsignedInteger",
              "value": 15
            }
          },
          "source": {
            "method": "DATA",
            "params": {
              "field": "low"
            }
          }
        }
      }
    )");

  const auto method = parse_node_method(config);

  const auto rma_method =
   series_method_cast<RmaMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
  ASSERT_NE(rma_method, nullptr);

  EXPECT_EQ(value_method_value(rma_method->period()), 15);

  const auto source = series_method_cast<DataMethod>(rma_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "low");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerHmaMethod)
{
  const auto config = json::parse(R"(
      {
        "method": "HMA",
        "params": {
          "period": {
            "method": "INPUT",
            "params": {
              "representation": "UnsignedInteger",
              "value": 25
            }
          },
          "source": {
            "method": "DATA",
            "params": {
              "field": "volume"
            }
          }
        }
      }
    )");

  const auto method = parse_node_method(config);

  const auto hma_method =
   series_method_cast<HmaMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
  ASSERT_NE(hma_method, nullptr);

  EXPECT_EQ(value_method_value(hma_method->period()), 25);

  const auto source = series_method_cast<DataMethod>(hma_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "volume");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerRsiMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "RSI",
      "params": {
        "period": 14,
        "source": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto rsi_method =
   series_method_cast<RsiMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
  ASSERT_NE(rsi_method, nullptr);

  EXPECT_EQ(value_method_value(rsi_method->period()), 14);

  const auto source = series_method_cast<DataMethod>(rsi_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "close");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerStddevMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "STDDEV",
      "params": {
        "period": 20,
        "source": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto stddev_method = series_method_cast<
   StddevMethod<ErasedSeriesMethod<BacktestMethodContext>,
                ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(stddev_method, nullptr);

  EXPECT_EQ(value_method_value(stddev_method->period()), 20);

  const auto source = series_method_cast<DataMethod>(stddev_method->source());
  ASSERT_NE(source, nullptr);

  EXPECT_EQ(source->field(), "close");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerValueMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "VALUE",
      "params": {
        "value": 100
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto value_method = series_method_cast<ValueMethod>(method);
  ASSERT_NE(value_method, nullptr);

  EXPECT_EQ(value_method->value(), 100);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerDataMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "DATA",
      "params": {
        "field": "open"
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto field_method = series_method_cast<DataMethod>(method);
  ASSERT_NE(field_method, nullptr);

  EXPECT_EQ(field_method->field(), "open");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseEquityMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "EQUITY"
    }
  )");

  const auto method = parse_node_method(config);

  const auto equity_method =
   series_method_cast<pludux::backtest::EquityMethod>(method);
  ASSERT_NE(equity_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseEquityPercentMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "EQUITY_PERCENT"
    }
  )");

  const auto method = parse_node_method(config);

  const auto equity_percent_method =
   series_method_cast<pludux::backtest::EquityPercentMethod>(method);
  ASSERT_NE(equity_percent_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseDrawdownMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "DRAWDOWN"
    }
  )");

  const auto method = parse_node_method(config);

  const auto drawdown_method =
   series_method_cast<pludux::backtest::DrawdownMethod>(method);
  ASSERT_NE(drawdown_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerAtrMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ATR",
      "params": {
        "period": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 14
          }
        },
        "maSmoothingType": "RMA"
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto atr_method =
   series_method_cast<AtrMethod<ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
  ASSERT_NE(atr_method, nullptr);

  EXPECT_EQ(value_method_value(atr_method->period()), 14);
  EXPECT_EQ(atr_method->ma_smoothing_type(), MaMethodType::Rma);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerBbMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "BB",
      "params": {
        "maType": "SMA",
        "period": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 20
          }
        },
        "stddev": {
          "method": "INPUT",
          "params": {
            "representation": "Decimal",
            "value": 2.0
          }
        },
        "maSource": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto bb_method =
   series_method_cast<BbMethod<ErasedSeriesMethod<BacktestMethodContext>,
                               ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
  ASSERT_NE(bb_method, nullptr);

  const auto ma_source = series_method_cast<DataMethod>(bb_method->source());
  EXPECT_NE(ma_source, nullptr);
  EXPECT_EQ(bb_method->ma_method_type(), MaMethodType::Sma);
  EXPECT_EQ(value_method_value(bb_method->period()), 20);
  EXPECT_EQ(value_method_value(bb_method->stddev()), 2.0);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerMacdMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "MACD",
      "params": {
        "fastPeriod": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 12
          }
        },
        "slowPeriod": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 26
          }
        },
        "signalPeriod": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 9
          }
        },
        "source": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto macd_method = series_method_cast<
   MacdMethod<ErasedSeriesMethod<BacktestMethodContext>,
              ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(macd_method, nullptr);

  const auto source = series_method_cast<DataMethod>(macd_method->source());
  EXPECT_NE(source, nullptr);
  EXPECT_EQ(value_method_value(macd_method->fast_period()), 12);
  EXPECT_EQ(value_method_value(macd_method->slow_period()), 26);
  EXPECT_EQ(value_method_value(macd_method->signal_period()), 9);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerStochMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "STOCH",
      "params": {
        "kPeriod": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 5
          }
        },
        "kSmooth": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 3
          }
        },
        "dPeriod": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 3
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto stoch_method = series_method_cast<
   StochMethod<ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(stoch_method, nullptr);

  EXPECT_EQ(value_method_value(stoch_method->k_period()), 5);
  EXPECT_EQ(value_method_value(stoch_method->k_smooth()), 3);
  EXPECT_EQ(value_method_value(stoch_method->d_period()), 3);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerStochRsiMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "STOCH_RSI",
      "params": {
        "rsiPeriod": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 14
          }
        },
        "kPeriod": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 5
          }
        },
        "kSmooth": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 3
          }
        },
        "dPeriod": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 3
          }
        },
        "rsiSource": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto stoch_rsi_method = series_method_cast<
   StochRsiMethod<ErasedSeriesMethod<BacktestMethodContext>,
                  ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(stoch_rsi_method, nullptr);

  const auto rsi_source =
   series_method_cast<DataMethod>(stoch_rsi_method->rsi_source());
  EXPECT_NE(rsi_source, nullptr);
  EXPECT_EQ(value_method_value(stoch_rsi_method->rsi_period()), 14);
  EXPECT_EQ(value_method_value(stoch_rsi_method->k_period()), 5);
  EXPECT_EQ(value_method_value(stoch_rsi_method->k_smooth()), 3);
  EXPECT_EQ(value_method_value(stoch_rsi_method->d_period()), 3);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerKcMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "KC",
      "params": {
        "maMethodType": "SMA",
        "period": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 5
          }
        },
        "maSource": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        },
        "bandMethodType": "ATR",
        "bandAtrPeriod": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 14
          }
        },
        "multiplier": {
          "method": "INPUT",
          "params": {
            "representation": "Decimal",
            "value": 1.0
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto kc_method =
   series_method_cast<KcMethod<ErasedSeriesMethod<BacktestMethodContext>,
                               ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
  ASSERT_NE(kc_method, nullptr);

  const auto ma_source_method =
   series_method_cast<DataMethod>(kc_method->source());
  EXPECT_NE(ma_source_method, nullptr);
  EXPECT_EQ(kc_method->ma_method_type(), MaMethodType::Sma);
  EXPECT_EQ(value_method_value(kc_method->period()), 5);
  EXPECT_EQ(kc_method->band_method_type(), KcBandMethodType::Atr);
  EXPECT_EQ(value_method_value(kc_method->band_atr_period()), 14);
  EXPECT_EQ(value_method_value(kc_method->multiplier()), 1.0);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerDonchianChannelMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "DC",
      "params": {
        "period": {
          "method": "INPUT",
          "params": {
            "representation": "UnsignedInteger",
            "value": 5
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto dc_method = series_method_cast<
   DonchianChannelMethod<ErasedSeriesMethod<BacktestMethodContext>>>(
   method);
  ASSERT_NE(dc_method, nullptr);

  EXPECT_EQ(value_method_value(dc_method->period()), 5);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerAddMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ADD",
      "params": {
        "augend": 50,
        "addend": {
          "method": "VALUE",
          "params": {
            "value": 25
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto add_method =
   series_method_cast<AddMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
  ASSERT_NE(add_method, nullptr);

  const auto augend = series_method_cast<ValueMethod>(add_method->left());
  const auto addend = series_method_cast<ValueMethod>(add_method->right());
  ASSERT_NE(augend, nullptr);
  ASSERT_NE(addend, nullptr);
  EXPECT_EQ(augend->value(), 50);
  EXPECT_EQ(addend->value(), 25);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerSubtractMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SUBTRACT",
      "params": {
        "minuend": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "subtrahend": {
          "method": "VALUE",
          "params": {
            "value": 30
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto subtract_method = series_method_cast<
   SubtractMethod<ErasedSeriesMethod<BacktestMethodContext>,
                  ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(subtract_method, nullptr);

  const auto minuend = series_method_cast<ValueMethod>(subtract_method->left());
  const auto subtrahend =
   series_method_cast<ValueMethod>(subtract_method->right());
  ASSERT_NE(minuend, nullptr);
  ASSERT_NE(subtrahend, nullptr);
  EXPECT_EQ(minuend->value(), 100);
  EXPECT_EQ(subtrahend->value(), 30);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerMultiplyMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "MULTIPLY",
      "params": {
        "multiplicand": {
          "method": "VALUE",
          "params": {
            "value": 10
          }
        },
        "multiplier": {
          "method": "VALUE",
          "params": {
            "value": 5
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto multiply_method = series_method_cast<
   MultiplyMethod<ErasedSeriesMethod<BacktestMethodContext>,
                  ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(multiply_method, nullptr);

  const auto multiplicand =
   series_method_cast<ValueMethod>(multiply_method->left());
  const auto multiplier =
   series_method_cast<ValueMethod>(multiply_method->right());
  ASSERT_NE(multiplicand, nullptr);
  ASSERT_NE(multiplier, nullptr);
  EXPECT_EQ(multiplicand->value(), 10);
  EXPECT_EQ(multiplier->value(), 5);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerDivideMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "DIVIDE",
      "params": {
        "dividend": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "divisor": {
          "method": "VALUE",
          "params": {
            "value": 2
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto divide_method = series_method_cast<
   DivideMethod<ErasedSeriesMethod<BacktestMethodContext>,
                ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(divide_method, nullptr);

  const auto dividend = series_method_cast<ValueMethod>(divide_method->left());
  const auto divisor = series_method_cast<ValueMethod>(divide_method->right());
  ASSERT_NE(dividend, nullptr);
  ASSERT_NE(divisor, nullptr);
  EXPECT_EQ(dividend->value(), 100);
  EXPECT_EQ(divisor->value(), 2);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerNegateMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "NEGATE",
      "params": {
        "operand": {
          "method": "VALUE",
          "params": {
            "value": 42
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto negate_method = series_method_cast<
   NegateMethod<ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(negate_method, nullptr);

  const auto operand =
   series_method_cast<ValueMethod>(negate_method->operand());
  ASSERT_NE(operand, nullptr);
  EXPECT_EQ(operand->value(), 42);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerSqrtMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "SQRT",
      "params": {
        "operand": {
          "method": "VALUE",
          "params": {
            "value": 16
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto sqrt_method = series_method_cast<
   SqrtMethod<ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(sqrt_method, nullptr);

  const auto operand = series_method_cast<ValueMethod>(sqrt_method->operand());
  ASSERT_NE(operand, nullptr);
  EXPECT_EQ(operand->value(), 16);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerChangeMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "CHANGE",
      "params": {
        "source": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto changes_method = series_method_cast<
   ChangeMethod<ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(changes_method, nullptr);

  const auto source = series_method_cast<DataMethod>(changes_method->source());
  ASSERT_NE(source, nullptr);
  EXPECT_EQ(source->field(), "close");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerAbsDiffMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ABS_DIFF",
      "params": {
        "minuend": {
          "method": "DATA",
          "params": {
            "field": "high"
          }
        },
        "subtrahend": {
          "method": "DATA",
          "params": {
            "field": "low"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto abs_diff_method = series_method_cast<
   AbsDiffMethod<ErasedSeriesMethod<BacktestMethodContext>,
                 ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(abs_diff_method, nullptr);

  const auto minuend = series_method_cast<DataMethod>(abs_diff_method->left());
  const auto subtrahend =
   series_method_cast<DataMethod>(abs_diff_method->right());
  ASSERT_NE(minuend, nullptr);
  ASSERT_NE(subtrahend, nullptr);
  EXPECT_EQ(minuend->field(), "high");
  EXPECT_EQ(subtrahend->field(), "low");

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseScreenerPercentageMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "PERCENTAGE",
      "params": {
        "base": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "percent": 20
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto percentage_method = series_method_cast<
   PercentageMethod<ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(percentage_method, nullptr);

  const auto base = series_method_cast<ValueMethod>(percentage_method->base());
  ASSERT_NE(base, nullptr);
  EXPECT_EQ(base->value(), 100);

  const auto percent = percentage_method->percent();
  EXPECT_EQ(percent, 20);

  const auto serialized_config = serialize_node_method(method);
  EXPECT_TRUE(serialized_config.at("params").contains("percent"));
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseStopTargetPriceMethods)
{
  const auto configs = std::vector<json>{
   json::parse(
    R"({"method":"SL_AMOUNT","params":{"amount":{"method":"VALUE","params":{"value":10}}}})"),
   json::parse(
    R"({"method":"TP_AMOUNT","params":{"amount":{"method":"VALUE","params":{"value":20}}}})"),
   json::parse(
    R"({"method":"SL_PERCENT","params":{"percent":{"method":"VALUE","params":{"value":10}}}})"),
   json::parse(
    R"({"method":"TP_PERCENT","params":{"percent":{"method":"VALUE","params":{"value":20}}}})"),
   json::parse(
    R"({"method":"SL_ATR","params":{"period":{"method":"VALUE","params":{"value":14}},"multiplier":{"method":"VALUE","params":{"value":2}},"maSmoothingType":"RMA"}})"),
   json::parse(
    R"({"method":"TP_ATR","params":{"period":{"method":"VALUE","params":{"value":14}},"multiplier":{"method":"VALUE","params":{"value":2}},"maSmoothingType":"RMA"}})"),
   json::parse(
    R"({"method":"SL_R_MULTIPLE","params":{"multiple":{"method":"VALUE","params":{"value":1}}}})"),
   json::parse(
    R"({"method":"TP_R_MULTIPLE","params":{"multiple":{"method":"VALUE","params":{"value":2}}}})")};

  for(const auto& config : configs) {
    const auto method = parse_node_method(config);
    const auto serialized_config = serialize_node_method(method);
    const auto deserialized_config = parse_node_method(serialized_config);
    EXPECT_EQ(method, deserialized_config);
  }
}

TEST_F(ConfigParserTest, ParsePositionContextValueMethods)
{
  const auto configs =
   std::vector<json>{json::parse(R"({"method":"INITIAL_ENTRY_PRICE"})"),
                     json::parse(R"({"method":"LATEST_ENTRY_PRICE"})"),
                     json::parse(R"({"method":"AVERAGE_PRICE"})"),
                     json::parse(R"({"method":"STOP_TARGET_REF_PRICE"})"),
                     json::parse(R"({"method":"POSITION_DIRECTION"})"),
                     json::parse(R"({"method":"PYRAMIDING_LAYER"})")};

  for(const auto& config : configs) {
    const auto method = parse_node_method(config);
    const auto serialized_config = serialize_node_method(method);
    const auto deserialized_config = parse_node_method(serialized_config);
    EXPECT_EQ(method, deserialized_config);
  }
}

TEST_F(ConfigParserTest, ParsePositionRMultipleMethod)
{
  const auto default_method =
   parse_node_method(json::parse(R"("POSITION_R_MULTIPLE")"));
  const auto custom_method = parse_node_method(json::parse(
   R"({"method":"POSITION_R_MULTIPLE","params":{"source":"OPEN"}})"));

  const auto* default_r_multiple =
   series_method_cast<pludux::backtest::PositionRMultipleMethod<
    ErasedSeriesMethod<BacktestMethodContext>>>(default_method);
  ASSERT_NE(default_r_multiple, nullptr);
  EXPECT_NE(series_method_cast<CloseMethod>(default_r_multiple->source()),
            nullptr);

  const auto* custom_r_multiple =
   series_method_cast<pludux::backtest::PositionRMultipleMethod<
    ErasedSeriesMethod<BacktestMethodContext>>>(custom_method);
  ASSERT_NE(custom_r_multiple, nullptr);
  EXPECT_NE(series_method_cast<OpenMethod>(custom_r_multiple->source()),
            nullptr);

  for(const auto& method : {default_method, custom_method}) {
    const auto serialized_config = serialize_node_method(method);
    EXPECT_EQ(serialized_config.at("method"), "POSITION_R_MULTIPLE");
    EXPECT_TRUE(serialized_config.at("params").contains("source"));
    EXPECT_EQ(method, parse_node_method(serialized_config));
  }
}

TEST_F(ConfigParserTest, ParseScreenerInvalidMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "INVALID"
    }
  )");

  EXPECT_THROW(parse_node_method(config), std::invalid_argument);
}

TEST_F(ConfigParserTest, ParseScreenerAllOfMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ALL_OF",
      "params": {
        "items": [
          {
            "method": "GREATER_THAN",
            "params": {
              "threshold": {
                "method": "VALUE",
                "params": {
                  "value": 100
                }
              },
              "target": {
                "method": "DATA",
                "params": {
                  "field": "close"
                }
              }
            }
          },
          {
            "method": "LESS_THAN",
            "params": {
              "threshold": {
                "method": "VALUE",
                "params": {
                  "value": 200
                }
              },
              "target": {
                "method": "DATA",
                "params": {
                  "field": "close"
                }
              }
            }
          }
        ]
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);
  ASSERT_EQ(config.at("params").at("items").size(), 2);
  const auto parsed_all_of = node_cast<SignalAllOfNode>(filter);
  ASSERT_NE(parsed_all_of, nullptr);
  ASSERT_EQ(parsed_all_of->conditions().size(), 2);

  const auto serialized_config = config_parser.serialize_node(filter);

  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerAnyOfMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ANY_OF",
      "params": {
        "items": [
          {
            "method": "GREATER_THAN",
            "params": {
              "threshold": {
                "method": "VALUE",
                "params": {
                  "value": 100
                }
              },
              "target": {
                "method": "DATA",
                "params": {
                  "field": "close"
                }
              }
            }
          },
          {
            "method": "LESS_THAN",
            "params": {
              "threshold": {
                "method": "VALUE",
                "params": {
                  "value": 200
                }
              },
              "target": {
                "method": "DATA",
                "params": {
                  "field": "close"
                }
              }
            }
          }
        ]
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);
  ASSERT_EQ(config.at("params").at("items").size(), 2);
  const auto parsed_any_of = node_cast<SignalAnyOfNode>(filter);
  ASSERT_NE(parsed_any_of, nullptr);
  ASSERT_EQ(parsed_any_of->conditions().size(), 2);

  const auto serialized_config = config_parser.serialize_node(filter);

  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerGreaterThanMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "GREATER_THAN",
      "params": {
        "threshold": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto greater_than_filter = node_cast<SignalGreaterThanNode>(filter);
  ASSERT_NE(greater_than_filter, nullptr);

  const auto target = node_cast<DataNode>(greater_than_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto threshold = node_cast<ValueNode>(greater_than_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerGreaterEqualMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "GREATER_EQUAL",
      "params": {
        "threshold": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto greater_equal_filter = node_cast<SignalGreaterEqualNode>(filter);
  ASSERT_NE(greater_equal_filter, nullptr);

  const auto target = node_cast<DataNode>(greater_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerLessThanMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "LESS_THAN",
      "params": {
        "threshold": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto less_than_filter = node_cast<SignalLessThanNode>(filter);
  ASSERT_NE(less_than_filter, nullptr);

  const auto target = node_cast<DataNode>(less_than_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerLessEqualMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "LESS_EQUAL",
      "params": {
        "threshold": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto less_equal_filter = node_cast<SignalLessEqualNode>(filter);
  ASSERT_NE(less_equal_filter, nullptr);

  const auto target = node_cast<DataNode>(less_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerEqualMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "EQUAL",
      "params": {
        "threshold": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto equal_filter = node_cast<SignalEqualNode>(filter);
  ASSERT_NE(equal_filter, nullptr);

  const auto target = node_cast<DataNode>(equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto threshold = node_cast<ValueNode>(equal_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerNotEqualMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "NOT_EQUAL",
      "params": {
        "threshold": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto not_equal_filter = node_cast<SignalNotEqualNode>(filter);
  ASSERT_NE(not_equal_filter, nullptr);

  const auto target = node_cast<DataNode>(not_equal_filter->target());
  ASSERT_NE(target, nullptr);

  EXPECT_EQ(target->field(), "close");

  const auto threshold = node_cast<ValueNode>(not_equal_filter->threshold());
  ASSERT_NE(threshold, nullptr);

  EXPECT_EQ(threshold->value(), 100);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerCrossunderMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "CROSSUNDER",
      "params": {
        "value": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        },
        "baseline": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto crossunder_filter = node_cast<SignalCrossunderNode>(filter);
  ASSERT_NE(crossunder_filter, nullptr);

  const auto signal = node_cast<DataNode>(crossunder_filter->source());
  ASSERT_NE(signal, nullptr);

  EXPECT_EQ(signal->field(), "close");

  const auto reference = node_cast<ValueNode>(crossunder_filter->reference());
  ASSERT_NE(reference, nullptr);

  EXPECT_EQ(reference->value(), 100);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerCrossoverMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "CROSSOVER",
      "params": {
        "value": {
          "method": "DATA",
          "params": {
            "field": "close"
          }
        },
        "baseline": {
          "method": "VALUE",
          "params": {
            "value": 100
          }
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto crossover_filter = node_cast<SignalCrossoverNode>(filter);
  ASSERT_NE(crossover_filter, nullptr);

  const auto signal = node_cast<DataNode>(crossover_filter->source());
  ASSERT_NE(signal, nullptr);

  EXPECT_EQ(signal->field(), "close");

  const auto reference = node_cast<ValueNode>(crossover_filter->reference());
  ASSERT_NE(reference, nullptr);

  EXPECT_EQ(reference->value(), 100);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerAlwaysMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "ALWAYS"
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto always_filter = node_cast<SignalAlwaysNode>(filter);
  ASSERT_NE(always_filter, nullptr);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerNeverMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "NEVER"
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto never_filter = node_cast<SignalNeverNode>(filter);
  ASSERT_NE(never_filter, nullptr);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerAndMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "AND",
      "params": {
        "firstCondition": {
          "method": "ALWAYS"
        },
        "secondCondition": {
          "method": "NEVER"
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto and_filter = node_cast<SignalAndNode>(filter);
  ASSERT_NE(and_filter, nullptr);

  const auto first_condition = and_filter->first_condition();
  const auto second_condition = and_filter->second_condition();

  const auto always_filter = node_cast<SignalAlwaysNode>(first_condition);
  const auto never_filter = node_cast<SignalNeverNode>(second_condition);

  ASSERT_NE(always_filter, nullptr);
  ASSERT_NE(never_filter, nullptr);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerOrMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "OR",
      "params": {
        "firstCondition": {
          "method": "ALWAYS"
        },
        "secondCondition": {
          "method": "NEVER"
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto or_filter = node_cast<SignalOrNode>(filter);
  ASSERT_NE(or_filter, nullptr);

  const auto first_condition = or_filter->first_condition();
  const auto second_condition = or_filter->second_condition();

  const auto always_filter = node_cast<SignalAlwaysNode>(first_condition);
  const auto never_filter = node_cast<SignalNeverNode>(second_condition);

  ASSERT_NE(always_filter, nullptr);
  ASSERT_NE(never_filter, nullptr);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerNotMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "NOT",
      "params": {
        "condition": true
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto not_filter = node_cast<SignalNotNode>(filter);
  ASSERT_NE(not_filter, nullptr);

  const auto other_condition = not_filter->other_condition();
  const auto always_filter = node_cast<SignalAlwaysNode>(other_condition);

  ASSERT_NE(always_filter, nullptr);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseScreenerXorMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "XOR",
      "params": {
        "firstCondition": {
          "method": "ALWAYS"
        },
        "secondCondition": {
          "method": "NEVER"
        }
      }
    }
  )");

  const auto filter = config_parser.parse_node(config);

  const auto xor_filter = node_cast<SignalXorNode>(filter);
  ASSERT_NE(xor_filter, nullptr);

  const auto first_condition = xor_filter->first_condition();
  const auto second_condition = xor_filter->second_condition();

  const auto always_filter = node_cast<SignalAlwaysNode>(first_condition);
  const auto never_filter = node_cast<SignalNeverNode>(second_condition);

  ASSERT_NE(always_filter, nullptr);
  ASSERT_NE(never_filter, nullptr);

  const auto serialized_config = config_parser.serialize_node(filter);
  const auto deserialized_filter = config_parser.parse_node(serialized_config);
  EXPECT_EQ(filter, deserialized_filter);
}

TEST_F(ConfigParserTest, ParseHighestMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "HIGHEST",
      "params": {
        "period": {
          "method": "VALUE",
          "params": {
            "value": 14
          }
        },
        "source": {
          "method": "CLOSE"
        }
      }
    }
  )");

  const auto method = parse_node_method(config);
  const auto highest_method = series_method_cast<
   HighestMethod<ErasedSeriesMethod<BacktestMethodContext>,
                 ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(highest_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseLowestMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "LOWEST",
      "params": {
        "period": {
          "method": "VALUE",
          "params": {
            "value": 14
          }
        },
        "source": {
          "method": "CLOSE"
        }
      }
    }
  )");

  const auto method = parse_node_method(config);
  const auto lowest_method = series_method_cast<
   LowestMethod<ErasedSeriesMethod<BacktestMethodContext>,
                ErasedSeriesMethod<BacktestMethodContext>>>(method);
  ASSERT_NE(lowest_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseTrMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "TR"
    }
  )");

  const auto method = parse_node_method(config);
  const auto tr_method = series_method_cast<TrMethod>(method);
  ASSERT_NE(tr_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseSignalMethodsAsSeriesRoundTrip)
{
  const auto configs = std::vector<json>{
   json::parse(R"({"method":"ALWAYS"})"),
   json::parse(R"({"method":"NEVER"})"),
   json::parse(R"({"method":"NOT","params":{"condition":true}})"),
   json::parse(
    R"({"method":"AND","params":{"firstCondition":true,"secondCondition":false}})"),
   json::parse(
    R"({"method":"OR","params":{"firstCondition":false,"secondCondition":true}})"),
   json::parse(
    R"({"method":"XOR","params":{"firstCondition":true,"secondCondition":false}})"),
   json::parse(R"({"method":"ALL_OF","params":{"items":[true,false,true]}})"),
   json::parse(R"({"method":"ANY_OF","params":{"items":[false,false,true]}})"),
   json::parse(
    R"({"method":"CROSSOVER","params":{"value":{"method":"VALUE","params":{"value":2}},"baseline":{"method":"VALUE","params":{"value":1}}}})"),
   json::parse(
    R"({"method":"CROSSUNDER","params":{"value":{"method":"VALUE","params":{"value":1}},"baseline":{"method":"VALUE","params":{"value":2}}}})"),
   json::parse(
    R"({"method":"GREATER_THAN","params":{"target":{"method":"VALUE","params":{"value":2}},"threshold":{"method":"VALUE","params":{"value":1}}}})"),
   json::parse(
    R"({"method":"GREATER_EQUAL","params":{"target":{"method":"VALUE","params":{"value":2}},"threshold":{"method":"VALUE","params":{"value":2}}}})"),
   json::parse(
    R"({"method":"LESS_THAN","params":{"target":{"method":"VALUE","params":{"value":1}},"threshold":{"method":"VALUE","params":{"value":2}}}})"),
   json::parse(
    R"({"method":"LESS_EQUAL","params":{"target":{"method":"VALUE","params":{"value":1}},"threshold":{"method":"VALUE","params":{"value":1}}}})"),
   json::parse(
    R"({"method":"EQUAL","params":{"target":{"method":"VALUE","params":{"value":1}},"threshold":{"method":"VALUE","params":{"value":1}}}})"),
   json::parse(
    R"({"method":"NOT_EQUAL","params":{"target":{"method":"VALUE","params":{"value":1}},"threshold":{"method":"VALUE","params":{"value":2}}}})")};

  for(const auto& config : configs) {
    const auto method = parse_node_method(config);
    const auto serialized_config = serialize_node_method(method);
    const auto deserialized_config = parse_node_method(serialized_config);
    EXPECT_EQ(method, deserialized_config);
  }
}

TEST_F(ConfigParserTest, ParseAnyConditionMethodIsInvalid)
{
  const auto config = json::parse(R"(
    {
      "method": "invalid not existed filter should throw exception"
    }
  )");

  EXPECT_THROW(config_parser.parse_node(config), std::exception);
}

TEST_F(ConfigParserTest, ParseAnyConditionMethodWithInvalidRequiredFields)
{
  const auto config = json::parse(R"(
    {
      "method": "AND"
    }
  )");

  EXPECT_THROW(config_parser.parse_node(config), std::exception);
}

TEST_F(ConfigParserTest, SeriesNodeRegistrySerializationDeserialization)
{
  const auto config = json::parse(R"(
    {
      "name1": {
        "method": "DATA",
        "params": {
          "field": "close"
        }
      },
      "name2": {
        "method": "VALUE",
        "params": {
          "value": 100
        }
      }
    }
  )");

  auto series_nodes =
   OrderedNamedRegistry<ErasedNode<BacktestMethodContext>>{};
  series_nodes.set("name1", DataNode{"close"});
  series_nodes.set("name2", ValueNode{100});

  auto serialize_series_nodes =
   [this](const OrderedNamedRegistry<ErasedNode<BacktestMethodContext>>&
           series_nodes) {
     auto series_nodes_config = jsoncons::ojson{};
     for(const auto& [series_name, series_node] : series_nodes) {
       series_nodes_config[series_name] =
        config_parser.serialize_node(series_node);
     }
     return series_nodes_config;
   };
  auto parse_series_nodes = [this](const jsoncons::ojson& series_nodes_config) {
    auto parsed_series_nodes =
     OrderedNamedRegistry<ErasedNode<BacktestMethodContext>>{};
    for(const auto& [series_name, series_config] :
        series_nodes_config.object_range()) {
      parsed_series_nodes.set(series_name,
                              config_parser.parse_node(series_config));
    }
    return parsed_series_nodes;
  };

  const auto serialized_config = serialize_series_nodes(series_nodes);

  const auto deserialized_series_nodes = parse_series_nodes(serialized_config);
  const auto deserialized_config = parse_series_nodes(config);
  EXPECT_EQ(deserialized_config, deserialized_series_nodes);
  EXPECT_EQ(series_nodes, deserialized_series_nodes);
}

TEST_F(ConfigParserTest, EntryFilterRoundTripsResultAndAccountNodes)
{
  const auto config = json::parse(R"(
    {
      "method": "AND",
      "firstCondition": {
        "method": "GREATER_EQUAL",
        "target": {
          "method": "MODEL_PERFORMANCE",
          "params": {
            "metric": "LIFETIME_COUNT"
          }
        },
        "threshold": {
          "method": "VALUE",
          "value": 25
        }
      },
      "secondCondition": {
        "method": "LESS_EQUAL",
        "target": {
          "method": "REQUESTED_RISK_WITH_FEES"
        },
        "threshold": {
          "method": "EQUITY"
        }
      }
    }
  )");

  const auto node = backtest::parse_entry_filter_node(config);
  const auto serialized = backtest::serialize_entry_filter_node(node);
  EXPECT_EQ(serialized.at("firstCondition").at("target").at("method"),
            "MODEL_PERFORMANCE");
  EXPECT_EQ(serialized.at("firstCondition").at("target").at("params").at(
              "metric").as<std::string>(),
            "LIFETIME_COUNT");
  EXPECT_EQ(node, backtest::parse_entry_filter_node(serialized));
}

TEST_F(ConfigParserTest, EntryFilterRejectsMarketDataNodes)
{
  const auto config = json::parse(R"(
    {
      "method": "GREATER_THAN",
      "target": {
        "method": "CLOSE"
      },
      "threshold": {
        "method": "VALUE",
        "value": 100
      }
    }
  )");

  EXPECT_THROW(backtest::parse_entry_filter_node(config),
               std::invalid_argument);
}

TEST_F(ConfigParserTest, EntryFilterRejectsNonBooleanRoot)
{
  const auto config =
   json::parse(
    R"({"method":"MODEL_PERFORMANCE","params":{"metric":"WIN_RATE"}})");

  EXPECT_THROW(backtest::parse_entry_filter_node(config),
               std::invalid_argument);
}

TEST_F(ConfigParserTest, EntryFilterRejectsLegacyStrategyPerformanceMethod)
{
  const auto config = json::parse(R"(
    {"method":"GREATER_THAN","target":{"method":"STRATEGY_PERFORMANCE","params":{"metric":"WIN_RATE"}},"threshold":0}
  )");

  EXPECT_THROW(backtest::parse_entry_filter_node(config), std::invalid_argument);
}

TEST_F(ConfigParserTest, EntryFilterRejectsIntegerPerformanceMetric)
{
  const auto config = json::parse(R"(
    {
      "method": "GREATER_THAN",
      "target": {
        "method": "MODEL_PERFORMANCE",
        "params": {
          "metric": 0
        }
      },
      "threshold": 0
    }
  )");

  EXPECT_THROW(backtest::parse_entry_filter_node(config),
               std::invalid_argument);
}

TEST_F(ConfigParserTest, EntryFilterRejectsPerformanceMetricWithoutParams)
{
  const auto config = json::parse(R"(
    {
      "method": "GREATER_THAN",
      "target": {
        "method": "MODEL_PERFORMANCE"
      },
      "threshold": 0
    }
  )");

  EXPECT_THROW(backtest::parse_entry_filter_node(config),
               std::invalid_argument);
}

TEST_F(ConfigParserTest, EntryFilterRejectsPerformanceMetricWithoutMetric)
{
  const auto config = json::parse(R"(
    {
      "method": "GREATER_THAN",
      "target": {
        "method": "MODEL_PERFORMANCE",
        "params": {}
      },
      "threshold": 0
    }
  )");

  EXPECT_THROW(backtest::parse_entry_filter_node(config),
               std::invalid_argument);
}

TEST_F(ConfigParserTest, EntryFilterRejectsUnknownPerformanceMetric)
{
  const auto config = json::parse(R"(
    {
      "method": "GREATER_THAN",
      "target": {
        "method": "MODEL_PERFORMANCE",
        "params": {
          "metric": "UNKNOWN"
        }
      },
      "threshold": 0
    }
  )");

  EXPECT_THROW(backtest::parse_entry_filter_node(config),
               std::invalid_argument);
}

TEST_F(ConfigParserTest, EntryFilterRejectsLowercasePerformanceMetric)
{
  const auto config = json::parse(R"(
    {
      "method": "GREATER_THAN",
      "target": {
        "method": "MODEL_PERFORMANCE",
        "params": {
          "metric": "win_rate"
        }
      },
      "threshold": 0
    }
  )");

  EXPECT_THROW(backtest::parse_entry_filter_node(config),
               std::invalid_argument);
}
