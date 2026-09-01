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
using SignalGreaterThanNode = pludux::GreaterThanNode<BacktestMethodContext>;
using SignalGreaterEqualNode = pludux::GreaterEqualNode<BacktestMethodContext>;
using SignalLessThanNode = pludux::LessThanNode<BacktestMethodContext>;
using SignalLessEqualNode = pludux::LessEqualNode<BacktestMethodContext>;
using SignalCrossoverNode = pludux::CrossoverNode<BacktestMethodContext>;
using SignalCrossunderNode = pludux::CrossunderNode<BacktestMethodContext>;
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
   "VALUE.CONSTANT",
   [](const Parser&, const Node& node) -> json {
     const auto* value = node_cast<ValueNode>(node);
     return value ? json::object{{"value", value->value()}} : json::null();
   },
   [](Parser::Parser, const json& params) -> Node {
     return ValueNode{params.at("value").as_double()};
   });
  parser.register_node_parser(
   "OPERATOR.ADD",
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
    {"method":"OPERATOR.ADD","params":{"augend":{"method":"VALUE.CONSTANT","params":{"value":2}},"addend":{"method":"VALUE.CONSTANT","params":{"value":3}}}}
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

TEST(ConfigParserInfrastructureTest, RejectsDuplicateNodeParserRegistration)
{
  using Parser = ConfigParser<BacktestMethodContext>;
  using Node = ErasedNode<BacktestMethodContext>;
  auto parser = Parser{};
  const auto serialize = [](const Parser&, const Node&) -> json {
    return json::null();
  };
  const auto deserialize = [](Parser::Parser, const json&) -> Node {
    return ValueNode{0.0};
  };

  parser.register_node_parser("TEST.VALUE", serialize, deserialize);
  EXPECT_THROW(
   parser.register_node_parser("TEST.VALUE", serialize, deserialize),
   std::invalid_argument);
}

TEST(ConfigParserInfrastructureTest, RejectsMultipleMatchingNodeSerializers)
{
  using Parser = ConfigParser<BacktestMethodContext>;
  using Node = ErasedNode<BacktestMethodContext>;
  auto parser = Parser{};
  const auto serialize = [](const Parser&, const Node& node) -> json {
    return node_cast<ValueNode>(node) ? json::object() : json::null();
  };
  const auto deserialize = [](Parser::Parser, const json&) -> Node {
    return ValueNode{0.0};
  };

  parser.register_node_parser("TEST.VALUE_A", serialize, deserialize);
  parser.register_node_parser("TEST.VALUE_B", serialize, deserialize);
  EXPECT_THROW(parser.serialize_node(Node{ValueNode{1.0}}),
               std::invalid_argument);
}

TEST(ConfigParserInfrastructureTest, RejectsLegacyFlatMethodIdentifiers)
{
  auto parser = make_backtest_model_config_parser();
  for(const auto* legacy : {"SMA", "ADD", "CLOSE", "GREATER_THAN"}) {
    SCOPED_TRACE(legacy);
    EXPECT_THROW(parser.parse_node(json::object{{"method", legacy}}),
                 std::invalid_argument);
  }

  auto comparator = make_requested_order_comparator_config_parser();
  EXPECT_THROW(
   comparator.parse_node(json::object{{"method", "REQUESTED_ORDER_PRICE"}}),
   std::invalid_argument);

  EXPECT_THROW(
   backtest::parse_model_config_json(
    "Legacy plot",
    json::parse(
     R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"plots":[{"items":[{"method":"LINE"}]}]})")),
   std::invalid_argument);
  EXPECT_THROW(
   backtest::parse_model_config_json(
    "Legacy plot source",
    json::parse(
     R"({"version":1,"execution":{"intrabarPath":"CANDLE_DIRECTION"},"plots":[{"items":[{"method":"PLOT.LINE","params":{"source":{"method":"SERIES"}}}]}]})")),
   std::invalid_argument);
}

TEST(ModelConfigParserTest, ParsesShorthandsDefaultsAndTypedNodes)
{
  auto parser = make_backtest_model_config_parser();
  const auto config = json::parse(R"(
    {"method":"INDICATOR.SMA","params":{"source":{"method":"OPERATOR.ADD","params":{"augend":"MARKET_DATA.CLOSE","addend":2}}}}
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
  for(const auto* comparator_only :
      {"REQUESTED_ORDER.PRICE",
       "REQUESTED_ORDER.DIRECTION",
       "REQUESTED_ORDER.IS_PYRAMIDING",
       "REQUESTED_ORDER.RAW_QUANTITY",
       "REQUESTED_ORDER.RAW_QUANTITY_LIMIT",
       "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY",
       "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY_LIMIT",
       "REQUESTED_ORDER.QUANTITY",
       "REQUESTED_ORDER.NOTIONAL",
       "REQUESTED_ORDER.COST",
       "REQUESTED_ORDER.ESTIMATED_ENTRY_FEE",
       "REQUESTED_ORDER.ESTIMATED_1R_EXIT_FEE",
       "REQUESTED_ORDER.RISK_DISTANCE",
       "REQUESTED_ORDER.PRICE_RISK",
       "REQUESTED_ORDER.RISK_WITH_FEES",
       "REQUESTED_ORDER.FROZEN_UNIT_QUANTITY"}) {
    SCOPED_TRACE(comparator_only);
    EXPECT_THROW(parser.parse_node(json::parse(std::string{"{\"method\":\""} +
                                               comparator_only + "\"}")),
                 std::invalid_argument);
  }
  EXPECT_THROW(
   parser.parse_node(json::parse(
    R"({"method":"MODEL_PERFORMANCE.VALUE","params":{"metric":"WIN_RATE"}})")),
   std::invalid_argument);
}

TEST(ModelConfigParserTest, PreservesCanonicalCompositeDefaults)
{
  auto parser = make_backtest_model_config_parser();
  const auto config = json::parse(R"({"method":"INDICATOR.MACD"})");

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
   "0",
   "true",
   "false",
   "\"MARKET_DATA.OPEN\"",
   "\"MARKET_DATA.HIGH\"",
   "\"MARKET_DATA.LOW\"",
   "\"MARKET_DATA.CLOSE\"",
   "\"MARKET_DATA.VOLUME\"",
   "\"PORTFOLIO.EQUITY\"",
   "\"PORTFOLIO.EQUITY_PERCENT\"",
   "\"PORTFOLIO.DRAWDOWN\"",
   "\"POSITION.PYRAMIDING_LAYER\"",
   "\"POSITION.INITIAL_ENTRY_PRICE\"",
   "\"POSITION.LATEST_ENTRY_PRICE\"",
   "\"POSITION.AVERAGE_PRICE\"",
   "\"POSITION.STOP_TARGET_REF_PRICE\"",
   "\"POSITION.DIRECTION\"",
   "\"STOP_LOSS.ONE_R\"",
   "\"LOGIC.ALWAYS\"",
   "\"LOGIC.NEVER\"",
   R"({"method":"VALUE.CONSTANT","params":{"value":1.25}})",
   R"({"method":"MARKET_DATA.FIELD","params":{"field":"custom"}})",
   R"({"method":"SERIES.REFERENCE","params":{"name":"basis"}})",
   R"({"method":"INPUT.NUMERIC","params":{"label":"Period","representation":"SignedInteger","value":-2}})",
   R"({"method":"OPERATOR.CHANGE"})",
   R"({"method":"OPERATOR.LOOKBACK","params":{"period":2}})",
   R"({"method":"INDICATOR.SMA"})",
   R"({"method":"INDICATOR.EMA"})",
   R"({"method":"INDICATOR.WMA"})",
   R"({"method":"INDICATOR.RMA"})",
   R"({"method":"INDICATOR.HMA"})",
   R"({"method":"INDICATOR.RSI"})",
   R"({"method":"INDICATOR.HIGHEST"})",
   R"({"method":"INDICATOR.LOWEST"})",
   R"({"method":"INDICATOR.ROC"})",
   R"({"method":"INDICATOR.STDDEV"})",
   R"({"method":"INDICATOR.RVOL"})",
   R"({"method":"INDICATOR.TR"})",
   R"({"method":"INDICATOR.ATR","params":{"maSmoothingType":"EMA"}})",
   R"({"method":"INDICATOR.KC"})",
   R"({"method":"INDICATOR.DC"})",
   R"({"method":"INDICATOR.BB"})",
   R"({"method":"INDICATOR.MACD"})",
   R"({"method":"INDICATOR.STOCH"})",
   R"({"method":"INDICATOR.STOCH_RSI"})",
   R"({"method":"OPERATOR.SELECT_OUTPUT","params":{"output":"upper-band","source":"MARKET_DATA.CLOSE"}})",
   R"({"method":"OPERATOR.PERCENTAGE"})",
   R"({"method":"POSITION.R_MULTIPLE"})",
   R"({"method":"STOP_LOSS.AMOUNT"})",
   R"({"method":"TAKE_PROFIT.AMOUNT"})",
   R"({"method":"STOP_LOSS.PERCENT"})",
   R"({"method":"TAKE_PROFIT.PERCENT"})",
   R"({"method":"STOP_LOSS.ATR"})",
   R"({"method":"TAKE_PROFIT.ATR"})",
   R"({"method":"RISK_DISTANCE.AMOUNT"})",
   R"({"method":"RISK_DISTANCE.PERCENT"})",
   R"({"method":"RISK_DISTANCE.ATR"})",
   R"({"method":"STOP_LOSS.R_MULTIPLE"})",
   R"({"method":"TAKE_PROFIT.R_MULTIPLE"})",
   R"({"method":"OPERATOR.ADD","params":{"augend":1,"addend":2}})",
   R"({"method":"OPERATOR.SUBTRACT","params":{"minuend":1,"subtrahend":2}})",
   R"({"method":"OPERATOR.MULTIPLY","params":{"multiplicand":1,"multiplier":2}})",
   R"({"method":"OPERATOR.DIVIDE","params":{"dividend":1,"divisor":2}})",
   R"({"method":"OPERATOR.ABS_DIFF","params":{"minuend":1,"subtrahend":2}})",
   R"({"method":"OPERATOR.MAX","params":{"left":1,"right":2}})",
   R"({"method":"OPERATOR.MIN","params":{"left":1,"right":2}})",
   R"({"method":"OPERATOR.NEGATE","params":{"operand":1}})",
   R"({"method":"OPERATOR.ABS","params":{"operand":-1}})",
   R"({"method":"OPERATOR.SQRT","params":{"operand":4}})",
   R"({"method":"OPERATOR.POSITIVE_PART","params":{"operand":1}})",
   R"({"method":"OPERATOR.NEGATIVE_PART","params":{"operand":-1}})",
   R"({"method":"COMPARISON.GREATER_THAN","params":{"target":2,"threshold":1}})",
   R"({"method":"COMPARISON.GREATER_EQUAL","params":{"target":2,"threshold":1}})",
   R"({"method":"COMPARISON.LESS_THAN","params":{"target":1,"threshold":2}})",
   R"({"method":"COMPARISON.LESS_EQUAL","params":{"target":1,"threshold":2}})",
   R"({"method":"COMPARISON.EQUAL","params":{"target":1,"threshold":1}})",
   R"({"method":"COMPARISON.NOT_EQUAL","params":{"target":1,"threshold":2}})",
   R"({"method":"COMPARISON.CROSSOVER","params":{"value":"MARKET_DATA.CLOSE","baseline":"MARKET_DATA.OPEN"}})",
   R"({"method":"COMPARISON.CROSSUNDER","params":{"value":"MARKET_DATA.CLOSE","baseline":"MARKET_DATA.OPEN"}})",
   R"({"method":"LOGIC.AND","params":{"firstCondition":true,"secondCondition":false}})",
   R"({"method":"LOGIC.OR","params":{"firstCondition":true,"secondCondition":false}})",
   R"({"method":"LOGIC.XOR","params":{"firstCondition":true,"secondCondition":false}})",
   R"({"method":"LOGIC.NOT","params":{"condition":true}})",
   R"({"method":"LOGIC.ALL_OF","params":{"items":[true,false]}})",
   R"({"method":"LOGIC.ANY_OF","params":{"items":[true,false]}})"};

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
   json::parse(R"({"method":"OPERATOR.ABS","params":{"operand":-1}})"));
  EXPECT_NE(
   (node_cast<UnaryOperatorNode<Absolute<>, BacktestMethodContext>>(absolute)),
   nullptr);
  EXPECT_EQ(parser.parse_node(parser.serialize_node(absolute)), absolute);

  const auto maximum = parser.parse_node(
   json::parse(R"({"method":"OPERATOR.MAX","params":{"left":1,"right":2}})"));
  EXPECT_NE(
   (node_cast<BinaryOperatorNode<Maximum<>, BacktestMethodContext>>(maximum)),
   nullptr);
  EXPECT_EQ(parser.parse_node(parser.serialize_node(maximum)), maximum);

  const auto minimum = parser.parse_node(
   json::parse(R"({"method":"OPERATOR.MIN","params":{"left":1,"right":2}})"));
  EXPECT_NE(
   (node_cast<BinaryOperatorNode<Minimum<>, BacktestMethodContext>>(minimum)),
   nullptr);
  EXPECT_EQ(parser.parse_node(parser.serialize_node(minimum)), minimum);

  const auto positive_part = parser.parse_node(json::parse(
   R"({"method":"OPERATOR.POSITIVE_PART","params":{"operand":1}})"));
  EXPECT_NE(
   (node_cast<UnaryOperatorNode<PositivePart<>, BacktestMethodContext>>(
    positive_part)),
   nullptr);
  EXPECT_EQ(parser.parse_node(parser.serialize_node(positive_part)),
            positive_part);

  const auto negative_part = parser.parse_node(json::parse(
   R"({"method":"OPERATOR.NEGATIVE_PART","params":{"operand":-1}})"));
  EXPECT_NE(
   (node_cast<UnaryOperatorNode<NegativePart<>, BacktestMethodContext>>(
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
    {"method":"OPERATOR.DIVIDE","params":{"dividend":{"method":"REQUESTED_ORDER.NOTIONAL"},"divisor":{"method":"OPERATOR.LOOKBACK","params":{"period":2,"source":"MARKET_DATA.CLOSE"}}}}
  )");
  const auto node = parser.parse_node(config);
  const auto* divide = node_cast<
   BinaryOperatorNode<std::divides<>, backtest::RequestedOrderMethodContext>>(
   node);
  EXPECT_NE(divide, nullptr);
  const auto serialized = parser.serialize_node(node);
  EXPECT_EQ(parser.parse_node(serialized), node);
  EXPECT_THROW(parser.parse_node(json::parse(
                R"({"method":"SERIES.REFERENCE","params":{"name":"x"}})")),
               std::invalid_argument);
  EXPECT_THROW(parser.parse_node(json::parse(R"({"method":"INDICATOR.ATR"})")),
               std::invalid_argument);
  EXPECT_THROW(
   parser.parse_node(json::parse(R"({"method":"PORTFOLIO.EQUITY"})")),
   std::invalid_argument);
  EXPECT_THROW(parser.parse_node(json::parse("true")), std::invalid_argument);
  EXPECT_THROW(parser.parse_node(json::parse("false")), std::invalid_argument);
}

TEST(RequestedOrderComparatorConfigParserTest,
     RoundTripsEveryCanonicalComparatorRegistrationAndRejectsModelOnlyNodes)
{
  auto parser = make_requested_order_comparator_config_parser();
  const auto configurations = std::vector<std::string>{
   "1",
   "\"MARKET_DATA.OPEN\"",
   "\"MARKET_DATA.HIGH\"",
   "\"MARKET_DATA.LOW\"",
   "\"MARKET_DATA.CLOSE\"",
   "\"MARKET_DATA.VOLUME\"",
   R"({"method":"VALUE.CONSTANT","params":{"value":1.25}})",
   R"({"method":"MARKET_DATA.FIELD","params":{"field":"custom"}})",
   R"({"method":"OPERATOR.LOOKBACK","params":{"period":2}})",
   R"({"method":"REQUESTED_ORDER.PRICE"})",
   R"({"method":"REQUESTED_ORDER.DIRECTION"})",
   R"({"method":"REQUESTED_ORDER.IS_PYRAMIDING"})",
   R"({"method":"REQUESTED_ORDER.RAW_QUANTITY"})",
   R"({"method":"REQUESTED_ORDER.RAW_QUANTITY_LIMIT"})",
   R"({"method":"REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY"})",
   R"({"method":"REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY_LIMIT"})",
   R"({"method":"REQUESTED_ORDER.QUANTITY"})",
   R"({"method":"REQUESTED_ORDER.NOTIONAL"})",
   R"({"method":"REQUESTED_ORDER.COST"})",
   R"({"method":"REQUESTED_ORDER.ESTIMATED_ENTRY_FEE"})",
   R"({"method":"REQUESTED_ORDER.ESTIMATED_1R_EXIT_FEE"})",
   R"({"method":"REQUESTED_ORDER.RISK_DISTANCE"})",
   R"({"method":"REQUESTED_ORDER.PRICE_RISK"})",
   R"({"method":"REQUESTED_ORDER.RISK_WITH_FEES"})",
   R"({"method":"REQUESTED_ORDER.FROZEN_UNIT_QUANTITY"})",
   R"({"method":"OPERATOR.ADD","params":{"augend":1,"addend":2}})",
   R"({"method":"OPERATOR.SUBTRACT","params":{"minuend":1,"subtrahend":2}})",
   R"({"method":"OPERATOR.MULTIPLY","params":{"multiplicand":1,"multiplier":2}})",
   R"({"method":"OPERATOR.DIVIDE","params":{"dividend":1,"divisor":2}})",
   R"({"method":"OPERATOR.ABS_DIFF","params":{"left":1,"right":2}})",
   R"({"method":"OPERATOR.MAX","params":{"left":1,"right":2}})",
   R"({"method":"OPERATOR.MIN","params":{"left":1,"right":2}})",
   R"({"method":"OPERATOR.NEGATE","params":{"operand":1}})",
   R"({"method":"OPERATOR.ABS","params":{"operand":1}})",
   R"({"method":"OPERATOR.SQRT","params":{"operand":4}})",
   R"({"method":"OPERATOR.POSITIVE_PART","params":{"operand":1}})",
   R"({"method":"OPERATOR.NEGATIVE_PART","params":{"operand":-1}})"};

  for(const auto& configuration : configurations) {
    SCOPED_TRACE(configuration);
    const auto config = json::parse(configuration);
    const auto node = parser.parse_node(config);
    EXPECT_EQ(parser.parse_node(parser.serialize_node(node)), node);
  }

  for(const auto* model_only : {"PORTFOLIO.EQUITY",
                                "PORTFOLIO.EQUITY_PERCENT",
                                "PORTFOLIO.DRAWDOWN",
                                "OPERATOR.CHANGE",
                                "INDICATOR.SMA",
                                "INDICATOR.EMA",
                                "INDICATOR.WMA",
                                "INDICATOR.RMA",
                                "INDICATOR.HMA",
                                "INDICATOR.RSI",
                                "INDICATOR.HIGHEST",
                                "INDICATOR.LOWEST",
                                "INDICATOR.ROC",
                                "INDICATOR.RVOL",
                                "INDICATOR.ATR",
                                "INDICATOR.TR",
                                "INDICATOR.KC",
                                "INDICATOR.DC",
                                "SERIES.REFERENCE",
                                "INPUT.NUMERIC",
                                "OPERATOR.SELECT_OUTPUT",
                                "INDICATOR.BB",
                                "INDICATOR.MACD",
                                "INDICATOR.STOCH",
                                "INDICATOR.STOCH_RSI",
                                "OPERATOR.PERCENTAGE",
                                "STOP_LOSS.AMOUNT",
                                "TAKE_PROFIT.AMOUNT",
                                "STOP_LOSS.PERCENT",
                                "TAKE_PROFIT.PERCENT",
                                "STOP_LOSS.ATR",
                                "TAKE_PROFIT.ATR",
                                "RISK_DISTANCE.AMOUNT",
                                "RISK_DISTANCE.PERCENT",
                                "RISK_DISTANCE.ATR",
                                "STOP_LOSS.ONE_R",
                                "STOP_LOSS.R_MULTIPLE",
                                "TAKE_PROFIT.R_MULTIPLE",
                                "POSITION.INITIAL_ENTRY_PRICE",
                                "POSITION.LATEST_ENTRY_PRICE",
                                "POSITION.AVERAGE_PRICE",
                                "POSITION.STOP_TARGET_REF_PRICE",
                                "POSITION.DIRECTION",
                                "POSITION.PYRAMIDING_LAYER",
                                "POSITION.R_MULTIPLE",
                                "COMPARISON.GREATER_THAN",
                                "COMPARISON.GREATER_EQUAL",
                                "COMPARISON.LESS_THAN",
                                "COMPARISON.LESS_EQUAL",
                                "COMPARISON.EQUAL",
                                "COMPARISON.NOT_EQUAL",
                                "COMPARISON.CROSSOVER",
                                "COMPARISON.CROSSUNDER",
                                "LOGIC.ALWAYS",
                                "LOGIC.NEVER",
                                "LOGIC.AND",
                                "LOGIC.OR",
                                "LOGIC.NOT",
                                "LOGIC.XOR",
                                "LOGIC.ALL_OF",
                                "LOGIC.ANY_OF"}) {
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
  , method_{node_to_erased_method<BacktestMethodContext>(node_, node_context)}
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
        "method": "MARKET_DATA.FIELD",
        "params": {
          "name": "high",
          "field": "high"
        }
      }
    )"));

    parse_node_method(json::parse(R"(
      {
        "method": "MARKET_DATA.FIELD",
        "params": {
          "name": "low",
          "field": "low"
        }
      }
    )"));

    parse_node_method(json::parse(R"(
      {
        "method": "MARKET_DATA.FIELD",
        "params": {
          "name": "close",
          "field": "close"
        }
      }
    )"));

    parse_node_method(json::parse(R"(
      {
        "method": "MARKET_DATA.FIELD",
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
          "method": "PLOT.MOMENTUM_HISTOGRAM",
          "params": {
            "source": {
              "method": "PLOT_SOURCE.SERIES",
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

  const auto strategy = backtest::parse_model_config_json("Test", config);
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

  const auto serialized = backtest::serialize_model_config_json(strategy);
  const auto& serialized_params =
   serialized.at("plots").at(0).at("items").at(0).at("params");
  EXPECT_TRUE(serialized_params.contains("positiveRisingColor"));
  EXPECT_TRUE(serialized_params.contains("positiveFallingColor"));
  EXPECT_TRUE(serialized_params.contains("negativeFallingColor"));
  EXPECT_TRUE(serialized_params.contains("negativeRisingColor"));

  const auto round_trip = backtest::parse_model_config_json("Test", serialized);
  EXPECT_EQ(method, round_trip.plots().at(0).items().at(0));
}

TEST(PlotMethodParserTest, RejectMomentumHistogramWithMissingColor)
{
  const auto config = json::parse(R"(
    {
      "version": 1,
      "plots": [{
        "items": [{
          "method": "PLOT.MOMENTUM_HISTOGRAM",
          "params": {
            "source": {
              "method": "PLOT_SOURCE.CONSTANT",
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
      "method": "SERIES.REFERENCE",
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
      "method": "OPERATOR.LOOKBACK",
      "params": {
        "period": 3,
        "source": {
          "method": "MARKET_DATA.CLOSE"
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
      "method": "INPUT.NUMERIC",
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
      "method": "OPERATOR.SELECT_OUTPUT",
      "params": {
        "output": "upper-band",
        "source": {
          "method": "INDICATOR.MACD",
          "params": {
            "fastPeriod": 12,
            "slowPeriod": 26,
            "signalPeriod": 9,
            "input": {
              "method": "MARKET_DATA.CLOSE"
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

  const auto macd_method =
   series_method_cast<MacdMethod<ErasedSeriesMethod<BacktestMethodContext>,
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
      "method": "OPERATOR.SELECT_OUTPUT",
      "params": {
        "output": "upper-band",
        "source": {
          "method": "INDICATOR.MACD",
          "params": {
            "fastPeriod": {"method": "VALUE.CONSTANT", "params": {"value": 12}},
            "slowPeriod": {"method": "VALUE.CONSTANT", "params": {"value": 26}},
            "signalPeriod": {"method": "VALUE.CONSTANT", "params": {"value": 9}},
            "source": {"method": "MARKET_DATA.CLOSE"}
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
      "method": "MARKET_DATA.OPEN"
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
      "method": "MARKET_DATA.HIGH"
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
      "method": "MARKET_DATA.LOW"
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
      "method": "MARKET_DATA.CLOSE"
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
      "method": "MARKET_DATA.VOLUME"
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
      "method": "INDICATOR.SMA",
      "params": {
        "period": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 14
          }
        },
        "source": {
          "method": "MARKET_DATA.FIELD",
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
      "method": "INDICATOR.EMA",
      "params": {
        "period": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 10
          }
        },
        "source": {
          "method": "MARKET_DATA.FIELD",
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
        "method": "INDICATOR.WMA",
        "params": {
          "period": {
            "method": "INPUT.NUMERIC",
            "params": {
              "representation": "UnsignedInteger",
              "value": 20
            }
          },
          "source": {
            "method": "MARKET_DATA.FIELD",
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
        "method": "INDICATOR.RMA",
        "params": {
          "period": {
            "method": "INPUT.NUMERIC",
            "params": {
              "representation": "UnsignedInteger",
              "value": 15
            }
          },
          "source": {
            "method": "MARKET_DATA.FIELD",
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
        "method": "INDICATOR.HMA",
        "params": {
          "period": {
            "method": "INPUT.NUMERIC",
            "params": {
              "representation": "UnsignedInteger",
              "value": 25
            }
          },
          "source": {
            "method": "MARKET_DATA.FIELD",
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
      "method": "INDICATOR.RSI",
      "params": {
        "period": 14,
        "source": {
          "method": "MARKET_DATA.FIELD",
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
      "method": "INDICATOR.STDDEV",
      "params": {
        "period": 20,
        "source": {
          "method": "MARKET_DATA.FIELD",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto stddev_method =
   series_method_cast<StddevMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                   ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
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
      "method": "VALUE.CONSTANT",
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
      "method": "MARKET_DATA.FIELD",
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
      "method": "PORTFOLIO.EQUITY"
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
      "method": "PORTFOLIO.EQUITY_PERCENT"
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
      "method": "PORTFOLIO.DRAWDOWN"
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
      "method": "INDICATOR.ATR",
      "params": {
        "period": {
          "method": "INPUT.NUMERIC",
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
      "method": "INDICATOR.BB",
      "params": {
        "maType": "SMA",
        "period": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 20
          }
        },
        "stddev": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "Decimal",
            "value": 2.0
          }
        },
        "maSource": {
          "method": "MARKET_DATA.FIELD",
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
      "method": "INDICATOR.MACD",
      "params": {
        "fastPeriod": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 12
          }
        },
        "slowPeriod": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 26
          }
        },
        "signalPeriod": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 9
          }
        },
        "source": {
          "method": "MARKET_DATA.FIELD",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto macd_method =
   series_method_cast<MacdMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                 ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
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
      "method": "INDICATOR.STOCH",
      "params": {
        "kPeriod": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 5
          }
        },
        "kSmooth": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 3
          }
        },
        "dPeriod": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 3
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto stoch_method =
   series_method_cast<StochMethod<ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
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
      "method": "INDICATOR.STOCH_RSI",
      "params": {
        "rsiPeriod": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 14
          }
        },
        "kPeriod": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 5
          }
        },
        "kSmooth": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 3
          }
        },
        "dPeriod": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 3
          }
        },
        "rsiSource": {
          "method": "MARKET_DATA.FIELD",
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
      "method": "INDICATOR.KC",
      "params": {
        "maMethodType": "SMA",
        "period": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 5
          }
        },
        "maSource": {
          "method": "MARKET_DATA.FIELD",
          "params": {
            "field": "close"
          }
        },
        "bandMethodType": "ATR",
        "bandAtrPeriod": {
          "method": "INPUT.NUMERIC",
          "params": {
            "representation": "UnsignedInteger",
            "value": 14
          }
        },
        "multiplier": {
          "method": "INPUT.NUMERIC",
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
      "method": "INDICATOR.DC",
      "params": {
        "period": {
          "method": "INPUT.NUMERIC",
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
   DonchianChannelMethod<ErasedSeriesMethod<BacktestMethodContext>>>(method);
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
      "method": "OPERATOR.ADD",
      "params": {
        "augend": 50,
        "addend": {
          "method": "VALUE.CONSTANT",
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
      "method": "OPERATOR.SUBTRACT",
      "params": {
        "minuend": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 100
          }
        },
        "subtrahend": {
          "method": "VALUE.CONSTANT",
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
      "method": "OPERATOR.MULTIPLY",
      "params": {
        "multiplicand": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 10
          }
        },
        "multiplier": {
          "method": "VALUE.CONSTANT",
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
      "method": "OPERATOR.DIVIDE",
      "params": {
        "dividend": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 100
          }
        },
        "divisor": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 2
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto divide_method =
   series_method_cast<DivideMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                   ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
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
      "method": "OPERATOR.NEGATE",
      "params": {
        "operand": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 42
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto negate_method =
   series_method_cast<NegateMethod<ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
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
      "method": "OPERATOR.SQRT",
      "params": {
        "operand": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 16
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto sqrt_method =
   series_method_cast<SqrtMethod<ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
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
      "method": "OPERATOR.CHANGE",
      "params": {
        "source": {
          "method": "MARKET_DATA.FIELD",
          "params": {
            "field": "close"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto changes_method =
   series_method_cast<ChangeMethod<ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
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
      "method": "OPERATOR.ABS_DIFF",
      "params": {
        "minuend": {
          "method": "MARKET_DATA.FIELD",
          "params": {
            "field": "high"
          }
        },
        "subtrahend": {
          "method": "MARKET_DATA.FIELD",
          "params": {
            "field": "low"
          }
        }
      }
    }
  )");

  const auto method = parse_node_method(config);

  const auto abs_diff_method =
   series_method_cast<AbsDiffMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                    ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
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
      "method": "OPERATOR.PERCENTAGE",
      "params": {
        "base": {
          "method": "VALUE.CONSTANT",
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
    R"({"method":"STOP_LOSS.AMOUNT","params":{"amount":{"method":"VALUE.CONSTANT","params":{"value":10}}}})"),
   json::parse(
    R"({"method":"TAKE_PROFIT.AMOUNT","params":{"amount":{"method":"VALUE.CONSTANT","params":{"value":20}}}})"),
   json::parse(
    R"({"method":"STOP_LOSS.PERCENT","params":{"percent":{"method":"VALUE.CONSTANT","params":{"value":10}}}})"),
   json::parse(
    R"({"method":"TAKE_PROFIT.PERCENT","params":{"percent":{"method":"VALUE.CONSTANT","params":{"value":20}}}})"),
   json::parse(
    R"({"method":"STOP_LOSS.ATR","params":{"period":{"method":"VALUE.CONSTANT","params":{"value":14}},"multiplier":{"method":"VALUE.CONSTANT","params":{"value":2}},"maSmoothingType":"RMA"}})"),
   json::parse(
    R"({"method":"TAKE_PROFIT.ATR","params":{"period":{"method":"VALUE.CONSTANT","params":{"value":14}},"multiplier":{"method":"VALUE.CONSTANT","params":{"value":2}},"maSmoothingType":"RMA"}})"),
   json::parse(
    R"({"method":"STOP_LOSS.R_MULTIPLE","params":{"multiple":{"method":"VALUE.CONSTANT","params":{"value":1}}}})"),
   json::parse(
    R"({"method":"TAKE_PROFIT.R_MULTIPLE","params":{"multiple":{"method":"VALUE.CONSTANT","params":{"value":2}}}})")};

  for(const auto& config : configs) {
    const auto method = parse_node_method(config);
    const auto serialized_config = serialize_node_method(method);
    const auto deserialized_config = parse_node_method(serialized_config);
    EXPECT_EQ(method, deserialized_config);
  }
}

TEST_F(ConfigParserTest, ParsePositionContextValueMethods)
{
  const auto configs = std::vector<json>{
   json::parse(R"({"method":"POSITION.INITIAL_ENTRY_PRICE"})"),
   json::parse(R"({"method":"POSITION.LATEST_ENTRY_PRICE"})"),
   json::parse(R"({"method":"POSITION.AVERAGE_PRICE"})"),
   json::parse(R"({"method":"POSITION.STOP_TARGET_REF_PRICE"})"),
   json::parse(R"({"method":"POSITION.DIRECTION"})"),
   json::parse(R"({"method":"POSITION.PYRAMIDING_LAYER"})")};

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
   parse_node_method(json::parse(R"("POSITION.R_MULTIPLE")"));
  const auto custom_method = parse_node_method(json::parse(
   R"({"method":"POSITION.R_MULTIPLE","params":{"source":"MARKET_DATA.OPEN"}})"));

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
    EXPECT_EQ(serialized_config.at("method"), "POSITION.R_MULTIPLE");
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
      "method": "LOGIC.ALL_OF",
      "params": {
        "items": [
          {
            "method": "COMPARISON.GREATER_THAN",
            "params": {
              "threshold": {
                "method": "VALUE.CONSTANT",
                "params": {
                  "value": 100
                }
              },
              "target": {
                "method": "MARKET_DATA.FIELD",
                "params": {
                  "field": "close"
                }
              }
            }
          },
          {
            "method": "COMPARISON.LESS_THAN",
            "params": {
              "threshold": {
                "method": "VALUE.CONSTANT",
                "params": {
                  "value": 200
                }
              },
              "target": {
                "method": "MARKET_DATA.FIELD",
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
      "method": "LOGIC.ANY_OF",
      "params": {
        "items": [
          {
            "method": "COMPARISON.GREATER_THAN",
            "params": {
              "threshold": {
                "method": "VALUE.CONSTANT",
                "params": {
                  "value": 100
                }
              },
              "target": {
                "method": "MARKET_DATA.FIELD",
                "params": {
                  "field": "close"
                }
              }
            }
          },
          {
            "method": "COMPARISON.LESS_THAN",
            "params": {
              "threshold": {
                "method": "VALUE.CONSTANT",
                "params": {
                  "value": 200
                }
              },
              "target": {
                "method": "MARKET_DATA.FIELD",
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
      "method": "COMPARISON.GREATER_THAN",
      "params": {
        "threshold": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "MARKET_DATA.FIELD",
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
      "method": "COMPARISON.GREATER_EQUAL",
      "params": {
        "threshold": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "MARKET_DATA.FIELD",
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
      "method": "COMPARISON.LESS_THAN",
      "params": {
        "threshold": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "MARKET_DATA.FIELD",
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
      "method": "COMPARISON.LESS_EQUAL",
      "params": {
        "threshold": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "MARKET_DATA.FIELD",
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
      "method": "COMPARISON.EQUAL",
      "params": {
        "threshold": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "MARKET_DATA.FIELD",
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
      "method": "COMPARISON.NOT_EQUAL",
      "params": {
        "threshold": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 100
          }
        },
        "target": {
          "method": "MARKET_DATA.FIELD",
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
      "method": "COMPARISON.CROSSUNDER",
      "params": {
        "value": {
          "method": "MARKET_DATA.FIELD",
          "params": {
            "field": "close"
          }
        },
        "baseline": {
          "method": "VALUE.CONSTANT",
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
      "method": "COMPARISON.CROSSOVER",
      "params": {
        "value": {
          "method": "MARKET_DATA.FIELD",
          "params": {
            "field": "close"
          }
        },
        "baseline": {
          "method": "VALUE.CONSTANT",
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
      "method": "LOGIC.ALWAYS"
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
      "method": "LOGIC.NEVER"
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
      "method": "LOGIC.AND",
      "params": {
        "firstCondition": {
          "method": "LOGIC.ALWAYS"
        },
        "secondCondition": {
          "method": "LOGIC.NEVER"
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
      "method": "LOGIC.OR",
      "params": {
        "firstCondition": {
          "method": "LOGIC.ALWAYS"
        },
        "secondCondition": {
          "method": "LOGIC.NEVER"
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
      "method": "LOGIC.NOT",
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
      "method": "LOGIC.XOR",
      "params": {
        "firstCondition": {
          "method": "LOGIC.ALWAYS"
        },
        "secondCondition": {
          "method": "LOGIC.NEVER"
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
      "method": "INDICATOR.HIGHEST",
      "params": {
        "period": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 14
          }
        },
        "source": {
          "method": "MARKET_DATA.CLOSE"
        }
      }
    }
  )");

  const auto method = parse_node_method(config);
  const auto highest_method =
   series_method_cast<HighestMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                    ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
  ASSERT_NE(highest_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseLowestMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "INDICATOR.LOWEST",
      "params": {
        "period": {
          "method": "VALUE.CONSTANT",
          "params": {
            "value": 14
          }
        },
        "source": {
          "method": "MARKET_DATA.CLOSE"
        }
      }
    }
  )");

  const auto method = parse_node_method(config);
  const auto lowest_method =
   series_method_cast<LowestMethod<ErasedSeriesMethod<BacktestMethodContext>,
                                   ErasedSeriesMethod<BacktestMethodContext>>>(
    method);
  ASSERT_NE(lowest_method, nullptr);

  const auto serialized_config = serialize_node_method(method);
  const auto deserialized_config = parse_node_method(serialized_config);
  EXPECT_EQ(method, deserialized_config);
}

TEST_F(ConfigParserTest, ParseTrMethod)
{
  const auto config = json::parse(R"(
    {
      "method": "INDICATOR.TR"
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
   json::parse(R"({"method":"LOGIC.ALWAYS"})"),
   json::parse(R"({"method":"LOGIC.NEVER"})"),
   json::parse(R"({"method":"LOGIC.NOT","params":{"condition":true}})"),
   json::parse(
    R"({"method":"LOGIC.AND","params":{"firstCondition":true,"secondCondition":false}})"),
   json::parse(
    R"({"method":"LOGIC.OR","params":{"firstCondition":false,"secondCondition":true}})"),
   json::parse(
    R"({"method":"LOGIC.XOR","params":{"firstCondition":true,"secondCondition":false}})"),
   json::parse(
    R"({"method":"LOGIC.ALL_OF","params":{"items":[true,false,true]}})"),
   json::parse(
    R"({"method":"LOGIC.ANY_OF","params":{"items":[false,false,true]}})"),
   json::parse(
    R"({"method":"COMPARISON.CROSSOVER","params":{"value":{"method":"VALUE.CONSTANT","params":{"value":2}},"baseline":{"method":"VALUE.CONSTANT","params":{"value":1}}}})"),
   json::parse(
    R"({"method":"COMPARISON.CROSSUNDER","params":{"value":{"method":"VALUE.CONSTANT","params":{"value":1}},"baseline":{"method":"VALUE.CONSTANT","params":{"value":2}}}})"),
   json::parse(
    R"({"method":"COMPARISON.GREATER_THAN","params":{"target":{"method":"VALUE.CONSTANT","params":{"value":2}},"threshold":{"method":"VALUE.CONSTANT","params":{"value":1}}}})"),
   json::parse(
    R"({"method":"COMPARISON.GREATER_EQUAL","params":{"target":{"method":"VALUE.CONSTANT","params":{"value":2}},"threshold":{"method":"VALUE.CONSTANT","params":{"value":2}}}})"),
   json::parse(
    R"({"method":"COMPARISON.LESS_THAN","params":{"target":{"method":"VALUE.CONSTANT","params":{"value":1}},"threshold":{"method":"VALUE.CONSTANT","params":{"value":2}}}})"),
   json::parse(
    R"({"method":"COMPARISON.LESS_EQUAL","params":{"target":{"method":"VALUE.CONSTANT","params":{"value":1}},"threshold":{"method":"VALUE.CONSTANT","params":{"value":1}}}})"),
   json::parse(
    R"({"method":"COMPARISON.EQUAL","params":{"target":{"method":"VALUE.CONSTANT","params":{"value":1}},"threshold":{"method":"VALUE.CONSTANT","params":{"value":1}}}})"),
   json::parse(
    R"({"method":"COMPARISON.NOT_EQUAL","params":{"target":{"method":"VALUE.CONSTANT","params":{"value":1}},"threshold":{"method":"VALUE.CONSTANT","params":{"value":2}}}})")};

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
      "method": "LOGIC.AND"
    }
  )");

  EXPECT_THROW(config_parser.parse_node(config), std::exception);
}

TEST_F(ConfigParserTest, SeriesNodeRegistrySerializationDeserialization)
{
  const auto config = json::parse(R"(
    {
      "name1": {
        "method": "MARKET_DATA.FIELD",
        "params": {
          "field": "close"
        }
      },
      "name2": {
        "method": "VALUE.CONSTANT",
        "params": {
          "value": 100
        }
      }
    }
  )");

  auto series_nodes = OrderedNamedRegistry<ErasedNode<BacktestMethodContext>>{};
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
      "method": "LOGIC.AND",
      "firstCondition": {
        "method": "COMPARISON.GREATER_EQUAL",
        "target": {
          "method": "MODEL_PERFORMANCE.VALUE",
          "params": {
            "metric": "LIFETIME_COUNT"
          }
        },
        "threshold": {
          "method": "VALUE.CONSTANT",
          "value": 25
        }
      },
      "secondCondition": {
        "method": "COMPARISON.LESS_EQUAL",
        "target": {
          "method": "REQUESTED_ORDER.RISK_WITH_FEES"
        },
        "threshold": {
          "method": "PORTFOLIO.EQUITY"
        }
      }
    }
  )");

  const auto node = backtest::parse_entry_filter_node(config);
  const auto serialized = backtest::serialize_entry_filter_node(node);
  EXPECT_EQ(serialized.at("firstCondition").at("target").at("method"),
            "MODEL_PERFORMANCE.VALUE");
  EXPECT_EQ(serialized.at("firstCondition")
             .at("target")
             .at("params")
             .at("metric")
             .as<std::string>(),
            "LIFETIME_COUNT");
  EXPECT_EQ(node, backtest::parse_entry_filter_node(serialized));
}

TEST_F(ConfigParserTest, EntryFilterRejectsMarketDataNodes)
{
  const auto config = json::parse(R"(
    {
      "method": "COMPARISON.GREATER_THAN",
      "target": {
        "method": "MARKET_DATA.CLOSE"
      },
      "threshold": {
        "method": "VALUE.CONSTANT",
        "value": 100
      }
    }
  )");

  EXPECT_THROW(backtest::parse_entry_filter_node(config),
               std::invalid_argument);
}

TEST_F(ConfigParserTest, EntryFilterRejectsNonBooleanRoot)
{
  const auto config = json::parse(
   R"({"method":"MODEL_PERFORMANCE.VALUE","params":{"metric":"WIN_RATE"}})");

  EXPECT_THROW(backtest::parse_entry_filter_node(config),
               std::invalid_argument);
}

TEST_F(ConfigParserTest, EntryFilterRejectsLegacyStrategyPerformanceMethod)
{
  const auto config = json::parse(R"(
    {"method":"COMPARISON.GREATER_THAN","target":{"method":"MODEL_PERFORMANCE","params":{"metric":"WIN_RATE"}},"threshold":0}
  )");

  EXPECT_THROW(backtest::parse_entry_filter_node(config),
               std::invalid_argument);
}

TEST_F(ConfigParserTest, EntryFilterRejectsIntegerPerformanceMetric)
{
  const auto config = json::parse(R"(
    {
      "method": "COMPARISON.GREATER_THAN",
      "target": {
        "method": "MODEL_PERFORMANCE.VALUE",
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
      "method": "COMPARISON.GREATER_THAN",
      "target": {
        "method": "MODEL_PERFORMANCE.VALUE"
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
      "method": "COMPARISON.GREATER_THAN",
      "target": {
        "method": "MODEL_PERFORMANCE.VALUE",
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
      "method": "COMPARISON.GREATER_THAN",
      "target": {
        "method": "MODEL_PERFORMANCE.VALUE",
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
      "method": "COMPARISON.GREATER_THAN",
      "target": {
        "method": "MODEL_PERFORMANCE.VALUE",
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
