module;
#include <array>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#ifdef __EMSCRIPTEN__
#include "../emscripten_js_imports.hpp"
#else
#include <nfd.hpp>
#endif

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.strategies_window;

import pludux.backtest;
import :window_context;

namespace pludux::apps {

using pludux::backtest::AnyPlotMethod;
using pludux::backtest::AnyPlotSourceMethod;
using pludux::backtest::AveragePriceNode;
using pludux::backtest::DrawdownNode;
using pludux::backtest::EquityNode;
using pludux::backtest::EquityPercentNode;
using pludux::backtest::HLinePlotMethod;
using pludux::backtest::InitialEntryPriceNode;
using pludux::backtest::LatestEntryPriceNode;
using pludux::backtest::PositionDirectionNode;
using pludux::backtest::SlAmountNode;
using pludux::backtest::SlAtrNode;
using pludux::backtest::SlPercentNode;
using pludux::backtest::StopTargetRefPriceNode;
using pludux::backtest::TpAmountNode;
using pludux::backtest::TpAtrNode;
using pludux::backtest::TpPercentNode;
using pludux::backtest::TpRMultipleNode;
using LinePlotMethod = pludux::backtest::LinePlotMethod<AnyPlotSourceMethod>;
using HistogramPlotMethod =
 pludux::backtest::HistogramPlotMethod<AnyPlotSourceMethod>;
using pludux::backtest::ConstantPlotSourceMethod;
using pludux::backtest::SeriesPlotSourceMethod;

auto get_default_series_node(const std::string& series_id) -> ErasedNode
{
  if(series_id == "OPEN") {
    return OpenNode{};
  } else if(series_id == "CLOSE") {
    return CloseNode{};
  } else if(series_id == "HIGH") {
    return HighNode{};
  } else if(series_id == "LOW") {
    return LowNode{};
  } else if(series_id == "VOLUME") {
    return VolumeNode{};
  } else if(series_id == "CHANGE") {
    return ChangeNode{CloseNode{}};
  } else if(series_id == "DATA") {
    return DataNode{};
  } else if(series_id == "EQUITY") {
    return EquityNode{};
  } else if(series_id == "EQUITY_PERCENT") {
    return EquityPercentNode{};
  } else if(series_id == "DRAWDOWN") {
    return DrawdownNode{};
  } else if(series_id == "SMA") {
    return SmaNode{CloseNode{}, 14};
  } else if(series_id == "EMA") {
    return EmaNode{CloseNode{}, 14};
  } else if(series_id == "WMA") {
    return WmaNode{CloseNode{}, 14};
  } else if(series_id == "HMA") {
    return HmaNode{CloseNode{}, 14};
  } else if(series_id == "RMA") {
    return RmaNode{CloseNode{}, 14};
  } else if(series_id == "RSI") {
    return RsiNode{CloseNode{}, 14};
  } else if(series_id == "ROC") {
    return RocNode{CloseNode{}, 14};
  } else if(series_id == "RVOL") {
    return RvolNode{14};
  } else if(series_id == "HIGHEST") {
    return HighestNode{CloseNode{}, 14};
  } else if(series_id == "LOWEST") {
    return LowestNode{CloseNode{}, 14};
  } else if(series_id == "TR") {
    return TrNode{};
  } else if(series_id == "MACD") {
    return MacdNode{CloseNode{}, 12, 26, 9};
  } else if(series_id == "ATR") {
    return AtrNode{14};
  } else if(series_id == "STDDEV") {
    return StddevNode{CloseNode{}, 14};
  } else if(series_id == "BB") {
    return BbNode{CloseNode{}, 20, 2.0, MaNodeType::Sma};
  } else if(series_id == "KC") {
    return KcNode{
     CloseNode{}, 20, 1.5, 14, KcBandNodeType::Atr, MaNodeType::Ema};
  } else if(series_id == "DC") {
    return DonchianChannelNode{};
  } else if(series_id == "STOCH") {
    return StochNode{14, 3, 3};
  } else if(series_id == "STOCH_RSI") {
    return StochRsiNode{CloseNode{}, 14, 14, 3, 3};
  } else if(series_id == "SERIES_REFERENCE") {
    return SeriesNode{""};
  } else if(series_id == "SERIES_RESULT") {
    return SeriesResultNode{""};
  } else if(series_id == "VALUE") {
    return ValueNode{0.0};
  } else if(series_id == "LOOKBACK") {
    return LookbackNode{CloseNode{}, 1};
  } else if(series_id == "ALL_OF") {
    return AllOfNode{};
  } else if(series_id == "ANY_OF") {
    return AnyOfNode{};
  } else if(series_id == "ALWAYS") {
    return TrueNode{};
  } else if(series_id == "NEVER") {
    return FalseNode{};
  } else if(series_id == "LESS_THAN") {
    return LessThanNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "GREATER_THAN") {
    return GreaterThanNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "LESS_EQUAL") {
    return LessEqualNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "GREATER_EQUAL") {
    return GreaterEqualNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "EQUAL") {
    return EqualNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "NOT_EQUAL") {
    return NotEqualNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "CROSSOVER") {
    return CrossoverNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "CROSSUNDER") {
    return CrossunderNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "NOT") {
    return LogicalNotNode{FalseNode{}};
  } else if(series_id == "AND") {
    return LogicalAndNode{FalseNode{}, FalseNode{}};
  } else if(series_id == "OR") {
    return LogicalOrNode{FalseNode{}, FalseNode{}};
  } else if(series_id == "XOR") {
    return LogicalXorNode{FalseNode{}, FalseNode{}};
  } else if(series_id == "INPUT") {
    return NumericInputNode{"Input"};
  } else if(series_id == "ADD") {
    return AddNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "SUBTRACT") {
    return SubtractNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "MULTIPLY") {
    return MultiplyNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "DIVIDE") {
    return DivideNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "NEGATE") {
    return NegateNode{CloseNode{}};
  } else if(series_id == "SQRT") {
    return SqrtNode{CloseNode{}};
  } else if(series_id == "PERCENTAGE") {
    return PercentageNode{CloseNode{}, 100.0};
  } else if(series_id == "SL_AMOUNT") {
    return SlAmountNode{1000.0};
  } else if(series_id == "TP_AMOUNT") {
    return TpAmountNode{2000.0};
  } else if(series_id == "SL_PERCENT") {
    return SlPercentNode{10.0};
  } else if(series_id == "TP_PERCENT") {
    return TpPercentNode{20.0};
  } else if(series_id == "SL_ATR") {
    return SlAtrNode{14.0, 2.0};
  } else if(series_id == "TP_ATR") {
    return TpAtrNode{14.0, 4.0};
  } else if(series_id == "TP_R_MULTIPLE") {
    return TpRMultipleNode{2.0};
  } else if(series_id == "INITIAL_ENTRY_PRICE") {
    return InitialEntryPriceNode{};
  } else if(series_id == "LATEST_ENTRY_PRICE") {
    return LatestEntryPriceNode{};
  } else if(series_id == "AVERAGE_PRICE") {
    return AveragePriceNode{};
  } else if(series_id == "STOP_TARGET_REF_PRICE") {
    return StopTargetRefPriceNode{};
  } else if(series_id == "POSITION_DIRECTION") {
    return PositionDirectionNode{};
  } else if(series_id == "ABS_DIFF") {
    return AbsDiffNode{CloseNode{}, CloseNode{}};
  } else if(series_id == "SELECT_OUTPUT") {
    return SelectOutputNode{CloseNode{}, NodeOutput::MiddleBand};
  }

  throw std::invalid_argument{
   std::format("Unknown series node id: {}", series_id)};
}

auto get_series_node_id(const ErasedNode& node) -> std::string
{
  if(node_cast<SelectOutputNode>(node)) {
    return "SELECT_OUTPUT";
  } else if(node_cast<SeriesNode>(node)) {
    return "SERIES_REFERENCE";
  } else if(node_cast<SeriesResultNode>(node)) {
    return "SERIES_RESULT";
  } else if(node_cast<CloseNode>(node)) {
    return "CLOSE";
  } else if(node_cast<OpenNode>(node)) {
    return "OPEN";
  } else if(node_cast<HighNode>(node)) {
    return "HIGH";
  } else if(node_cast<LowNode>(node)) {
    return "LOW";
  } else if(node_cast<VolumeNode>(node)) {
    return "VOLUME";
  } else if(node_cast<ChangeNode>(node)) {
    return "CHANGE";
  } else if(node_cast<DataNode>(node)) {
    return "DATA";
  } else if(node_cast<EquityNode>(node)) {
    return "EQUITY";
  } else if(node_cast<EquityPercentNode>(node)) {
    return "EQUITY_PERCENT";
  } else if(node_cast<DrawdownNode>(node)) {
    return "DRAWDOWN";
  } else if(node_cast<ValueNode>(node)) {
    return "VALUE";
  } else if(node_cast<StddevNode>(node)) {
    return "STDDEV";
  } else if(node_cast<BbNode>(node)) {
    return "BB";
  } else if(node_cast<KcNode>(node)) {
    return "KC";
  } else if(node_cast<DonchianChannelNode>(node)) {
    return "DC";
  } else if(node_cast<SmaNode>(node)) {
    return "SMA";
  } else if(node_cast<EmaNode>(node)) {
    return "EMA";
  } else if(node_cast<WmaNode>(node)) {
    return "WMA";
  } else if(node_cast<HmaNode>(node)) {
    return "HMA";
  } else if(node_cast<RmaNode>(node)) {
    return "RMA";
  } else if(node_cast<RsiNode>(node)) {
    return "RSI";
  } else if(node_cast<RocNode>(node)) {
    return "ROC";
  } else if(node_cast<RvolNode>(node)) {
    return "RVOL";
  } else if(node_cast<HighestNode>(node)) {
    return "HIGHEST";
  } else if(node_cast<LowestNode>(node)) {
    return "LOWEST";
  } else if(node_cast<TrNode>(node)) {
    return "TR";
  } else if(node_cast<MacdNode>(node)) {
    return "MACD";
  } else if(node_cast<AtrNode>(node)) {
    return "ATR";
  } else if(node_cast<StochNode>(node)) {
    return "STOCH";
  } else if(node_cast<StochRsiNode>(node)) {
    return "STOCH_RSI";
  } else if(node_cast<AddNode>(node)) {
    return "ADD";
  } else if(node_cast<SubtractNode>(node)) {
    return "SUBTRACT";
  } else if(node_cast<MultiplyNode>(node)) {
    return "MULTIPLY";
  } else if(node_cast<DivideNode>(node)) {
    return "DIVIDE";
  } else if(node_cast<NegateNode>(node)) {
    return "NEGATE";
  } else if(node_cast<SqrtNode>(node)) {
    return "SQRT";
  } else if(node_cast<PercentageNode>(node)) {
    return "PERCENTAGE";
  } else if(node_cast<SlAmountNode>(node)) {
    return "SL_AMOUNT";
  } else if(node_cast<TpAmountNode>(node)) {
    return "TP_AMOUNT";
  } else if(node_cast<SlPercentNode>(node)) {
    return "SL_PERCENT";
  } else if(node_cast<TpPercentNode>(node)) {
    return "TP_PERCENT";
  } else if(node_cast<SlAtrNode>(node)) {
    return "SL_ATR";
  } else if(node_cast<TpAtrNode>(node)) {
    return "TP_ATR";
  } else if(node_cast<TpRMultipleNode>(node)) {
    return "TP_R_MULTIPLE";
  } else if(node_cast<InitialEntryPriceNode>(node)) {
    return "INITIAL_ENTRY_PRICE";
  } else if(node_cast<LatestEntryPriceNode>(node)) {
    return "LATEST_ENTRY_PRICE";
  } else if(node_cast<AveragePriceNode>(node)) {
    return "AVERAGE_PRICE";
  } else if(node_cast<StopTargetRefPriceNode>(node)) {
    return "STOP_TARGET_REF_PRICE";
  } else if(node_cast<PositionDirectionNode>(node)) {
    return "POSITION_DIRECTION";
  } else if(node_cast<AbsDiffNode>(node)) {
    return "ABS_DIFF";
  } else if(node_cast<LookbackNode>(node)) {
    return "LOOKBACK";
  } else if(node_cast<NumericInputNode>(node)) {
    return "INPUT";
  } else if(node_cast<AllOfNode>(node)) {
    return "ALL_OF";
  } else if(node_cast<AnyOfNode>(node)) {
    return "ANY_OF";
  } else if(node_cast<TrueNode>(node)) {
    return "ALWAYS";
  } else if(node_cast<FalseNode>(node)) {
    return "NEVER";
  } else if(node_cast<LessThanNode>(node)) {
    return "LESS_THAN";
  } else if(node_cast<GreaterThanNode>(node)) {
    return "GREATER_THAN";
  } else if(node_cast<LessEqualNode>(node)) {
    return "LESS_EQUAL";
  } else if(node_cast<GreaterEqualNode>(node)) {
    return "GREATER_EQUAL";
  } else if(node_cast<EqualNode>(node)) {
    return "EQUAL";
  } else if(node_cast<NotEqualNode>(node)) {
    return "NOT_EQUAL";
  } else if(node_cast<CrossoverNode>(node)) {
    return "CROSSOVER";
  } else if(node_cast<CrossunderNode>(node)) {
    return "CROSSUNDER";
  } else if(node_cast<LogicalNotNode>(node)) {
    return "NOT";
  } else if(node_cast<LogicalAndNode>(node)) {
    return "AND";
  } else if(node_cast<LogicalOrNode>(node)) {
    return "OR";
  } else if(node_cast<LogicalXorNode>(node)) {
    return "XOR";
  }

  return "UNKNOWN";
}

auto get_series_node_title(const std::string& series_id) -> std::string
{
  if(series_id == "OPEN") {
    return "Open Price";
  } else if(series_id == "CLOSE") {
    return "Close Price";
  } else if(series_id == "HIGH") {
    return "High Price";
  } else if(series_id == "LOW") {
    return "Low Price";
  } else if(series_id == "VOLUME") {
    return "Volume";
  } else if(series_id == "CHANGE") {
    return "Change";
  } else if(series_id == "DATA") {
    return "Data";
  } else if(series_id == "EQUITY") {
    return "Equity";
  } else if(series_id == "EQUITY_PERCENT") {
    return "Equity (%)";
  } else if(series_id == "DRAWDOWN") {
    return "Drawdown";
  } else if(series_id == "SMA") {
    return "Simple Moving Average (SMA)";
  } else if(series_id == "EMA") {
    return "Exponential Moving Average (EMA)";
  } else if(series_id == "WMA") {
    return "Weighted Moving Average (WMA)";
  } else if(series_id == "HMA") {
    return "Hull Moving Average (HMA)";
  } else if(series_id == "RMA") {
    return "Running Moving Average (RMA)";
  } else if(series_id == "RSI") {
    return "Relative Strength Index (RSI)";
  } else if(series_id == "ROC") {
    return "Rate of Change (ROC)";
  } else if(series_id == "RVOL") {
    return "Relative Volume (RVOL)";
  } else if(series_id == "HIGHEST") {
    return "Highest";
  } else if(series_id == "LOWEST") {
    return "Lowest";
  } else if(series_id == "TR") {
    return "True Range (TR)";
  } else if(series_id == "MACD") {
    return "Moving Average Convergence Divergence (MACD)";
  } else if(series_id == "ATR") {
    return "Average True Range (ATR)";
  } else if(series_id == "STDDEV") {
    return "Standard Deviation (STDDEV)";
  } else if(series_id == "BB") {
    return "Bollinger Bands";
  } else if(series_id == "KC") {
    return "Keltner Channel (KC)";
  } else if(series_id == "DC") {
    return "Donchian Channels (DC)";
  } else if(series_id == "STOCH") {
    return "Stochastic Oscillator";
  } else if(series_id == "STOCH_RSI") {
    return "Stochastic RSI";
  } else if(series_id == "SERIES_REFERENCE") {
    return "Series Node";
  } else if(series_id == "SERIES_RESULT") {
    return "Series Result";
  } else if(series_id == "VALUE") {
    return "Value";
  } else if(series_id == "LOOKBACK") {
    return "Lookback";
  } else if(series_id == "ALL_OF") {
    return "All Of";
  } else if(series_id == "ANY_OF") {
    return "Any Of";
  } else if(series_id == "ALWAYS") {
    return "Always";
  } else if(series_id == "NEVER") {
    return "Never";
  } else if(series_id == "LESS_THAN") {
    return "Less Than";
  } else if(series_id == "GREATER_THAN") {
    return "Greater Than";
  } else if(series_id == "LESS_EQUAL") {
    return "Less Equal";
  } else if(series_id == "GREATER_EQUAL") {
    return "Greater Equal";
  } else if(series_id == "EQUAL") {
    return "Equal";
  } else if(series_id == "NOT_EQUAL") {
    return "Not Equal";
  } else if(series_id == "CROSSOVER") {
    return "Crossover";
  } else if(series_id == "CROSSUNDER") {
    return "Crossunder";
  } else if(series_id == "NOT") {
    return "Not";
  } else if(series_id == "AND") {
    return "And";
  } else if(series_id == "OR") {
    return "Or";
  } else if(series_id == "XOR") {
    return "Xor";
  } else if(series_id == "INPUT") {
    return "From Input";
  } else if(series_id == "ADD") {
    return "Addition";
  } else if(series_id == "SUBTRACT") {
    return "Subtraction";
  } else if(series_id == "MULTIPLY") {
    return "Multiplication";
  } else if(series_id == "DIVIDE") {
    return "Division";
  } else if(series_id == "NEGATE") {
    return "Negation";
  } else if(series_id == "SQRT") {
    return "Square Root";
  } else if(series_id == "PERCENTAGE") {
    return "Percentage";
  } else if(series_id == "SL_AMOUNT") {
    return "Stop Loss Amount";
  } else if(series_id == "TP_AMOUNT") {
    return "Take Profit Amount";
  } else if(series_id == "SL_PERCENT") {
    return "Stop Loss Percent";
  } else if(series_id == "TP_PERCENT") {
    return "Take Profit Percent";
  } else if(series_id == "SL_ATR") {
    return "Stop Loss ATR";
  } else if(series_id == "TP_ATR") {
    return "Take Profit ATR";
  } else if(series_id == "TP_R_MULTIPLE") {
    return "Take Profit R Multiple";
  } else if(series_id == "INITIAL_ENTRY_PRICE") {
    return "Initial Entry Price";
  } else if(series_id == "LATEST_ENTRY_PRICE") {
    return "Latest Entry Price";
  } else if(series_id == "AVERAGE_PRICE") {
    return "Average Price";
  } else if(series_id == "STOP_TARGET_REF_PRICE") {
    return "Stop/Target Reference Price";
  } else if(series_id == "POSITION_DIRECTION") {
    return "Position Direction";
  } else if(series_id == "ABS_DIFF") {
    return "Absolute Difference";
  } else if(series_id == "SELECT_OUTPUT") {
    return "Select Output";
  }

  return "Unknown";
}

auto get_condition_node_id(const ErasedNode& node) -> std::string
{
  if(node_cast<AllOfNode>(node)) {
    return "ALL_OF";
  } else if(node_cast<AnyOfNode>(node)) {
    return "ANY_OF";
  } else if(node_cast<TrueNode>(node)) {
    return "ALWAYS";
  } else if(node_cast<FalseNode>(node)) {
    return "NEVER";
  } else if(node_cast<LessThanNode>(node)) {
    return "LESS_THAN";
  } else if(node_cast<GreaterThanNode>(node)) {
    return "GREATER_THAN";
  } else if(node_cast<LessEqualNode>(node)) {
    return "LESS_EQUAL";
  } else if(node_cast<GreaterEqualNode>(node)) {
    return "GREATER_EQUAL";
  } else if(node_cast<EqualNode>(node)) {
    return "EQUAL";
  } else if(node_cast<NotEqualNode>(node)) {
    return "NOT_EQUAL";
  } else if(node_cast<CrossoverNode>(node)) {
    return "CROSSOVER";
  } else if(node_cast<CrossunderNode>(node)) {
    return "CROSSUNDER";
  } else if(node_cast<LogicalNotNode>(node)) {
    return "NOT";
  } else if(node_cast<LogicalAndNode>(node)) {
    return "AND";
  } else if(node_cast<LogicalOrNode>(node)) {
    return "OR";
  } else if(node_cast<LogicalXorNode>(node)) {
    return "XOR";
  }

  return "UNKNOWN";
}

auto get_condition_node_title(const std::string& condition_id) -> std::string
{
  if(condition_id == "ALL_OF") {
    return "All Of";
  } else if(condition_id == "ANY_OF") {
    return "Any Of";
  } else if(condition_id == "ALWAYS") {
    return "Always";
  } else if(condition_id == "NEVER") {
    return "Never";
  } else if(condition_id == "LESS_THAN") {
    return "Less Than";
  } else if(condition_id == "GREATER_THAN") {
    return "Greater Than";
  } else if(condition_id == "LESS_EQUAL") {
    return "Less Equal";
  } else if(condition_id == "GREATER_EQUAL") {
    return "Greater Equal";
  } else if(condition_id == "EQUAL") {
    return "Equal";
  } else if(condition_id == "NOT_EQUAL") {
    return "Not Equal";
  } else if(condition_id == "CROSSOVER") {
    return "Crossover";
  } else if(condition_id == "CROSSUNDER") {
    return "Crossunder";
  } else if(condition_id == "NOT") {
    return "Not";
  } else if(condition_id == "AND") {
    return "And";
  } else if(condition_id == "OR") {
    return "Or";
  } else if(condition_id == "XOR") {
    return "Xor";
  }

  return "Unknown";
}

auto get_default_condition_node(const std::string& condition_id) -> ErasedNode
{
  if(condition_id == "ALL_OF") {
    return AllOfNode{};
  } else if(condition_id == "ANY_OF") {
    return AnyOfNode{};
  } else if(condition_id == "ALWAYS") {
    return TrueNode{};
  } else if(condition_id == "NEVER") {
    return FalseNode{};
  } else if(condition_id == "LESS_THAN") {
    return LessThanNode{CloseNode{}, CloseNode{}};
  } else if(condition_id == "GREATER_THAN") {
    return GreaterThanNode{CloseNode{}, CloseNode{}};
  } else if(condition_id == "LESS_EQUAL") {
    return LessEqualNode{CloseNode{}, CloseNode{}};
  } else if(condition_id == "GREATER_EQUAL") {
    return GreaterEqualNode{CloseNode{}, CloseNode{}};
  } else if(condition_id == "EQUAL") {
    return EqualNode{CloseNode{}, CloseNode{}};
  } else if(condition_id == "NOT_EQUAL") {
    return NotEqualNode{CloseNode{}, CloseNode{}};
  } else if(condition_id == "CROSSOVER") {
    return CrossoverNode{CloseNode{}, CloseNode{}};
  } else if(condition_id == "CROSSUNDER") {
    return CrossunderNode{CloseNode{}, CloseNode{}};
  } else if(condition_id == "NOT") {
    return LogicalNotNode{FalseNode{}};
  } else if(condition_id == "AND") {
    return LogicalAndNode{FalseNode{}, FalseNode{}};
  } else if(condition_id == "OR") {
    return LogicalOrNode{FalseNode{}, FalseNode{}};
  } else if(condition_id == "XOR") {
    return LogicalXorNode{FalseNode{}, FalseNode{}};
  }

  throw std::invalid_argument{
   std::format("Unknown condition node id: {}", condition_id)};
}

auto get_plot_method_id(const AnyPlotMethod& method) -> std::string
{
  if(plot_method_cast<HLinePlotMethod>(method)) {
    return "HLINE";
  }

  if(plot_method_cast<LinePlotMethod>(method)) {
    return "LINE";
  }

  if(plot_method_cast<HistogramPlotMethod>(method)) {
    return "HISTOGRAM";
  }

  return "UNKNOWN";
}

auto get_plot_method_title(const std::string& plot_id) -> std::string
{
  if(plot_id == "LINE") {
    return "Line";
  } else if(plot_id == "HLINE") {
    return "Horizontal Line";
  } else if(plot_id == "HISTOGRAM") {
    return "Histogram";
  }

  return "Unknown";
}

auto get_default_plot_method(const std::string& plot_id) -> AnyPlotMethod
{
  if(plot_id == "LINE") {
    return LinePlotMethod{ConstantPlotSourceMethod{0.0}, 0xFFFFFFFF};
  }

  if(plot_id == "HLINE") {
    return HLinePlotMethod{0.0, 0xFFFFFFFF};
  }

  if(plot_id == "HISTOGRAM") {
    return HistogramPlotMethod{ConstantPlotSourceMethod{0.0}, 0xFFFFFFFF};
  }

  throw std::invalid_argument{
   std::format("Unknown plot method id: {}", plot_id)};
}

auto get_plot_source_method_id(const AnyPlotSourceMethod& method) -> std::string
{
  if(plot_source_method_cast<ConstantPlotSourceMethod>(method)) {
    return "CONSTANT";
  } else if(plot_source_method_cast<SeriesPlotSourceMethod>(method)) {
    return "SERIES_RESULT";
  }

  return "UNKNOWN";
}

auto get_plot_source_method_title(const std::string& plot_source_id)
 -> std::string
{
  if(plot_source_id == "CONSTANT") {
    return "Constant Value";
  }

  if(plot_source_id == "SERIES_RESULT") {
    return "Series Result";
  }

  return "Unknown";
}

auto get_default_plot_source_method(const std::string& plot_source_id)
 -> AnyPlotSourceMethod
{
  if(plot_source_id == "CONSTANT") {
    return ConstantPlotSourceMethod{0.0};
  }

  if(plot_source_id == "SERIES_RESULT") {
    return SeriesPlotSourceMethod{""};
  }

  throw std::invalid_argument{
   std::format("Unknown plot source method id: {}", plot_source_id)};
}

export class StrategiesWindow {
public:
  StrategiesWindow()
  {
  }

  void render(this auto& self, WindowContext& context)
  {
    ImGui::Begin("Strategies", nullptr);

    switch(self.current_page_) {
    case Page::AddNew:
      self.render_add_new_strategy(context);
      break;
    case Page::Edit:
      self.render_edit_strategy(context);
      break;
    case Page::List:
    default:
      self.render_list_strategies(context);
      break;
    }

    ImGui::End();
  }

private:
  enum class Page { List, AddNew, Edit } current_page_{Page::List};

  std::optional<backtest::StrategyStoreHandle> selected_strategy_handle_opt_;
  std::shared_ptr<backtest::Strategy> editing_strategy_ptr_;

  std::vector<std::string> available_series_names_;
  std::unordered_map<std::string, std::string> changed_series_names_;

  void render_list_strategies(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto& strategy_handles = app_state.get_strategy_handles();
    const auto backtest_ptr = app_state.selected_backtest_if_present();

    ImGui::BeginGroup();
    ImGui::BeginChild(
     "item view",
     ImVec2(
      0,
      -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us

    for(std::size_t i = 0; i < strategy_handles.size(); ++i) {
      const auto strategy_handle = strategy_handles[i];
      const auto& strategy = app_state.get_strategy(strategy_handle);
      const auto& strategy_name = strategy.name();

      ImGui::PushID(i);

      ImGui::SetNextItemAllowOverlap();
      auto is_selected =
       backtest_ptr && backtest_ptr->strategy_handle() == strategy_handle;
      const auto row_start = ImGui::GetCursorScreenPos();
      const auto row_width = ImGui::GetContentRegionAvail().x;
      const auto row_height = ImGui::GetFrameHeight();

      ImGui::Selectable("##strategy_row",
                        &is_selected,
                        ImGuiSelectableFlags_AllowOverlap,
                        ImVec2(row_width, row_height));

      ImGui::SetCursorScreenPos(row_start);
      ImGui::AlignTextToFramePadding();
      ImGui::TextUnformatted(strategy_name.c_str());

      const auto spacing = ImGui::GetStyle().ItemSpacing.x;
      const auto frame_padding_x = ImGui::GetStyle().FramePadding.x;
      const auto export_width =
       ImGui::CalcTextSize("Export").x + (2.0f * frame_padding_x);
      const auto edit_width =
       ImGui::CalcTextSize("Edit").x + (2.0f * frame_padding_x);
      const auto delete_width =
       ImGui::CalcTextSize("Delete").x + (2.0f * frame_padding_x);
      const auto more_width =
       ImGui::CalcTextSize("More...").x + (2.0f * frame_padding_x);
      const auto buttons_width = export_width + spacing + edit_width + spacing +
                                 delete_width + spacing + more_width;
      const auto buttons_start_x = row_start.x + row_width - buttons_width;

      ImGui::SetCursorScreenPos(ImVec2(buttons_start_x, row_start.y));

      if(ImGui::Button("Export")) {
        auto serialized_strategy = stringify_backtest_strategy(strategy);

#ifdef __EMSCRIPTEN__
        const auto file_name = "pludux-strategy-" + strategy_name + ".json";
        const auto& file_content = serialized_strategy;
        pludux_js_save_file(
         file_name.c_str(), file_content.c_str(), "application/json");

#else
        auto nfd_guard = NFD::Guard{};
        auto out_path = NFD::UniquePath{};

        const auto filter_item =
         std::array<nfdfilteritem_t, 1>{{"JSON Files", "json"}};

        auto result =
         NFD::SaveDialog(out_path, filter_item.data(), filter_item.size());

        if(result == NFD_OKAY) {
          const auto saved_path = std::string(out_path.get());
          context.push_action(
           [saved_path, serialized_strategy](ApplicationState& app_state) {
             auto out_stream = std::ofstream{saved_path};

             if(!out_stream.is_open()) {
               const auto error_message =
                std::format("Failed to open '{}' for writing.", saved_path);
               throw std::runtime_error(error_message);
             }

             out_stream << serialized_strategy;
           });
        } else if(result == NFD_CANCEL) {
          // User cancelled the save dialog
        } else {
          const auto error_message =
           std::format("Error '{}': {}", "Export", NFD::GetError());
          throw std::runtime_error(error_message);
        }
#endif
      }
      ImGui::SameLine();
      if(ImGui::Button("Edit")) {
        self.selected_strategy_handle_opt_ = strategy_handle;
        self.editing_strategy_ptr_ =
         std::make_shared<backtest::Strategy>(strategy);
        self.current_page_ = Page::Edit;
      }
      ImGui::SameLine();
      if(ImGui::Button("Delete")) {
        context.push_action([strategy_handle](ApplicationState& app_state) {
          app_state.remove_strategy(strategy_handle);
        });
      }

      ImGui::SameLine();
      if(ImGui::Button("More...")) {
        ImGui::OpenPopup("strategy_menu_more");
      }

      if(ImGui::BeginPopup("strategy_menu_more")) {
        if(ImGui::MenuItem("Duplicate")) {
          context.push_action([strategy_handle](ApplicationState& app_state) {
            const auto& strategy = app_state.get_strategy(strategy_handle);
            auto duplicate_strategy = strategy;
            duplicate_strategy.name(strategy.name() + " Copy");
            app_state.add_strategy(std::move(duplicate_strategy));
          });
        }

        const auto move_up_disabled = i == 0;
        if(ImGui::MenuItem("Move Up", nullptr, false, !move_up_disabled)) {
          context.push_action(
           [from_index = i, to_index = i - 1](ApplicationState& app_state) {
             app_state.reorder_list_strategy(from_index, to_index);
           });
        }

        const auto move_down_disabled = i == strategy_handles.size() - 1;
        if(ImGui::MenuItem("Move Down", nullptr, false, !move_down_disabled)) {
          context.push_action(
           [from_index = i, to_index = i + 1](ApplicationState& app_state) {
             app_state.reorder_list_strategy(from_index, to_index);
           });
        }

        ImGui::EndPopup();
      }

      ImGui::PopID();
    }

    ImGui::EndChild();
    if(ImGui::Button("Add New Strategy")) {
      self.current_page_ = Page::AddNew;

      self.selected_strategy_handle_opt_ = std::nullopt;

      self.editing_strategy_ptr_ = std::make_shared<backtest::Strategy>();
    }

    ImGui::SameLine();

    if(ImGui::Button("Import Strategies")) {
#ifdef __EMSCRIPTEN__

      using JsOnOpenedFileContentReady =
       std::function<void(const std::string&, const std::string&, void*)>;

      static const auto callback =
       JsOnOpenedFileContentReady{[](const std::string& file_name,
                                     const std::string& file_data,
                                     void* user_data) {
         auto& context = *reinterpret_cast<WindowContext*>(user_data);

         auto action = LoadStrategyJsonAction{file_name, file_data};
         context.push_action(std::move(action));
       }};

      pludux_js_open_multiple_text_files(".json", &callback, &context);
#else
      auto nfd_guard = NFD::Guard{};
      auto in_paths = NFD::UniquePathSet{};

      const auto filter_item =
       std::array<nfdfilteritem_t, 1>{{"JSON Files", "json"}};

      auto result = NFD::OpenDialogMultiple(
       in_paths, filter_item.data(), filter_item.size());

      try {
        if(result == NFD_OKAY) {
          auto paths_count = nfdpathsetsize_t{};

          result = NFD::PathSet::Count(in_paths, paths_count);
          if(result == NFD_ERROR) {
            const auto error_message =
             std::format("Error '{}': {}", "Import", NFD::GetError());
            throw std::runtime_error(error_message);
          }

          for(nfdpathsetsize_t i = 0; i < paths_count; ++i) {
            auto in_path = NFD::UniquePathSetPath{};
            result = NFD::PathSet::GetPath(in_paths, i, in_path);

            if(result == NFD_ERROR) {
              const auto error_message =
               std::format("Error '{}': {}", "Import", NFD::GetError());
              throw std::runtime_error(error_message);
            } else {
              const auto selected_path = std::string(in_path.get());
              context.push_action(LoadStrategyJsonAction{selected_path});
            }
          }

        } else if(result == NFD_CANCEL) {
          // User cancelled the open dialog
        } else {
          const auto error_message =
           std::format("Error '{}': {}", "Import", NFD::GetError());
          throw std::runtime_error(error_message);
        }
      } catch(const std::exception& ex) {
        const auto error_message = std::string(ex.what());
        context.alert(error_message);
      }
#endif
    }

    ImGui::EndGroup();
  }

  void render_add_new_strategy(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("add_new_strategy",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Add New Strategy");
    ImGui::Separator();
    ImGui::SetNextItemWidth(-1);

    {
      self.edit_strategy_form(context);
    }

    ImGui::EndChild();

    if(ImGui::Button("Create")) {
      self.submit_strategy_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::EndGroup();
  }

  void render_edit_strategy(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    ImGui::BeginChild("edit_strategy",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    ImGui::Text("Edit Strategy");
    ImGui::Separator();
    ImGui::SetNextItemWidth(-1);

    {
      self.edit_strategy_form(context);
    }

    ImGui::EndChild();

    const auto selected_strategy_handle =
     self.selected_strategy_handle_opt_.value();
    const auto& selected_strategy =
     context.app_state().get_strategy(selected_strategy_handle);

    const auto same_strategy =
     selected_strategy == *(self.editing_strategy_ptr_);

    if(ImGui::Button("OK")) {
      self.submit_strategy_changes(context);
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.reset();
    }

    ImGui::BeginDisabled(same_strategy);
    ImGui::SameLine();
    if(ImGui::Button("Apply")) {
      self.submit_strategy_changes(context);
    }
    ImGui::EndDisabled();

    ImGui::EndGroup();
  }

  void edit_strategy_form(this auto& self, WindowContext& context)
  {
    ImGui::BeginChild("edit_content",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

    {
      ImGui::Text("Strategy Name:");
      ImGui::SameLine();
      auto strategy_name = self.editing_strategy_ptr_->name();
      ImGui::InputText("##strategy_name", &strategy_name);
      self.editing_strategy_ptr_->name(strategy_name);
      ImGui::Text("");
    }

    {
      ImGui::SeparatorText("Series Nodes");

      auto& series_nodes = self.editing_strategy_ptr_->series_nodes();
      auto updated_series_nodes = series_nodes;
      self.changed_series_names_.clear();
      self.available_series_names_.clear();
      for(auto id_counter = 0;
          auto& [series_name, series_node] : series_nodes) {
        ImGui::PushID(id_counter++);

        ImGui::Text("Name:");
        ImGui::SameLine();
        auto updated_series_name = series_name;
        ImGui::InputText("##series_name", &updated_series_name);
        if(ImGui::IsItemDeactivatedAfterEdit()) {
          if(updated_series_name != series_name) {
            self.changed_series_names_[series_name] = updated_series_name;
          }
        }

        ImGui::Text("Node:");
        ImGui::SameLine();
        ImGui::PushID("series_node");
        self.render_series_node(series_node, context);
        ImGui::PopID();
        updated_series_nodes.set(series_name, series_node);

        // Delete button for the series. Right aligned on the new line
        const auto spacing = ImGui::GetStyle().ItemSpacing.x;
        const auto frame_padding_x = ImGui::GetStyle().FramePadding.x;
        const auto delete_width =
         ImGui::CalcTextSize("Delete").x + (2.0f * frame_padding_x);
        const auto line_start = ImGui::GetCursorScreenPos();
        const auto line_width = ImGui::GetContentRegionAvail().x;
        const auto delete_button_x = line_start.x + line_width - delete_width;
        ImGui::SetCursorScreenPos(ImVec2(delete_button_x, line_start.y));

        if(ImGui::Button("Delete")) {
          updated_series_nodes.remove(series_name);
          self.changed_series_names_.erase(series_name);
        } else {
          self.available_series_names_.push_back(series_name);
        }

        ImGui::Separator();
        ImGui::PopID();
      }

      for(auto& [old_name, new_name] : self.changed_series_names_) {
        if(updated_series_nodes.rename(old_name, new_name)) {
          auto it = std::ranges::find(self.available_series_names_, old_name);
          if(it != self.available_series_names_.end()) {
            *it = new_name;
          }
        } else {
          const auto error_message = std::format(
           "Failed to rename series '{}' to '{}'.", old_name, new_name);
          context.alert(error_message);
        }
      }

      series_nodes = std::move(updated_series_nodes);

      if(ImGui::Button("Add Series")) {
        auto new_series_name =
         std::format("new_var_{}", series_nodes.size() + 1);
        auto new_series_node = get_default_series_node("CLOSE");
        series_nodes.set(new_series_name, new_series_node);
      }

      ImGui::Text("");
    }

    {
      ImGui::SeparatorText("Positions");

      {
        ImGui::Text("Long Position:");
        ImGui::PushID("long_position");
        ImGui::Indent();

        auto long_position = self.editing_strategy_ptr_->long_position();
        self.render_position_form(long_position, context);
        self.editing_strategy_ptr_->long_position(std::move(long_position));

        ImGui::Unindent();
        ImGui::PopID();
      }

      ImGui::Text("");

      {
        ImGui::Text("Short Position:");
        ImGui::PushID("short_position");
        ImGui::Indent();

        auto short_position = self.editing_strategy_ptr_->short_position();
        self.render_position_form(short_position, context);
        self.editing_strategy_ptr_->short_position(std::move(short_position));

        ImGui::Unindent();
        ImGui::PopID();
      }

      ImGui::Text("");
    }

    {
      ImGui::SeparatorText("Plots");

      auto plot_groups = self.editing_strategy_ptr_->plots();
      for(auto i = 0; i < plot_groups.size(); ++i) {
        auto& plot_group = plot_groups[i];

        ImGui::PushID(i);

        ImGui::Text("Plot Name:");
        ImGui::SameLine();
        auto plot_name = plot_group.name();
        ImGui::InputText("##plot_name", &plot_name);
        plot_group.name(plot_name);

        ImGui::Text("Overlays");
        ImGui::SameLine();
        auto overlay_enabled = plot_group.is_overlay();
        ImGui::Checkbox("##overlay_enabled", &overlay_enabled);
        plot_group.is_overlay(overlay_enabled);

        ImGui::Separator();

        {
          auto plot_items = plot_group.items();
          for(auto j = 0; j < plot_items.size(); ++j) {
            auto& plot_method = plot_items[j];
            ImGui::PushID(j);

            self.render_plot_method(plot_method, context);

            const auto btn_label = "Remove Item";
            {
              const auto spacing = ImGui::GetStyle().ItemSpacing.x;
              const auto frame_padding_x = ImGui::GetStyle().FramePadding.x;
              const auto delete_width =
               ImGui::CalcTextSize(btn_label).x + (2.0f * frame_padding_x);
              const auto line_start = ImGui::GetCursorScreenPos();
              const auto line_width = ImGui::GetContentRegionAvail().x;
              const auto delete_button_x =
               line_start.x + line_width - delete_width;
              ImGui::SetCursorScreenPos(ImVec2(delete_button_x, line_start.y));
            }
            if(ImGui::Button(btn_label)) {
              plot_items.erase(plot_items.begin() + j);
              --j; // Adjust index after removal
            }

            ImGui::Separator();
            ImGui::PopID();
          }

          if(ImGui::Button("Add Item")) {
            plot_items.emplace_back(get_default_plot_method("HLINE"));
          }

          plot_group.items(plot_items);
        }

        const auto btn_label = "Remove Plot";
        {
          const auto spacing = ImGui::GetStyle().ItemSpacing.x;
          const auto frame_padding_x = ImGui::GetStyle().FramePadding.x;
          const auto delete_width =
           ImGui::CalcTextSize(btn_label).x + (2.0f * frame_padding_x);
          const auto line_start = ImGui::GetCursorScreenPos();
          const auto line_width = ImGui::GetContentRegionAvail().x;
          const auto delete_button_x = line_start.x + line_width - delete_width;
          ImGui::SetCursorScreenPos(ImVec2(delete_button_x, line_start.y));
        }
        if(ImGui::Button(btn_label)) {
          plot_groups.erase(plot_groups.begin() + i);
          --i; // Adjust index after removal
        }

        ImGui::Separator();
        ImGui::PopID();
      }

      if(ImGui::Button("Add Plot")) {
        plot_groups.emplace_back("New Plot");
      }

      self.editing_strategy_ptr_->plots(std::move(plot_groups));
    }

    ImGui::EndChild();
  }

  void render_position_form(this auto& self,
                            backtest::Strategy::Position& position,
                            WindowContext& context)
  {
    {
      ImGui::Text("Entry:");
      ImGui::PushID("entry");
      auto entry = position.entry();
      auto changed_node = self.render_condition_node(entry.signal(), context);
      entry.signal(std::move(changed_node));
      auto signal_delay = static_cast<int>(entry.signal_delay());
      ImGui::Text("Signal Delay:");
      ImGui::SameLine();
      if(ImGui::InputInt("##signal_delay", &signal_delay)) {
        if(signal_delay < 0) {
          signal_delay = 0;
        }
        entry.signal_delay(static_cast<std::size_t>(signal_delay));
      }
      ImGui::Text("Entry Price:");
      ImGui::SameLine();
      auto price = entry.price();
      ImGui::PushID("price");
      self.render_series_node(price, context);
      ImGui::PopID();
      entry.price(std::move(price));
      position.entry(std::move(entry));
      ImGui::PopID();
    }
    {
      ImGui::Separator();
      ImGui::Text("Exit:");
      ImGui::PushID("exit");
      auto exit = position.exit();
      auto changed_node = self.render_condition_node(exit.signal(), context);
      exit.signal(std::move(changed_node));
      auto signal_delay = static_cast<int>(exit.signal_delay());
      ImGui::Text("Signal Delay:");
      ImGui::SameLine();
      if(ImGui::InputInt("##signal_delay", &signal_delay)) {
        if(signal_delay < 0) {
          signal_delay = 0;
        }
        exit.signal_delay(static_cast<std::size_t>(signal_delay));
      }
      ImGui::Text("Exit Price:");
      ImGui::SameLine();
      auto price = exit.price();
      ImGui::PushID("price");
      self.render_series_node(price, context);
      ImGui::PopID();
      exit.price(std::move(price));
      position.exit(std::move(exit));
      ImGui::PopID();
    }
    {
      ImGui::SeparatorText("Pyramiding");
      ImGui::PushID("pyramiding");

      ImGui::Text("Signal:");
      auto pyramiding = position.pyramiding();

      auto changed_node =
       self.render_condition_node(pyramiding.signal(), context);
      pyramiding.signal(std::move(changed_node));

      auto signal_delay = static_cast<int>(pyramiding.signal_delay());
      ImGui::Text("Signal Delay:");
      ImGui::SameLine();
      if(ImGui::InputInt("##signal_delay", &signal_delay)) {
        if(signal_delay < 0) {
          signal_delay = 0;
        }
        pyramiding.signal_delay(static_cast<std::size_t>(signal_delay));
      }

      ImGui::Text("Pyramiding Price:");
      ImGui::SameLine();
      auto price = pyramiding.price();
      ImGui::PushID("price");
      self.render_series_node(price, context);
      ImGui::PopID();
      pyramiding.price(std::move(price));

      auto pyramiding_max_layers = static_cast<int>(pyramiding.max_layers());
      ImGui::Text("Max Layers:");
      ImGui::SameLine();
      if(ImGui::InputInt("##max_layers", &pyramiding_max_layers)) {
        if(pyramiding_max_layers < 1) {
          pyramiding_max_layers = 1;
        }
        pyramiding.max_layers(pyramiding_max_layers);
      }

      const auto reference_options =
       std::vector<std::pair<backtest::StopTargetReferencePrice, std::string>>{
        {backtest::StopTargetReferencePrice::LatestEntryPrice,
         "Latest Entry Price"},
        {backtest::StopTargetReferencePrice::AveragePrice, "Average Price"},
        {backtest::StopTargetReferencePrice::InitialEntryPrice,
         "Initial Entry Price"}};
      const auto render_reference_combo =
       [&reference_options](const char* label,
                            const char* id,
                            backtest::StopTargetReferencePrice current) {
         ImGui::Text("%s", label);
         ImGui::SameLine();

         auto selected = current;
         auto preview = std::string{"Latest Entry Price"};
         for(const auto& [reference, title] : reference_options) {
           if(reference == current) {
             preview = title;
             break;
           }
         }

         if(ImGui::BeginCombo(id, preview.c_str())) {
           for(const auto& [reference, title] : reference_options) {
             const auto is_selected = selected == reference;
             if(ImGui::Selectable(title.c_str(), is_selected)) {
               selected = reference;
             }
             if(is_selected) {
               ImGui::SetItemDefaultFocus();
             }
           }
           ImGui::EndCombo();
         }

         return selected;
       };

      pyramiding.favorable_stop_target_reference(
       render_reference_combo("Favorable SL/TP Reference:",
                              "##favorable_stop_target_reference",
                              pyramiding.favorable_stop_target_reference()));
      pyramiding.unfavorable_stop_target_reference(
       render_reference_combo("Unfavorable SL/TP Reference:",
                              "##unfavorable_stop_target_reference",
                              pyramiding.unfavorable_stop_target_reference()));

      position.pyramiding(std::move(pyramiding));
      ImGui::PopID();
    }
    {
      ImGui::SeparatorText("Stop Loss");
      ImGui::PushID("stop_loss");

      auto stop_loss = position.stop_loss();
      auto stop_loss_enabled = stop_loss.enabled();
      auto stop_loss_trailing = stop_loss.trailing();

      ImGui::Checkbox("Enable Stop Loss", &stop_loss_enabled);
      ImGui::Checkbox("Enable Trailing Stop Loss", &stop_loss_trailing);
      stop_loss.enabled(stop_loss_enabled);
      stop_loss.trailing(stop_loss_trailing);

      ImGui::Text("Stop Price:");
      ImGui::SameLine();
      auto stop_price = stop_loss.stop_price();
      ImGui::PushID("stop_price");
      self.render_series_node(stop_price, context);
      ImGui::PopID();
      stop_loss.stop_price(std::move(stop_price));

      position.stop_loss(std::move(stop_loss));
      ImGui::PopID();
    }
    {
      ImGui::SeparatorText("Take Profit");
      ImGui::PushID("take_profit");

      auto take_profit = position.take_profit();
      auto take_profit_enabled = take_profit.enabled();
      ImGui::Checkbox("Enable Take Profit", &take_profit_enabled);
      take_profit.enabled(take_profit_enabled);

      ImGui::Text("Target Price:");
      ImGui::SameLine();
      auto target_price = take_profit.target_price();
      ImGui::PushID("target_price");
      self.render_series_node(target_price, context);
      ImGui::PopID();
      take_profit.target_price(std::move(target_price));

      position.take_profit(std::move(take_profit));
      ImGui::PopID();
    }
  }

  void render_series_node(this auto& self,
                          ErasedNode& series_node,
                          WindowContext& context)
  {
    static const std::vector<std::string> series_ids = {"OPEN",
                                                        "CLOSE",
                                                        "HIGH",
                                                        "LOW",
                                                        "VOLUME",
                                                        "CHANGE",
                                                        "INPUT",
                                                        "ADD",
                                                        "SUBTRACT",
                                                        "MULTIPLY",
                                                        "DIVIDE",
                                                        "NEGATE",
                                                        "SQRT",
                                                        "PERCENTAGE",
                                                        "SL_AMOUNT",
                                                        "TP_AMOUNT",
                                                        "SL_PERCENT",
                                                        "TP_PERCENT",
                                                        "SL_ATR",
                                                        "TP_ATR",
                                                        "TP_R_MULTIPLE",
                                                        "INITIAL_ENTRY_PRICE",
                                                        "LATEST_ENTRY_PRICE",
                                                        "AVERAGE_PRICE",
                                                        "STOP_TARGET_REF_PRICE",
                                                        "POSITION_DIRECTION",
                                                        "ABS_DIFF",
                                                        "DATA",
                                                        "EQUITY",
                                                        "EQUITY_PERCENT",
                                                        "DRAWDOWN",
                                                        "SMA",
                                                        "EMA",
                                                        "WMA",
                                                        "HMA",
                                                        "RMA",
                                                        "RSI",
                                                        "ROC",
                                                        "RVOL",
                                                        "HIGHEST",
                                                        "LOWEST",
                                                        "TR",
                                                        "MACD",
                                                        "ATR",
                                                        "STDDEV",
                                                        "BB",
                                                        "KC",
                                                        "DC",
                                                        "STOCH",
                                                        "STOCH_RSI",
                                                        "SELECT_OUTPUT",
                                                        "SERIES_REFERENCE",
                                                        "SERIES_RESULT",
                                                        "VALUE",
                                                        "LOOKBACK",
                                                        "ALL_OF",
                                                        "ANY_OF",
                                                        "ALWAYS",
                                                        "NEVER",
                                                        "LESS_THAN",
                                                        "GREATER_THAN",
                                                        "LESS_EQUAL",
                                                        "GREATER_EQUAL",
                                                        "EQUAL",
                                                        "NOT_EQUAL",
                                                        "CROSSOVER",
                                                        "CROSSUNDER",
                                                        "NOT",
                                                        "AND",
                                                        "OR",
                                                        "XOR"};

    auto series_node_id = get_series_node_id(series_node);
    {
      const auto combo_preview_value = get_series_node_title(series_node_id);
      if(ImGui::BeginCombo("##Series", combo_preview_value.c_str())) {
        static auto filter = ImGuiTextFilter{};

        if(ImGui::IsWindowAppearing()) {
          ImGui::SetKeyboardFocusHere();
          filter.Clear();
        }

        filter.Draw("##Filter", -FLT_MIN);

        for(const auto& series_id : series_ids) {
          const auto series_title = get_series_node_title(series_id);
          const bool is_selected = series_id == series_node_id;

          if(filter.PassFilter(series_title.c_str())) {
            if(ImGui::Selectable(series_title.c_str(), is_selected)) {
              if((series_id == "SERIES_REFERENCE" ||
                  series_id == "SERIES_RESULT") &&
                 self.available_series_names_.empty()) {
                const auto series_reference_node_title =
                 get_series_node_title(series_id);
                const auto error_message =
                 std::format("Cannot select '{}' when there are no available "
                             "series other than the current one.",
                             series_reference_node_title);
                context.alert(error_message);
              } else {
                series_node = get_default_series_node(series_id);
              }
            }
          }
        }
        ImGui::EndCombo();
      }
    }
    {
      ImGui::Indent();
      self.render_series_node_params(series_node, context);
      ImGui::Unindent();
    }
  }

  void render_series_node_params(this auto& self,
                                 ErasedNode& node,
                                 WindowContext& context)
  {
    ([&]<typename... Ts>() mutable {
      ([&]() mutable -> bool {
        if(auto specific_node = node_cast<Ts>(node)) {
          self.render_series_node_params(*specific_node, context);
          return true;
        }

        return false;
      }() || ...);
    }.template operator()<SelectOutputNode,
                          SeriesNode,
                          SeriesResultNode,
                          DataNode,
                          LookbackNode,
                          NumericInputNode,

                          AllOfNode,
                          AnyOfNode,
                          TrueNode,
                          FalseNode,
                          LessThanNode,
                          GreaterThanNode,
                          LessEqualNode,
                          GreaterEqualNode,
                          EqualNode,
                          NotEqualNode,
                          CrossoverNode,
                          CrossunderNode,
                          LogicalNotNode,
                          LogicalAndNode,
                          LogicalOrNode,
                          LogicalXorNode,

                          BbNode,
                          KcNode,
                          DonchianChannelNode,
                          StochNode,
                          StochRsiNode,
                          HighestNode,
                          LowestNode,
                          TrNode,
                          SmaNode,
                          EmaNode,
                          RmaNode,
                          WmaNode,
                          HmaNode,
                          RsiNode,
                          RocNode,
                          RvolNode,

                          MacdNode,
                          AtrNode,
                          ValueNode,
                          ChangeNode,
                          AddNode,
                          SubtractNode,
                          MultiplyNode,
                          DivideNode,
                          PercentageNode,
                          SlAmountNode,
                          TpAmountNode,
                          SlPercentNode,
                          TpPercentNode,
                          SlAtrNode,
                          TpAtrNode,
                          TpRMultipleNode,
                          InitialEntryPriceNode,
                          LatestEntryPriceNode,
                          AveragePriceNode,
                          StopTargetRefPriceNode,
                          PositionDirectionNode,
                          AbsDiffNode,
                          NegateNode,
                          SqrtNode,
                          StddevNode>());
  }

  void render_series_node_params(this auto& self,
                                 SelectOutputNode& node,
                                 WindowContext& context)
  {
    {
      const auto output_options =
       std::vector<NodeOutput>{NodeOutput::UpperBand,
                               NodeOutput::MiddleBand,
                               NodeOutput::LowerBand,
                               NodeOutput::MacdLine,
                               NodeOutput::SignalLine,
                               NodeOutput::Histogram,
                               NodeOutput::KPercent,
                               NodeOutput::DPercent};

      const auto output_map = std::unordered_map<NodeOutput, std::string>{
       {NodeOutput::UpperBand, "Upper Band"},
       {NodeOutput::MiddleBand, "Middle Band"},
       {NodeOutput::LowerBand, "Lower Band"},
       {NodeOutput::MacdLine, "MACD Line"},
       {NodeOutput::SignalLine, "Signal Line"},
       {NodeOutput::Histogram, "Histogram"},
       {NodeOutput::KPercent, "%K"},
       {NodeOutput::DPercent, "%D"}};

      ImGui::Text("Output:");
      ImGui::SameLine();
      auto output = node.output();
      {
        const auto output_str = output_map.at(output);
        if(ImGui::BeginCombo("##output", output_str.c_str())) {
          for(auto output_option : output_options) {
            const auto output_option_str = output_map.at(output_option);
            const bool is_selected = output == output_option;
            if(ImGui::Selectable(output_option_str.c_str(), is_selected)) {
              node.output(output_option);
            }
          }
          ImGui::EndCombo();
        }
      }
    }
    {
      ImGui::Text("Source:");
      ImGui::SameLine();
      auto output_source = node.source();
      ImGui::PushID("source");
      self.render_series_node(output_source, context);
      ImGui::PopID();
      node.source(std::move(output_source));
    }
  }

  void render_series_node_params(this auto& self,
                                 SeriesNode& node,
                                 WindowContext& context)
  {
    if(self.changed_series_names_.contains(node.name())) {
      const auto new_name = self.changed_series_names_.at(node.name());
      node.name(new_name);
    } else if(std::ranges::find(self.available_series_names_, node.name()) ==
              self.available_series_names_.end()) {
      node.name("");
    }

    ImGui::Text("Name:");
    ImGui::SameLine();

    const auto display_name = node.name();
    if(ImGui::BeginCombo("##named_series", display_name.c_str())) {
      for(const auto& name_option : self.available_series_names_) {
        ImGui::PushID(name_option.c_str());

        const bool is_selected = display_name == name_option;
        if(ImGui::Selectable(name_option.c_str(), is_selected)) {
          node.name(name_option);
        }

        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
  }

  void render_series_node_params(this auto& self,
                                 SeriesResultNode& node,
                                 WindowContext& context)
  {
    if(self.changed_series_names_.contains(node.name())) {
      const auto new_name = self.changed_series_names_.at(node.name());
      node.name(new_name);
    } else if(std::ranges::find(self.available_series_names_, node.name()) ==
              self.available_series_names_.end()) {
      node.name("");
    }

    ImGui::Text("Name:");
    ImGui::SameLine();

    const auto display_name = node.name();
    if(ImGui::BeginCombo("##named_series", display_name.c_str())) {
      for(const auto& name_option : self.available_series_names_) {
        ImGui::PushID(name_option.c_str());

        const bool is_selected = display_name == name_option;
        if(ImGui::Selectable(name_option.c_str(), is_selected)) {
          node.name(name_option);
        }

        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
  }

  void render_series_node_params(this auto& self,
                                 BbNode& node,
                                 WindowContext& context)
  {
    ImGui::Text("MA Type:");
    ImGui::SameLine();
    {
      const auto ma_type_options = std::vector<MaNodeType>{MaNodeType::Sma,
                                                           MaNodeType::Ema,
                                                           MaNodeType::Wma,
                                                           MaNodeType::Hma,
                                                           MaNodeType::Rma};

      const auto get_ma_type_string =
       [](MaNodeType ma_type) static -> std::string {
        switch(ma_type) {
        case MaNodeType::Sma:
          return "SMA";
        case MaNodeType::Ema:
          return "EMA";
        case MaNodeType::Wma:
          return "WMA";
        case MaNodeType::Hma:
          return "HMA";
        case MaNodeType::Rma:
          return "RMA";
        }

        std::unreachable();
      };

      const auto ma_type_str = get_ma_type_string(node.ma_node_type());
      if(ImGui::BeginCombo("##ma_type", ma_type_str.c_str())) {
        for(const auto& ma_type_option : ma_type_options) {
          const auto ma_type_option_str = get_ma_type_string(ma_type_option);
          const bool is_selected = ma_type_str == ma_type_option_str;
          if(ImGui::Selectable(ma_type_option_str.c_str(), is_selected)) {
            node.ma_node_type(ma_type_option);
          }
        }
        ImGui::EndCombo();
      }
    }

    ImGui::Text("Period:");
    ImGui::SameLine();
    auto period = node.period();
    ImGui::PushID("period");
    self.render_series_node(period, context);
    ImGui::PopID();
    node.period(std::move(period));

    ImGui::Text("StdDev:");
    ImGui::SameLine();
    auto stddev = node.stddev();
    ImGui::PushID("stddev");
    self.render_series_node(stddev, context);
    ImGui::PopID();
    node.stddev(std::move(stddev));

    ImGui::Text("Source:");
    ImGui::SameLine();
    auto source = node.source();
    ImGui::PushID("source");
    self.render_series_node(source, context);
    ImGui::PopID();
    node.source(std::move(source));
  }

  void render_series_node_params(this auto& self,
                                 KcNode& node,
                                 WindowContext& context)
  {
    {
      ImGui::Text("Length:");
      ImGui::SameLine();
      auto period = node.period();
      ImGui::PushID("period");
      self.render_series_node(period, context);
      ImGui::PopID();
      node.period(std::move(period));
    }
    {
      ImGui::Text("MA Type:");
      ImGui::SameLine();
      {
        const auto ma_type_options =
         std::unordered_map<MaNodeType, std::string>{{MaNodeType::Sma, "SMA"},
                                                     {MaNodeType::Ema, "EMA"},
                                                     {MaNodeType::Wma, "WMA"},
                                                     {MaNodeType::Hma, "HMA"},
                                                     {MaNodeType::Rma, "RMA"}};

        const auto ma_type_str = ma_type_options.at(node.ma_node_type());
        if(ImGui::BeginCombo("##kc_ma_type", ma_type_str.c_str())) {
          for(const auto& [ma_type_option, ma_type_option_str] :
              ma_type_options) {
            const bool is_selected = ma_type_str == ma_type_option_str;
            if(ImGui::Selectable(ma_type_option_str.c_str(), is_selected)) {
              node.ma_node_type(ma_type_option);
            }
          }
          ImGui::EndCombo();
        }
      }
    }
    {
      ImGui::Text("Source:");
      ImGui::SameLine();
      auto source = node.source();
      ImGui::PushID("source");
      self.render_series_node(source, context);
      ImGui::PopID();
      node.source(std::move(source));
    }
    {
      ImGui::Text("Band Type:");
      ImGui::SameLine();
      {
        const auto band_type_options =
         std::unordered_map<KcBandNodeType, std::string>{
          {KcBandNodeType::Atr, "ATR"},
          {KcBandNodeType::Tr, "True Range"},
          {KcBandNodeType::RangeHighLow, "Range (High-Low)"}};

        const auto band_type_str = band_type_options.at(node.band_node_type());
        if(ImGui::BeginCombo("##kc_band_type", band_type_str.c_str())) {
          for(const auto& [band_type_option, band_type_option_str] :
              band_type_options) {
            const bool is_selected = band_type_str == band_type_option_str;
            if(ImGui::Selectable(band_type_option_str.c_str(), is_selected)) {
              node.band_node_type(band_type_option);
            }
          }
          ImGui::EndCombo();
        }
      }
    }
    {
      ImGui::Text("ATR Length:");
      ImGui::SameLine();
      auto band_atr_period = node.band_atr_period();
      ImGui::PushID("band_atr_period");
      self.render_series_node(band_atr_period, context);
      ImGui::PopID();
      node.band_atr_period(std::move(band_atr_period));
    }
    {
      ImGui::Text("Multiplier:");
      ImGui::SameLine();
      auto multiplier = node.multiplier();
      ImGui::PushID("multiplier");
      self.render_series_node(multiplier, context);
      ImGui::PopID();
      node.multiplier(std::move(multiplier));
    }
  }

  void render_series_node_params(this auto& self,
                                 DonchianChannelNode& node,
                                 WindowContext& context)
  {
    {
      ImGui::Text("Length:");
      ImGui::SameLine();
      auto period = node.period();
      ImGui::PushID("period");
      self.render_series_node(period, context);
      ImGui::PopID();
      node.period(std::move(period));
    }
  }

  void render_series_node_params(this auto& self,
                                 StochNode& node,
                                 WindowContext& context)
  {
    {
      ImGui::Text("D Period:");
      ImGui::SameLine();
      auto d_period = node.d_period();
      ImGui::PushID("d_period");
      self.render_series_node(d_period, context);
      ImGui::PopID();
      node.d_period(std::move(d_period));
    }
    {
      ImGui::Text("K Period:");
      ImGui::SameLine();
      auto k_period = node.k_period();
      ImGui::PushID("k_period");
      self.render_series_node(k_period, context);
      ImGui::PopID();
      node.k_period(std::move(k_period));
    }
    {
      ImGui::Text("K Smooth:");
      ImGui::SameLine();
      auto k_smooth = node.k_smooth();
      ImGui::PushID("k_smooth");
      self.render_series_node(k_smooth, context);
      ImGui::PopID();
      node.k_smooth(std::move(k_smooth));
    }
  }

  void render_series_node_params(this auto& self,
                                 StochRsiNode& node,
                                 WindowContext& context)
  {
    {
      ImGui::Text("D Period:");
      ImGui::SameLine();
      auto d_period = node.d_period();
      ImGui::PushID("d_period");
      self.render_series_node(d_period, context);
      ImGui::PopID();
      node.d_period(std::move(d_period));
    }
    {
      ImGui::Text("K Period:");
      ImGui::SameLine();
      auto k_period = node.k_period();
      ImGui::PushID("k_period");
      self.render_series_node(k_period, context);
      ImGui::PopID();
      node.k_period(std::move(k_period));
    }
    {
      ImGui::Text("K Smooth:");
      ImGui::SameLine();
      auto k_smooth = node.k_smooth();
      ImGui::PushID("k_smooth");
      self.render_series_node(k_smooth, context);
      ImGui::PopID();
      node.k_smooth(std::move(k_smooth));
    }
    {
      ImGui::Text("RSI Period:");
      ImGui::SameLine();
      auto rsi_period = node.rsi_period();
      ImGui::PushID("rsi_period");
      self.render_series_node(rsi_period, context);
      ImGui::PopID();
      node.rsi_period(std::move(rsi_period));
    }
    {
      ImGui::Text("RSI Source:");
      ImGui::SameLine();
      auto rsi_source = node.rsi_source();
      ImGui::PushID("rsi_source");
      self.render_series_node(rsi_source, context);
      ImGui::PopID();
      node.rsi_source(std::move(rsi_source));
    }
  }

  void render_series_node_params(this auto& self,
                                 RvolNode& node,
                                 WindowContext& context)
  {
    ImGui::Text("Period:");
    ImGui::SameLine();
    auto period = node.period();
    ImGui::PushID("period");
    self.render_series_node(period, context);
    ImGui::PopID();
    node.period(std::move(period));
  }

  template<typename TConditionNode>
    requires std::same_as<TConditionNode, AllOfNode> ||
             std::same_as<TConditionNode, AnyOfNode> ||
             std::same_as<TConditionNode, TrueNode> ||
             std::same_as<TConditionNode, FalseNode> ||
             std::same_as<TConditionNode, LessThanNode> ||
             std::same_as<TConditionNode, GreaterThanNode> ||
             std::same_as<TConditionNode, LessEqualNode> ||
             std::same_as<TConditionNode, GreaterEqualNode> ||
             std::same_as<TConditionNode, EqualNode> ||
             std::same_as<TConditionNode, NotEqualNode> ||
             std::same_as<TConditionNode, CrossoverNode> ||
             std::same_as<TConditionNode, CrossunderNode> ||
             std::same_as<TConditionNode, LogicalNotNode> ||
             std::same_as<TConditionNode, LogicalAndNode> ||
             std::same_as<TConditionNode, LogicalOrNode> ||
             std::same_as<TConditionNode, LogicalXorNode>
  void render_series_node_params(this auto& self,
                                 TConditionNode& node,
                                 WindowContext& context)
  {
    auto changed_node = self.render_condition_node(ErasedNode{node}, context);
    if(const auto* updated = node_cast<TConditionNode>(changed_node)) {
      node = *updated;
    }
  }

  void render_series_node_params(this auto& self,
                                 TrNode& node,
                                 WindowContext& context)
  {
    ImGui::TextUnformatted("No parameters.");
  }

  template<typename TNodeWithPeriod>
    requires std::same_as<TNodeWithPeriod, SmaNode> ||
             std::same_as<TNodeWithPeriod, EmaNode> ||
             std::same_as<TNodeWithPeriod, RmaNode> ||
             std::same_as<TNodeWithPeriod, WmaNode> ||
             std::same_as<TNodeWithPeriod, HmaNode> ||
             std::same_as<TNodeWithPeriod, RsiNode> ||
             std::same_as<TNodeWithPeriod, RocNode> ||
             std::same_as<TNodeWithPeriod, HighestNode> ||
             std::same_as<TNodeWithPeriod, LowestNode> ||
             std::same_as<TNodeWithPeriod, StddevNode>
  void render_series_node_params(this auto& self,
                                 TNodeWithPeriod& node,
                                 WindowContext& context)
  {
    ImGui::Text("Period:");
    ImGui::SameLine();
    auto period = node.period();
    ImGui::PushID("period");
    self.render_series_node(period, context);
    ImGui::PopID();
    node.period(std::move(period));

    ImGui::Text("Source:");
    ImGui::SameLine();
    auto source = node.source();
    ImGui::PushID("source");
    self.render_series_node(source, context);
    ImGui::PopID();
    node.source(std::move(source));
  }

  template<typename TBinaryOpNode>
    requires std::same_as<TBinaryOpNode, AddNode> ||
             std::same_as<TBinaryOpNode, SubtractNode> ||
             std::same_as<TBinaryOpNode, MultiplyNode> ||
             std::same_as<TBinaryOpNode, DivideNode> ||
             std::same_as<TBinaryOpNode, AbsDiffNode>
  void render_series_node_params(this auto& self,
                                 TBinaryOpNode& node,
                                 WindowContext& context)
  {
    {
      ImGui::Text("Left:");
      ImGui::SameLine();
      auto left = node.left();
      ImGui::PushID("left");
      self.render_series_node(left, context);
      ImGui::PopID();
      node.left(std::move(left));
    }

    {
      ImGui::Text("Right:");
      ImGui::SameLine();
      auto right = node.right();
      ImGui::PushID("right");
      self.render_series_node(right, context);
      ImGui::PopID();
      node.right(std::move(right));
    }
  }

  template<typename TUnaryOpNode>
    requires std::same_as<TUnaryOpNode, NegateNode> ||
             std::same_as<TUnaryOpNode, SqrtNode>
  void render_series_node_params(this auto& self,
                                 TUnaryOpNode& node,
                                 WindowContext& context)
  {
    ImGui::Text("Value:");
    ImGui::SameLine();
    auto value = node.operand();
    ImGui::PushID("value");
    self.render_series_node(value, context);
    ImGui::PopID();
    node.operand(std::move(value));
  }

  void render_series_node_params(this auto& self,
                                 PercentageNode& node,
                                 WindowContext& context)
  {
    {
      ImGui::Text("Percent:");
      ImGui::SameLine();
      auto percent = node.percent();
      ImGui::PushID("percent");
      if(ImGui::InputDouble("##percent", &percent, 0.1, 1.0, "%.2f")) {
        node.percent(std::move(percent));
      }
      ImGui::PopID();
    }

    {
      ImGui::Text("Base:");
      ImGui::SameLine();
      auto base = node.base();
      ImGui::PushID("base");
      self.render_series_node(base, context);
      ImGui::PopID();
      node.base(std::move(base));
    }
  }

  void render_stop_target_value_node_params(this auto& self,
                                            auto& node,
                                            WindowContext& context,
                                            const char* label)
  {
    ImGui::Text("%s:", label);
    ImGui::SameLine();
    auto value = node.value();
    ImGui::PushID(label);
    self.render_series_node(value, context);
    ImGui::PopID();
    node.value(std::move(value));
  }

  void render_series_node_params(this auto& self,
                                 SlAmountNode& node,
                                 WindowContext& context)
  {
    self.render_stop_target_value_node_params(node, context, "Amount");
  }

  void render_series_node_params(this auto& self,
                                 TpAmountNode& node,
                                 WindowContext& context)
  {
    self.render_stop_target_value_node_params(node, context, "Amount");
  }

  void render_series_node_params(this auto& self,
                                 SlPercentNode& node,
                                 WindowContext& context)
  {
    self.render_stop_target_value_node_params(node, context, "Percent");
  }

  void render_series_node_params(this auto& self,
                                 TpPercentNode& node,
                                 WindowContext& context)
  {
    self.render_stop_target_value_node_params(node, context, "Percent");
  }

  void render_series_node_params(this auto& self,
                                 TpRMultipleNode& node,
                                 WindowContext& context)
  {
    self.render_stop_target_value_node_params(node, context, "Multiple");
  }

  void
  render_series_node_params(this auto&, InitialEntryPriceNode&, WindowContext&)
  {
  }

  void
  render_series_node_params(this auto&, LatestEntryPriceNode&, WindowContext&)
  {
  }

  void render_series_node_params(this auto&, AveragePriceNode&, WindowContext&)
  {
  }

  void
  render_series_node_params(this auto&, StopTargetRefPriceNode&, WindowContext&)
  {
  }

  void
  render_series_node_params(this auto&, PositionDirectionNode&, WindowContext&)
  {
  }

  void render_series_node_params(this auto& self,
                                 ChangeNode& node,
                                 WindowContext& context)
  {
    ImGui::Text("Source:");
    ImGui::SameLine();
    auto source = node.source();
    ImGui::PushID("source");
    self.render_series_node(source, context);
    ImGui::PopID();
    node.source(std::move(source));
  }

  void render_series_node_params(this auto& self,
                                 DataNode& node,
                                 WindowContext& context)
  {
    ImGui::Text("Field:");
    ImGui::SameLine();
    auto field = node.field();
    if(ImGui::InputText("##field", &field)) {
      node.field(field);
    }
  }

  void render_series_node_params(this auto& self,
                                 MacdNode& node,
                                 WindowContext& context)
  {
    {
      ImGui::Text("Fast Period:");
      ImGui::SameLine();
      auto fast_period = node.fast_period();
      ImGui::PushID("fast_period");
      self.render_series_node(fast_period, context);
      ImGui::PopID();
      node.fast_period(std::move(fast_period));
    }
    {
      ImGui::Text("Slow Period:");
      ImGui::SameLine();
      auto slow_period = node.slow_period();
      ImGui::PushID("slow_period");
      self.render_series_node(slow_period, context);
      ImGui::PopID();
      node.slow_period(std::move(slow_period));
    }
    {
      ImGui::Text("Signal Period:");
      ImGui::SameLine();
      auto signal_period = node.signal_period();
      ImGui::PushID("signal_period");
      self.render_series_node(signal_period, context);
      ImGui::PopID();
      node.signal_period(std::move(signal_period));
    }
    {
      ImGui::Text("Source:");
      ImGui::SameLine();
      auto source = node.source();
      ImGui::PushID("source");
      self.render_series_node(source, context);
      ImGui::PopID();
      node.source(std::move(source));
    }
  }

  void render_series_node_params(this auto& self,
                                 AtrNode& node,
                                 WindowContext& context)
  {
    ImGui::Text("Period:");
    ImGui::SameLine();
    auto period = node.period();
    ImGui::PushID("period");
    self.render_series_node(period, context);
    ImGui::PopID();
    node.period(std::move(period));

    ImGui::Text("Smoothing:");
    ImGui::SameLine();

    const auto ma_types = std::unordered_map<MaNodeType, std::string>{
     {MaNodeType::Sma, "SMA"},
     {MaNodeType::Ema, "EMA"},
     {MaNodeType::Wma, "WMA"},
     {MaNodeType::Hma, "HMA"},
     {MaNodeType::Rma, "RMA"},
    };

    const auto current_ma_title = ma_types.at(node.ma_smoothing_type());
    if(ImGui::BeginCombo("##atr_smoothing_type", current_ma_title.c_str())) {
      for(const auto& [ma_type, ma_title] : ma_types) {
        const bool is_selected = node.ma_smoothing_type() == ma_type;
        if(ImGui::Selectable(ma_title.c_str(), is_selected)) {
          node.ma_smoothing_type(ma_type);
        }
      }
      ImGui::EndCombo();
    }
  }

  void render_stop_target_atr_node_params(this auto& self,
                                          auto& node,
                                          WindowContext& context)
  {
    ImGui::Text("Period:");
    ImGui::SameLine();
    auto period = node.period();
    ImGui::PushID("period");
    self.render_series_node(period, context);
    ImGui::PopID();
    node.period(std::move(period));

    ImGui::Text("Multiplier:");
    ImGui::SameLine();
    auto multiplier = node.multiplier();
    ImGui::PushID("multiplier");
    self.render_series_node(multiplier, context);
    ImGui::PopID();
    node.multiplier(std::move(multiplier));

    ImGui::Text("Smoothing:");
    ImGui::SameLine();

    const auto ma_types = std::unordered_map<MaNodeType, std::string>{
     {MaNodeType::Sma, "SMA"},
     {MaNodeType::Ema, "EMA"},
     {MaNodeType::Wma, "WMA"},
     {MaNodeType::Hma, "HMA"},
     {MaNodeType::Rma, "RMA"},
    };

    const auto current_ma_title = ma_types.at(node.ma_smoothing_type());
    if(ImGui::BeginCombo("##atr_smoothing_type", current_ma_title.c_str())) {
      for(const auto& [ma_type, ma_title] : ma_types) {
        const bool is_selected = node.ma_smoothing_type() == ma_type;
        if(ImGui::Selectable(ma_title.c_str(), is_selected)) {
          node.ma_smoothing_type(ma_type);
        }
      }
      ImGui::EndCombo();
    }
  }

  void render_series_node_params(this auto& self,
                                 SlAtrNode& node,
                                 WindowContext& context)
  {
    self.render_stop_target_atr_node_params(node, context);
  }

  void render_series_node_params(this auto& self,
                                 TpAtrNode& node,
                                 WindowContext& context)
  {
    self.render_stop_target_atr_node_params(node, context);
  }

  void render_series_node_params(this auto& self,
                                 ValueNode& node,
                                 WindowContext& context)
  {
    ImGui::Text("Value:");
    ImGui::SameLine();
    auto value = node.value();
    if(ImGui::InputDouble("##value", &value, 0.1, 1.0, "%.2f")) {
      node.value(value);
    }
  }

  void render_series_node_params(this auto& self,
                                 LookbackNode& node,
                                 WindowContext& context)
  {
    ImGui::Text("Periods:");
    ImGui::SameLine();
    auto periods = static_cast<int>(node.period());
    if(ImGui::InputInt("##lookback_periods", &periods)) {
      if(periods < 1) {
        periods = 1;
      }
      node.period(static_cast<std::size_t>(periods));
    }
    {
      ImGui::Text("Source:");
      ImGui::SameLine();
      auto source = node.source();
      ImGui::PushID("source");
      self.render_series_node(source, context);
      ImGui::PopID();
      node.source(std::move(source));
    }
  }

  void render_series_node_params(this auto& self,
                                 NumericInputNode& node,
                                 WindowContext& context)
  {
    ImGui::Text("Label:");
    ImGui::SameLine();

    auto input_label = node.label();
    if(ImGui::InputText("##input_label", &input_label)) {
      node.label(std::move(input_label));
    }

    auto input_representation = node.representation();
    auto formatted_value = node.value();
    auto value_changed = false;

    constexpr std::array<const char*, 3> input_types{
     "Decimal", "Signed Integer", "Unsigned Integer"};
    auto selected_type_index = static_cast<int>(input_representation);

    ImGui::Text("Type:");
    ImGui::SameLine();
    if(ImGui::Combo("##input_type",
                    &selected_type_index,
                    input_types.data(),
                    static_cast<int>(input_types.size()))) {
      input_representation =
       static_cast<pludux::NumericInputNode::ValueRepresentation>(
        selected_type_index);
      node.representation(input_representation);
      value_changed = true;
    }

    ImGui::Text("Value:");
    ImGui::SameLine();
    if(input_representation ==
       pludux::NumericInputNode::ValueRepresentation::Decimal) {
      auto editable = formatted_value;
      if(ImGui::InputDouble("##input_value", &editable)) {
        formatted_value = editable;
        value_changed = true;
      }
    } else if(input_representation ==
              pludux::NumericInputNode::ValueRepresentation::SignedInteger) {
      auto editable = static_cast<std::int64_t>(formatted_value);
      if(ImGui::InputScalar("##input_value", ImGuiDataType_S64, &editable)) {
        formatted_value = static_cast<double>(editable);
        value_changed = true;
      }
    } else {
      auto editable = static_cast<std::uint64_t>(formatted_value);
      if(ImGui::InputScalar("##input_value", ImGuiDataType_U64, &editable)) {
        formatted_value = static_cast<double>(editable);
        value_changed = true;
      }
    }

    if(value_changed) {
      node.value(formatted_value);
    }
  }

  auto draw_condition_node_combo(this auto& self, const ErasedNode& condition)
   -> std::string
  {
    static const auto condition_ids = std::vector<std::string>{"EQUAL",
                                                               "NOT_EQUAL",
                                                               "GREATER_THAN",
                                                               "LESS_THAN",
                                                               "GREATER_EQUAL",
                                                               "LESS_EQUAL",
                                                               "CROSSOVER",
                                                               "CROSSUNDER",
                                                               "ALL_OF",
                                                               "ANY_OF",
                                                               "NOT",
                                                               "AND",
                                                               "OR",
                                                               "XOR",
                                                               "ALWAYS",
                                                               "NEVER"};

    auto result_condition_id = get_condition_node_id(condition);

    ImGui::PushID(result_condition_id.c_str());
    {
      const auto combo_preview_value =
       get_condition_node_title(result_condition_id);
      if(ImGui::BeginCombo("##Conditions", combo_preview_value.c_str())) {
        static auto filter = ImGuiTextFilter{};

        if(ImGui::IsWindowAppearing()) {
          ImGui::SetKeyboardFocusHere();
          filter.Clear();
        }

        filter.Draw("##Filter", -FLT_MIN);

        for(const auto& condition_id : condition_ids) {
          const bool is_selected = condition_id == result_condition_id;
          const auto condition_title = get_condition_node_title(condition_id);

          if(filter.PassFilter(condition_title.c_str())) {
            if(ImGui::Selectable(condition_title.c_str(), is_selected)) {
              result_condition_id = condition_id;
            }
          }
        }
        ImGui::EndCombo();
      }
    }

    ImGui::PopID();

    return result_condition_id;
  }

  auto make_condition_node_from_other(this auto& self,
                                      const std::string& condition_id,
                                      auto other_condition) -> ErasedNode
  {
    const auto get_condition_series_params =
     []<typename TNode>(
      const TNode& node) -> std::pair<ErasedNode, ErasedNode> {
      if constexpr(requires {
                     { node.source() } -> std::convertible_to<ErasedNode>;
                     { node.reference() } -> std::convertible_to<ErasedNode>;
                   }) {
        return {node.source(), node.reference()};
      } else if constexpr(requires {
                            {
                              node.target()
                            } -> std::convertible_to<ErasedNode>;
                            {
                              node.threshold()
                            } -> std::convertible_to<ErasedNode>;
                          }) {
        return {node.target(), node.threshold()};
      } else {
        return {CloseNode{}, CloseNode{}};
      }
    };

    if(condition_id == "EQUAL") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return EqualNode{std::move(lhs_series_param),
                       std::move(rhs_series_param)};
    }

    if(condition_id == "NOT_EQUAL") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return NotEqualNode{std::move(lhs_series_param),
                          std::move(rhs_series_param)};
    }

    if(condition_id == "GREATER_THAN") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return GreaterThanNode{std::move(lhs_series_param),
                             std::move(rhs_series_param)};
    }

    if(condition_id == "LESS_THAN") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return LessThanNode{std::move(lhs_series_param),
                          std::move(rhs_series_param)};
    }

    if(condition_id == "GREATER_EQUAL") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return GreaterEqualNode{std::move(lhs_series_param),
                              std::move(rhs_series_param)};
    }

    if(condition_id == "LESS_EQUAL") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return LessEqualNode{std::move(lhs_series_param),
                           std::move(rhs_series_param)};
    }

    if(condition_id == "CROSSOVER") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return CrossoverNode{std::move(lhs_series_param),
                           std::move(rhs_series_param)};
    }

    if(condition_id == "CROSSUNDER") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return CrossunderNode{std::move(lhs_series_param),
                            std::move(rhs_series_param)};
    }

    const auto get_conditions_param =
     []<typename TNode>(const TNode& node) -> std::vector<ErasedNode> {
      if constexpr(requires {
                     {
                       node.conditions()
                     } -> std::convertible_to<std::vector<ErasedNode>>;
                   }) {
        return node.conditions();
      } else if constexpr(requires {
                            {
                              node.other_condition()
                            } -> std::convertible_to<ErasedNode>;
                          }) {
        return {node.other_condition()};
      } else if constexpr(requires {
                            {
                              node.first_condition()
                            } -> std::convertible_to<ErasedNode>;
                            {
                              node.second_condition()
                            } -> std::convertible_to<ErasedNode>;
                          }) {
        return {node.first_condition(), node.second_condition()};
      } else {
        return {node};
      }
    };

    if(condition_id == "ALL_OF") {
      const auto conditions = get_conditions_param(other_condition);
      return AllOfNode{conditions};
    }

    if(condition_id == "ANY_OF") {
      const auto conditions = get_conditions_param(other_condition);
      return AnyOfNode{conditions};
    }

    const auto get_first_condition_param =
     []<typename TNode>(const TNode& node) -> ErasedNode {
      if constexpr(requires {
                     { node.condition() } -> std::convertible_to<ErasedNode>;
                   }) {
        return node.condition();
      } else if constexpr(requires {
                            {
                              node.first_condition()
                            } -> std::convertible_to<ErasedNode>;
                          }) {
        return node.first_condition();
      } else if constexpr(requires {
                            {
                              node.conditions()
                            } -> std::convertible_to<std::vector<ErasedNode>>;
                          }) {
        const auto conditions = node.conditions();
        if(!conditions.empty()) {
          return conditions.front();
        } else {
          return node;
        }
      } else if constexpr(requires {
                            {
                              node.other_condition()
                            } -> std::convertible_to<ErasedNode>;
                          }) {
        return node.other_condition();
      } else {
        return node;
      }
    };

    const auto get_second_condition_param =
     []<typename TNode>(const TNode& node) -> ErasedNode {
      if constexpr(requires {
                     {
                       node.other_condition()
                     } -> std::convertible_to<ErasedNode>;
                   }) {
        return node.other_condition();
      } else if constexpr(requires {
                            {
                              node.second_condition()
                            } -> std::convertible_to<ErasedNode>;
                          }) {
        return node.second_condition();
      } else {
        return node;
      }
    };

    if(condition_id == "NOT") {
      return LogicalNotNode{get_first_condition_param(other_condition)};
    }

    if(condition_id == "AND") {
      return LogicalAndNode{get_first_condition_param(other_condition),
                            get_second_condition_param(other_condition)};
    }

    if(condition_id == "OR") {
      return LogicalOrNode{get_first_condition_param(other_condition),
                           get_second_condition_param(other_condition)};
    }

    if(condition_id == "XOR") {
      return LogicalXorNode{get_first_condition_param(other_condition),
                            get_second_condition_param(other_condition)};
    }

    if(condition_id == "ALWAYS") {
      return TrueNode{};
    }

    if(condition_id == "NEVER") {
      return FalseNode{};
    }

    return FalseNode{};
  }

  auto render_condition_node(this auto& self,
                             const ErasedNode& any_condition,
                             WindowContext& context) -> ErasedNode
  {
    if(auto* condition_ptr = node_cast<AllOfNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<AnyOfNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<TrueNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<FalseNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<LessThanNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<GreaterThanNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<LessEqualNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<GreaterEqualNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<EqualNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<NotEqualNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<CrossoverNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<CrossunderNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<LogicalNotNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<LogicalAndNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<LogicalOrNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<LogicalXorNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else {
      return any_condition;
    }
  }

  template<typename TCondition>
    requires std::same_as<TCondition, EqualNode> ||
             std::same_as<TCondition, NotEqualNode> ||
             std::same_as<TCondition, GreaterThanNode> ||
             std::same_as<TCondition, LessThanNode> ||
             std::same_as<TCondition, GreaterEqualNode> ||
             std::same_as<TCondition, LessEqualNode>
  auto render_condition_node(this auto& self,
                             const TCondition& condition,
                             WindowContext& context) -> ErasedNode
  {
    auto new_condition = condition;

    auto target = new_condition.target();
    ImGui::PushID("left_param");
    self.render_series_node(target, context);
    ImGui::PopID();

    const auto updated_condition_id =
     self.draw_condition_node_combo(new_condition);

    auto threshold = new_condition.threshold();
    ImGui::PushID("right_param");
    self.render_series_node(threshold, context);
    ImGui::PopID();

    new_condition.target(std::move(target));
    new_condition.threshold(std::move(threshold));

    auto changed_node = self.make_condition_node_from_other(
     updated_condition_id, std::move(new_condition));

    return changed_node;
  }

  template<typename TCondition>
    requires std::same_as<TCondition, CrossoverNode> ||
             std::same_as<TCondition, CrossunderNode>
  auto render_condition_node(this auto& self,
                             const TCondition& condition,
                             WindowContext& context) -> ErasedNode
  {
    auto new_condition = condition;

    auto source = new_condition.source();
    ImGui::PushID("left_param");
    self.render_series_node(source, context);
    ImGui::PopID();

    const auto updated_condition_id =
     self.draw_condition_node_combo(new_condition);

    auto reference = new_condition.reference();
    ImGui::PushID("right_param");
    self.render_series_node(reference, context);
    ImGui::PopID();

    new_condition.source(std::move(source));
    new_condition.reference(std::move(reference));

    auto changed_node = self.make_condition_node_from_other(
     updated_condition_id, std::move(new_condition));

    return changed_node;
  }

  template<typename TCondition>
    requires std::same_as<TCondition, TrueNode> ||
             std::same_as<TCondition, FalseNode>
  auto render_condition_node(this auto& self,
                             const TCondition& condition,
                             WindowContext& context) -> ErasedNode
  {
    auto new_condition = condition;
    const auto updated_condition_id =
     self.draw_condition_node_combo(new_condition);

    auto changed_node = self.make_condition_node_from_other(
     updated_condition_id, std::move(new_condition));

    return changed_node;
  }

  template<typename TCondition>
    requires std::same_as<TCondition, AllOfNode> ||
             std::same_as<TCondition, AnyOfNode>
  auto render_condition_node(this auto& self,
                             const TCondition& condition,
                             WindowContext& context) -> ErasedNode
  {
    auto new_condition = condition;
    const auto updated_condition_id =
     self.draw_condition_node_combo(new_condition);
    ImGui::Indent();

    auto conditions = new_condition.conditions();
    for(auto i = 0; i < conditions.size(); ++i) {
      ImGui::PushID(i);
      auto& sub_condition = conditions[i];
      sub_condition = self.render_condition_node(sub_condition, context);

      const auto btn_label = "Remove Condition";
      {
        const auto spacing = ImGui::GetStyle().ItemSpacing.x;
        const auto frame_padding_x = ImGui::GetStyle().FramePadding.x;
        const auto delete_width =
         ImGui::CalcTextSize(btn_label).x + (2.0f * frame_padding_x);
        const auto line_start = ImGui::GetCursorScreenPos();
        const auto line_width = ImGui::GetContentRegionAvail().x;
        const auto delete_button_x = line_start.x + line_width - delete_width;
        ImGui::SetCursorScreenPos(ImVec2(delete_button_x, line_start.y));
      }
      if(ImGui::Button(btn_label)) {
        conditions.erase(conditions.begin() + i);
        --i;
      }

      ImGui::Separator();
      ImGui::PopID();
    }

    if(ImGui::Button("Add Condition")) {
      conditions.emplace_back(FalseNode{});
    }

    new_condition = TCondition(std::move(conditions));

    ImGui::Unindent();

    auto changed_node = self.make_condition_node_from_other(
     updated_condition_id, std::move(new_condition));

    return changed_node;
  }

  template<typename TCondition>
    requires std::same_as<TCondition, LogicalNotNode>
  auto render_condition_node(this auto& self,
                             const TCondition& condition,
                             WindowContext& context) -> ErasedNode
  {
    auto new_condition = condition;
    const auto updated_condition_id =
     self.draw_condition_node_combo(new_condition);
    ImGui::Indent();
    auto sub_condition = new_condition.other_condition();
    sub_condition = self.render_condition_node(sub_condition, context);
    new_condition.other_condition(std::move(sub_condition));
    ImGui::Unindent();
    auto changed_node = self.make_condition_node_from_other(
     updated_condition_id, std::move(new_condition));
    return changed_node;
  }

  template<typename TCondition>
    requires std::same_as<TCondition, LogicalAndNode> ||
             std::same_as<TCondition, LogicalOrNode> ||
             std::same_as<TCondition, LogicalXorNode>
  auto render_condition_node(this auto& self,
                             const TCondition& condition,
                             WindowContext& context) -> ErasedNode
  {
    auto new_condition = condition;
    auto first_condition = new_condition.first_condition();
    auto second_condition = new_condition.second_condition();

    {
      ImGui::PushID("first_condition");
      first_condition = self.render_condition_node(first_condition, context);
      ImGui::PopID();
    }

    ImGui::Separator();
    const auto updated_condition_id =
     self.draw_condition_node_combo(new_condition);
    ImGui::Separator();

    {
      ImGui::PushID("second_condition");
      second_condition = self.render_condition_node(second_condition, context);
      ImGui::PopID();
    }

    new_condition.first_condition(std::move(first_condition));
    new_condition.second_condition(std::move(second_condition));

    auto changed_node = self.make_condition_node_from_other(
     updated_condition_id, std::move(new_condition));
    return changed_node;
  }

  void render_plot_method(this auto& self,
                          AnyPlotMethod& plot_method,
                          WindowContext& context)
  {
    const auto plot_method_id = get_plot_method_id(plot_method);
    const auto combo_preview_value = get_plot_method_title(plot_method_id);
    if(ImGui::BeginCombo("##plot_method", combo_preview_value.c_str())) {
      static auto filter = ImGuiTextFilter{};

      if(ImGui::IsWindowAppearing()) {
        ImGui::SetKeyboardFocusHere();
        filter.Clear();
      }

      filter.Draw("##Filter", -FLT_MIN);

      for(const auto& plot_method_option : {"LINE", "HLINE", "HISTOGRAM"}) {
        const auto plot_method_title =
         get_plot_method_title(plot_method_option);
        const bool is_selected = plot_method_option == plot_method_id;

        if(filter.PassFilter(plot_method_title.c_str())) {
          if(ImGui::Selectable(plot_method_title.c_str(), is_selected)) {
            plot_method = get_default_plot_method(plot_method_option);
          }
        }
      }
      ImGui::EndCombo();
    }

    {
      ImGui::Indent();
      if(auto* method_ptr = plot_method_cast<LinePlotMethod>(plot_method)) {
        self.render_plot_method(*method_ptr, context);
      } else if(auto* method_ptr =
                 plot_method_cast<HLinePlotMethod>(plot_method)) {
        self.render_plot_method(*method_ptr, context);
      } else if(auto* method_ptr =
                 plot_method_cast<HistogramPlotMethod>(plot_method)) {
        self.render_plot_method(*method_ptr, context);
      }
      ImGui::Unindent();
    }
  }

  void render_plot_method(this auto& self,
                          HLinePlotMethod& plot_method,
                          WindowContext& context)
  {
    ImGui::Text("Level:");
    ImGui::SameLine();
    auto level = plot_method.level();
    if(ImGui::InputDouble("##hline_level", &level, 0.1, 1.0, "%.2f")) {
      plot_method.level(level);
    }

    ImGui::Text("Color:");
    ImGui::SameLine();
    const auto color_vec = ImGui::ColorConvertU32ToFloat4(plot_method.color());
    auto color = std::array{color_vec.x, color_vec.y, color_vec.z, color_vec.w};
    if(ImGui::ColorEdit4("##hline_color", color.data())) {
      plot_method.color(ImGui::ColorConvertFloat4ToU32(
       {color[0], color[1], color[2], color[3]}));
    }
  }

  template<typename TPlotMethod>
    requires std::same_as<TPlotMethod, LinePlotMethod> ||
             std::same_as<TPlotMethod, HistogramPlotMethod>
  void render_plot_method(this auto& self,
                          TPlotMethod& plot_method,
                          WindowContext& context)
  {
    static const auto source_ids =
     std::vector<std::string>{"SERIES_RESULT", "CONSTANT"};

    ImGui::Text("Source:");
    ImGui::SameLine();
    {
      auto plot_source_id = get_plot_source_method_id(plot_method.source());
      const auto combo_preview_value =
       get_plot_source_method_title(plot_source_id);
      if(ImGui::BeginCombo("##Sources", combo_preview_value.c_str())) {
        static auto filter = ImGuiTextFilter{};

        if(ImGui::IsWindowAppearing()) {
          ImGui::SetKeyboardFocusHere();
          filter.Clear();
        }

        filter.Draw("##Filter", -FLT_MIN);

        for(const auto& source_id : source_ids) {
          const auto source_title = get_plot_source_method_title(source_id);
          const bool is_selected = source_id == plot_source_id;

          if(filter.PassFilter(source_title.c_str())) {
            if(ImGui::Selectable(source_title.c_str(), is_selected)) {
              if(source_id == "SERIES_RESULT" &&
                 self.available_series_names_.empty()) {
                const auto source_method_title =
                 get_plot_source_method_title(source_id);
                const auto error_message =
                 std::format("Cannot select '{}' when there are no available "
                             "series other than the current one.",
                             source_method_title);
                context.alert(error_message);
              } else {
                auto source = get_default_plot_source_method(source_id);

                if(source_id == "SERIES_RESULT") {
                  if(auto* series_source_ptr =
                      plot_source_method_cast<SeriesPlotSourceMethod>(source)) {
                    const auto first_available_series_name =
                     self.available_series_names_.front();
                    series_source_ptr->series_name(first_available_series_name);
                  }
                }

                plot_method.source(std::move(source));
              }
            }
          }
        }
        ImGui::EndCombo();
      }
    }

    ImGui::Indent();
    auto source = plot_method.source();
    self.render_plot_source_method(source, context);
    plot_method.source(std::move(source));
    ImGui::Unindent();

    ImGui::Text("Color:");
    ImGui::SameLine();
    const auto color_vec = ImGui::ColorConvertU32ToFloat4(plot_method.color());
    auto color = std::array{color_vec.x, color_vec.y, color_vec.z, color_vec.w};
    if(ImGui::ColorEdit4("##line_color", color.data())) {
      plot_method.color(ImGui::ColorConvertFloat4ToU32(
       {color[0], color[1], color[2], color[3]}));
    }
  }

  void render_plot_source_method(this auto& self,
                                 SeriesPlotSourceMethod& plot_source_method,
                                 WindowContext& context)
  {
    if(self.changed_series_names_.contains(plot_source_method.series_name())) {
      plot_source_method.series_name(
       self.changed_series_names_.at(plot_source_method.series_name()));
    } else if(std::ranges::find(self.available_series_names_,
                                plot_source_method.series_name()) ==
              self.available_series_names_.end()) {
      plot_source_method.series_name("");
    }

    ImGui::Text("Series:");
    ImGui::SameLine();
    auto series_name = plot_source_method.series_name();
    if(ImGui::BeginCombo("##series_names", series_name.c_str())) {
      for(const auto& available_series_name : self.available_series_names_) {
        const bool is_selected = series_name == available_series_name;
        if(ImGui::Selectable(available_series_name.c_str(), is_selected)) {
          series_name = available_series_name;
        }
      }
      ImGui::EndCombo();
    }

    plot_source_method.series_name(series_name);
  }

  void render_plot_source_method(this auto& self,
                                 ConstantPlotSourceMethod& plot_source_method,
                                 WindowContext& context)
  {
    ImGui::Text("Value:");
    ImGui::SameLine();
    auto value = plot_source_method.value();
    if(ImGui::InputDouble("##constant_value", &value, 0.1, 1.0, "%.2f")) {
      plot_source_method.value(value);
    }
  }

  void render_plot_source_method(this auto& self,
                                 AnyPlotSourceMethod& plot_source_method,
                                 WindowContext& context)
  {
    if(auto* method_ptr =
        plot_source_method_cast<SeriesPlotSourceMethod>(plot_source_method)) {
      self.render_plot_source_method(*method_ptr, context);
    } else if(auto* method_ptr =
               plot_source_method_cast<ConstantPlotSourceMethod>(
                plot_source_method)) {
      self.render_plot_source_method(*method_ptr, context);
    } else {
      self.render_plot_source_method(*method_ptr, context);
    }
  }

  void submit_strategy_changes(this auto& self, WindowContext& context)
  {
    context.push_action(
     [strategy_handle_opt = self.selected_strategy_handle_opt_,
      edit_strategy_ptr =
       self.editing_strategy_ptr_](ApplicationState& app_state) {
       if(edit_strategy_ptr->name().empty()) {
         edit_strategy_ptr->name("Unnamed");
       }

       if(!strategy_handle_opt) {
         app_state.add_strategy(std::move(*edit_strategy_ptr));
         return;
       }

       app_state.update_strategy(strategy_handle_opt.value(),
                                 *edit_strategy_ptr);
     });
  }

  void reset(this auto& self)
  {
    self.current_page_ = Page::List;
    self.selected_strategy_handle_opt_ = std::nullopt;
    self.editing_strategy_ptr_ = nullptr;
    self.available_series_names_.clear();
    self.changed_series_names_.clear();
  }
};

} // namespace pludux::apps
