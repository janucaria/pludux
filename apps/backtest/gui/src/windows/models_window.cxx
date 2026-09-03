module;
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <optional>
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

#include "../ui/pludux_icons.hpp"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

export module pludux.apps.backtest:windows.models_window;

import pludux.backtest;
import :built_in_strategies;
import :window_context;
import :ui.widgets;

namespace pludux::apps {

using pludux::backtest::AveragePriceNode;
using pludux::backtest::DrawdownAdjustedQuantityLimitNode;
using pludux::backtest::DrawdownAdjustedQuantityNode;
using pludux::backtest::DrawdownNode;
using pludux::backtest::EquityNode;
using pludux::backtest::EquityPercentNode;
using pludux::backtest::ErasedPlotMethod;
using pludux::backtest::ErasedPlotMethodContext;
using pludux::backtest::ErasedPlotSourceMethod;
using pludux::backtest::EstimatedEntryFeeNode;
using pludux::backtest::EstimatedOneRExitFeeNode;
using pludux::backtest::FrozenUnitQuantityNode;
using pludux::backtest::HLinePlotMethod;
using pludux::backtest::InitialEntryPriceNode;
using pludux::backtest::IsPyramidingOrderNode;
using pludux::backtest::LatestEntryPriceNode;
using pludux::backtest::PositionDirectionNode;
using pludux::backtest::PositionRMultipleNode;
using pludux::backtest::PyramidingLayerNode;
using pludux::backtest::RawRequestedQuantityLimitNode;
using pludux::backtest::RawRequestedQuantityNode;
using pludux::backtest::RequestedCostNode;
using pludux::backtest::RequestedNotionalNode;
using pludux::backtest::RequestedOrderDirectionNode;
using pludux::backtest::RequestedOrderPriceNode;
using pludux::backtest::RequestedOrderRiskDistanceNode;
using pludux::backtest::RequestedPriceRiskNode;
using pludux::backtest::RequestedQuantityNode;
using pludux::backtest::RequestedRiskWithFeesNode;
using pludux::backtest::RiskDistanceAmountNode;
using pludux::backtest::RiskDistanceAtrNode;
using pludux::backtest::RiskDistancePercentNode;
using pludux::backtest::Sl1RNode;
using pludux::backtest::SlAmountNode;
using pludux::backtest::SlAtrNode;
using pludux::backtest::SlPercentNode;
using pludux::backtest::SlRMultipleNode;
using pludux::backtest::StopTargetRefPriceNode;
using pludux::backtest::TpAmountNode;
using pludux::backtest::TpAtrNode;
using pludux::backtest::TpPercentNode;
using pludux::backtest::TpRMultipleNode;
using LinePlotMethod = pludux::backtest::LinePlotMethod<
 ErasedPlotSourceMethod<ErasedPlotMethodContext>>;
using HistogramPlotMethod = pludux::backtest::HistogramPlotMethod<
 ErasedPlotSourceMethod<ErasedPlotMethodContext>>;
using MomentumHistogramPlotMethod =
 pludux::backtest::MomentumHistogramPlotMethod<
  ErasedPlotSourceMethod<ErasedPlotMethodContext>>;
using pludux::backtest::ConstantPlotSourceMethod;
using pludux::backtest::SeriesPlotSourceMethod;

auto get_default_series_node(const std::string& series_id)
 -> ErasedNode<backtest::BacktestMethodContext>
{
  if(series_id == "MARKET_DATA.OPEN") {
    return OpenNode{};
  } else if(series_id == "MARKET_DATA.CLOSE") {
    return CloseNode{};
  } else if(series_id == "MARKET_DATA.HIGH") {
    return HighNode{};
  } else if(series_id == "MARKET_DATA.LOW") {
    return LowNode{};
  } else if(series_id == "MARKET_DATA.VOLUME") {
    return VolumeNode{};
  } else if(series_id == "OPERATOR.CHANGE") {
    return ChangeNode<backtest::BacktestMethodContext>{CloseNode{}};
  } else if(series_id == "MARKET_DATA.FIELD") {
    return DataNode{};
  } else if(series_id == "PORTFOLIO.EQUITY") {
    return EquityNode{};
  } else if(series_id == "PORTFOLIO.EQUITY_PERCENT") {
    return EquityPercentNode{};
  } else if(series_id == "PORTFOLIO.DRAWDOWN") {
    return DrawdownNode{};
  } else if(series_id == "INDICATOR.SMA") {
    return SmaNode<backtest::BacktestMethodContext>{CloseNode{}, 14};
  } else if(series_id == "INDICATOR.EMA") {
    return EmaNode<backtest::BacktestMethodContext>{CloseNode{}, 14};
  } else if(series_id == "INDICATOR.WMA") {
    return WmaNode<backtest::BacktestMethodContext>{CloseNode{}, 14};
  } else if(series_id == "INDICATOR.HMA") {
    return HmaNode<backtest::BacktestMethodContext>{CloseNode{}, 14};
  } else if(series_id == "INDICATOR.RMA") {
    return RmaNode<backtest::BacktestMethodContext>{CloseNode{}, 14};
  } else if(series_id == "INDICATOR.RSI") {
    return RsiNode<backtest::BacktestMethodContext>{CloseNode{}, 14};
  } else if(series_id == "INDICATOR.ROC") {
    return RocNode<backtest::BacktestMethodContext>{CloseNode{}, 14};
  } else if(series_id == "INDICATOR.RVOL") {
    return RvolNode<backtest::BacktestMethodContext>{14};
  } else if(series_id == "INDICATOR.HIGHEST") {
    return HighestNode<backtest::BacktestMethodContext>{CloseNode{}, 14};
  } else if(series_id == "INDICATOR.LOWEST") {
    return LowestNode<backtest::BacktestMethodContext>{CloseNode{}, 14};
  } else if(series_id == "INDICATOR.TR") {
    return TrNode<backtest::BacktestMethodContext>{};
  } else if(series_id == "INDICATOR.MACD") {
    return MacdNode<backtest::BacktestMethodContext>{CloseNode{}, 12, 26, 9};
  } else if(series_id == "INDICATOR.ATR") {
    return AtrNode<backtest::BacktestMethodContext>{14};
  } else if(series_id == "INDICATOR.STDDEV") {
    return StddevNode<backtest::BacktestMethodContext>{CloseNode{}, 14};
  } else if(series_id == "INDICATOR.BB") {
    return BbNode<backtest::BacktestMethodContext>{
     CloseNode{}, 20, 2.0, MaNodeType::Sma};
  } else if(series_id == "INDICATOR.KC") {
    return KcNode<backtest::BacktestMethodContext>{
     CloseNode{}, 20, 1.5, 14, KcBandNodeType::Atr, MaNodeType::Ema};
  } else if(series_id == "INDICATOR.DC") {
    return DonchianChannelNode<backtest::BacktestMethodContext>{};
  } else if(series_id == "INDICATOR.STOCH") {
    return StochNode<backtest::BacktestMethodContext>{14, 3, 3};
  } else if(series_id == "INDICATOR.STOCH_RSI") {
    return StochRsiNode<backtest::BacktestMethodContext>{
     CloseNode{}, 14, 14, 3, 3};
  } else if(series_id == "SERIES.REFERENCE") {
    return SeriesNode{""};
  } else if(series_id == "VALUE.CONSTANT") {
    return ValueNode{0.0};
  } else if(series_id == "OPERATOR.LOOKBACK") {
    return LookbackNode<backtest::BacktestMethodContext>{CloseNode{}, 1};
  } else if(series_id == "LOGIC.ALL_OF") {
    return AllOfNode<backtest::BacktestMethodContext>{};
  } else if(series_id == "LOGIC.ANY_OF") {
    return AnyOfNode<backtest::BacktestMethodContext>{};
  } else if(series_id == "LOGIC.ALWAYS") {
    return TrueNode{};
  } else if(series_id == "LOGIC.NEVER") {
    return FalseNode{};
  } else if(series_id == "COMPARISON.LESS_THAN") {
    return LessThanNode<backtest::BacktestMethodContext>{CloseNode{},
                                                         CloseNode{}};
  } else if(series_id == "COMPARISON.GREATER_THAN") {
    return GreaterThanNode<backtest::BacktestMethodContext>{CloseNode{},
                                                            CloseNode{}};
  } else if(series_id == "COMPARISON.LESS_EQUAL") {
    return LessEqualNode<backtest::BacktestMethodContext>{CloseNode{},
                                                          CloseNode{}};
  } else if(series_id == "COMPARISON.GREATER_EQUAL") {
    return GreaterEqualNode<backtest::BacktestMethodContext>{CloseNode{},
                                                             CloseNode{}};
  } else if(series_id == "COMPARISON.EQUAL") {
    return EqualNode<backtest::BacktestMethodContext>{CloseNode{}, CloseNode{}};
  } else if(series_id == "COMPARISON.NOT_EQUAL") {
    return NotEqualNode<backtest::BacktestMethodContext>{CloseNode{},
                                                         CloseNode{}};
  } else if(series_id == "COMPARISON.CROSSOVER") {
    return CrossoverNode<backtest::BacktestMethodContext>{CloseNode{},
                                                          CloseNode{}};
  } else if(series_id == "COMPARISON.CROSSUNDER") {
    return CrossunderNode<backtest::BacktestMethodContext>{CloseNode{},
                                                           CloseNode{}};
  } else if(series_id == "LOGIC.NOT") {
    return LogicalNotNode<backtest::BacktestMethodContext>{FalseNode{}};
  } else if(series_id == "LOGIC.AND") {
    return LogicalAndNode<backtest::BacktestMethodContext>{FalseNode{},
                                                           FalseNode{}};
  } else if(series_id == "LOGIC.OR") {
    return LogicalOrNode<backtest::BacktestMethodContext>{FalseNode{},
                                                          FalseNode{}};
  } else if(series_id == "LOGIC.XOR") {
    return LogicalXorNode<backtest::BacktestMethodContext>{FalseNode{},
                                                           FalseNode{}};
  } else if(series_id == "INPUT.NUMERIC") {
    return NumericInputNode{"Input"};
  } else if(series_id == "OPERATOR.ADD") {
    return AddNode<backtest::BacktestMethodContext>{CloseNode{}, CloseNode{}};
  } else if(series_id == "OPERATOR.SUBTRACT") {
    return SubtractNode<backtest::BacktestMethodContext>{CloseNode{},
                                                         CloseNode{}};
  } else if(series_id == "OPERATOR.MULTIPLY") {
    return MultiplyNode<backtest::BacktestMethodContext>{CloseNode{},
                                                         CloseNode{}};
  } else if(series_id == "OPERATOR.DIVIDE") {
    return DivideNode<backtest::BacktestMethodContext>{CloseNode{},
                                                       CloseNode{}};
  } else if(series_id == "OPERATOR.NEGATE") {
    return NegateNode<backtest::BacktestMethodContext>{CloseNode{}};
  } else if(series_id == "OPERATOR.SQRT") {
    return SqrtNode<backtest::BacktestMethodContext>{CloseNode{}};
  } else if(series_id == "OPERATOR.PERCENTAGE") {
    return PercentageNode<backtest::BacktestMethodContext>{CloseNode{}, 100.0};
  } else if(series_id == "RISK_DISTANCE.AMOUNT") {
    return RiskDistanceAmountNode<backtest::BacktestMethodContext>{1000.0};
  } else if(series_id == "RISK_DISTANCE.PERCENT") {
    return RiskDistancePercentNode<backtest::BacktestMethodContext>{1.0};
  } else if(series_id == "RISK_DISTANCE.ATR") {
    return RiskDistanceAtrNode<backtest::BacktestMethodContext>{14.0, 2.0};
  } else if(series_id == "STOP_LOSS.AMOUNT") {
    return SlAmountNode<backtest::BacktestMethodContext>{1000.0};
  } else if(series_id == "TAKE_PROFIT.AMOUNT") {
    return TpAmountNode<backtest::BacktestMethodContext>{2000.0};
  } else if(series_id == "STOP_LOSS.PERCENT") {
    return SlPercentNode<backtest::BacktestMethodContext>{10.0};
  } else if(series_id == "TAKE_PROFIT.PERCENT") {
    return TpPercentNode<backtest::BacktestMethodContext>{20.0};
  } else if(series_id == "STOP_LOSS.ATR") {
    return SlAtrNode<backtest::BacktestMethodContext>{14.0, 2.0};
  } else if(series_id == "TAKE_PROFIT.ATR") {
    return TpAtrNode<backtest::BacktestMethodContext>{14.0, 4.0};
  } else if(series_id == "STOP_LOSS.ONE_R") {
    return Sl1RNode{};
  } else if(series_id == "STOP_LOSS.R_MULTIPLE") {
    return SlRMultipleNode<backtest::BacktestMethodContext>{1.0};
  } else if(series_id == "TAKE_PROFIT.R_MULTIPLE") {
    return TpRMultipleNode<backtest::BacktestMethodContext>{2.0};
  } else if(series_id == "POSITION.INITIAL_ENTRY_PRICE") {
    return InitialEntryPriceNode{};
  } else if(series_id == "POSITION.LATEST_ENTRY_PRICE") {
    return LatestEntryPriceNode{};
  } else if(series_id == "POSITION.AVERAGE_PRICE") {
    return AveragePriceNode{};
  } else if(series_id == "POSITION.STOP_TARGET_REF_PRICE") {
    return StopTargetRefPriceNode{};
  } else if(series_id == "POSITION.DIRECTION") {
    return PositionDirectionNode{};
  } else if(series_id == "POSITION.PYRAMIDING_LAYER") {
    return PyramidingLayerNode{};
  } else if(series_id == "POSITION.R_MULTIPLE") {
    return PositionRMultipleNode<backtest::BacktestMethodContext>{};
  } else if(series_id == "OPERATOR.ABS_DIFF") {
    return AbsDiffNode<backtest::BacktestMethodContext>{CloseNode{},
                                                        CloseNode{}};
  } else if(series_id == "OPERATOR.SELECT_OUTPUT") {
    return SelectOutputNode<backtest::BacktestMethodContext>{
     CloseNode{}, NodeOutput::MiddleBand};
  }

  throw std::invalid_argument{
   std::format("Unknown series node id: {}", series_id)};
}

auto get_default_portfolio_comparator_node(const std::string& id)
 -> ErasedNode<backtest::RequestedOrderMethodContext>
{
  if(id == "VALUE.CONSTANT") {
    return ValueNode{0.0};
  } else if(id == "MARKET_DATA.OPEN") {
    return OpenNode{};
  } else if(id == "MARKET_DATA.HIGH") {
    return HighNode{};
  } else if(id == "MARKET_DATA.LOW") {
    return LowNode{};
  } else if(id == "MARKET_DATA.CLOSE") {
    return CloseNode{};
  } else if(id == "MARKET_DATA.VOLUME") {
    return VolumeNode{};
  } else if(id == "MARKET_DATA.FIELD") {
    return DataNode{};
  } else if(id == "OPERATOR.LOOKBACK") {
    return LookbackNode<backtest::RequestedOrderMethodContext>{CloseNode{}, 1};
  } else if(id == "REQUESTED_ORDER.PRICE") {
    return RequestedOrderPriceNode{};
  } else if(id == "REQUESTED_ORDER.DIRECTION") {
    return RequestedOrderDirectionNode{};
  } else if(id == "REQUESTED_ORDER.IS_PYRAMIDING") {
    return IsPyramidingOrderNode{};
  } else if(id == "REQUESTED_ORDER.RAW_QUANTITY") {
    return RawRequestedQuantityNode{};
  } else if(id == "REQUESTED_ORDER.RAW_QUANTITY_LIMIT") {
    return RawRequestedQuantityLimitNode{};
  } else if(id == "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY") {
    return DrawdownAdjustedQuantityNode{};
  } else if(id == "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY_LIMIT") {
    return DrawdownAdjustedQuantityLimitNode{};
  } else if(id == "REQUESTED_ORDER.QUANTITY") {
    return RequestedQuantityNode{};
  } else if(id == "REQUESTED_ORDER.NOTIONAL") {
    return RequestedNotionalNode{};
  } else if(id == "REQUESTED_ORDER.COST") {
    return RequestedCostNode{};
  } else if(id == "REQUESTED_ORDER.ESTIMATED_ENTRY_FEE") {
    return EstimatedEntryFeeNode{};
  } else if(id == "REQUESTED_ORDER.ESTIMATED_1R_EXIT_FEE") {
    return EstimatedOneRExitFeeNode{};
  } else if(id == "REQUESTED_ORDER.RISK_DISTANCE") {
    return RequestedOrderRiskDistanceNode{};
  } else if(id == "REQUESTED_ORDER.PRICE_RISK") {
    return RequestedPriceRiskNode{};
  } else if(id == "REQUESTED_ORDER.RISK_WITH_FEES") {
    return RequestedRiskWithFeesNode{};
  } else if(id == "REQUESTED_ORDER.FROZEN_UNIT_QUANTITY") {
    return FrozenUnitQuantityNode{};
  }

  const auto value =
   ErasedNode<backtest::RequestedOrderMethodContext>{ValueNode{0.0}};
  const auto requested =
   ErasedNode<backtest::RequestedOrderMethodContext>{RequestedQuantityNode{}};
  if(id == "OPERATOR.ADD") {
    return AddNode<backtest::RequestedOrderMethodContext>{requested, value};
  } else if(id == "OPERATOR.SUBTRACT") {
    return SubtractNode<backtest::RequestedOrderMethodContext>{requested,
                                                               value};
  } else if(id == "OPERATOR.MULTIPLY") {
    return MultiplyNode<backtest::RequestedOrderMethodContext>{requested,
                                                               ValueNode{1.0}};
  } else if(id == "OPERATOR.DIVIDE") {
    return DivideNode<backtest::RequestedOrderMethodContext>{requested,
                                                             ValueNode{1.0}};
  } else if(id == "OPERATOR.ABS_DIFF") {
    return AbsDiffNode<backtest::RequestedOrderMethodContext>{requested, value};
  } else if(id == "OPERATOR.MAX") {
    return MaxNode<backtest::RequestedOrderMethodContext>{requested, value};
  } else if(id == "OPERATOR.MIN") {
    return MinNode<backtest::RequestedOrderMethodContext>{requested, value};
  } else if(id == "OPERATOR.NEGATE") {
    return NegateNode<backtest::RequestedOrderMethodContext>{requested};
  } else if(id == "OPERATOR.ABS") {
    return AbsNode<backtest::RequestedOrderMethodContext>{requested};
  } else if(id == "OPERATOR.SQRT") {
    return SqrtNode<backtest::RequestedOrderMethodContext>{requested};
  } else if(id == "OPERATOR.POSITIVE_PART") {
    return PositivePartNode<backtest::RequestedOrderMethodContext>{requested};
  } else if(id == "OPERATOR.NEGATIVE_PART") {
    return NegativePartNode<backtest::RequestedOrderMethodContext>{requested};
  }
  throw std::invalid_argument{
   std::format("Unknown requested order node id: {}", id)};
}

auto get_portfolio_comparator_node_id(const backtest::ComparatorNode& node)
 -> std::string
{
  auto parser = backtest::make_requested_order_comparator_config_parser();
  return backtest::config_string(parser.serialize_node(node).at("method"));
}

auto get_series_node_id(const ErasedNode<backtest::BacktestMethodContext>& node)
 -> std::string
{
  if(node_cast<SelectOutputNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.SELECT_OUTPUT";
  } else if(node_cast<SeriesNode>(node)) {
    return "SERIES.REFERENCE";
  } else if(node_cast<CloseNode>(node)) {
    return "MARKET_DATA.CLOSE";
  } else if(node_cast<OpenNode>(node)) {
    return "MARKET_DATA.OPEN";
  } else if(node_cast<HighNode>(node)) {
    return "MARKET_DATA.HIGH";
  } else if(node_cast<LowNode>(node)) {
    return "MARKET_DATA.LOW";
  } else if(node_cast<VolumeNode>(node)) {
    return "MARKET_DATA.VOLUME";
  } else if(node_cast<ChangeNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.CHANGE";
  } else if(node_cast<RequestedOrderPriceNode>(node)) {
    return "REQUESTED_ORDER.PRICE";
  } else if(node_cast<RequestedOrderDirectionNode>(node)) {
    return "REQUESTED_ORDER.DIRECTION";
  } else if(node_cast<IsPyramidingOrderNode>(node)) {
    return "REQUESTED_ORDER.IS_PYRAMIDING";
  } else if(node_cast<RawRequestedQuantityNode>(node)) {
    return "REQUESTED_ORDER.RAW_QUANTITY";
  } else if(node_cast<RawRequestedQuantityLimitNode>(node)) {
    return "REQUESTED_ORDER.RAW_QUANTITY_LIMIT";
  } else if(node_cast<DrawdownAdjustedQuantityNode>(node)) {
    return "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY";
  } else if(node_cast<DrawdownAdjustedQuantityLimitNode>(node)) {
    return "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY_LIMIT";
  } else if(node_cast<RequestedQuantityNode>(node)) {
    return "REQUESTED_ORDER.QUANTITY";
  } else if(node_cast<RequestedNotionalNode>(node)) {
    return "REQUESTED_ORDER.NOTIONAL";
  } else if(node_cast<RequestedCostNode>(node)) {
    return "REQUESTED_ORDER.COST";
  } else if(node_cast<EstimatedEntryFeeNode>(node)) {
    return "REQUESTED_ORDER.ESTIMATED_ENTRY_FEE";
  } else if(node_cast<EstimatedOneRExitFeeNode>(node)) {
    return "REQUESTED_ORDER.ESTIMATED_1R_EXIT_FEE";
  } else if(node_cast<RequestedOrderRiskDistanceNode>(node)) {
    return "REQUESTED_ORDER.RISK_DISTANCE";
  } else if(node_cast<RequestedPriceRiskNode>(node)) {
    return "REQUESTED_ORDER.PRICE_RISK";
  } else if(node_cast<RequestedRiskWithFeesNode>(node)) {
    return "REQUESTED_ORDER.RISK_WITH_FEES";
  } else if(node_cast<FrozenUnitQuantityNode>(node)) {
    return "REQUESTED_ORDER.FROZEN_UNIT_QUANTITY";
  } else if(node_cast<DataNode>(node)) {
    return "MARKET_DATA.FIELD";
  } else if(node_cast<EquityNode>(node)) {
    return "PORTFOLIO.EQUITY";
  } else if(node_cast<EquityPercentNode>(node)) {
    return "PORTFOLIO.EQUITY_PERCENT";
  } else if(node_cast<DrawdownNode>(node)) {
    return "PORTFOLIO.DRAWDOWN";
  } else if(node_cast<ValueNode>(node)) {
    return "VALUE.CONSTANT";
  } else if(node_cast<StddevNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.STDDEV";
  } else if(node_cast<BbNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.BB";
  } else if(node_cast<KcNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.KC";
  } else if(node_cast<DonchianChannelNode<backtest::BacktestMethodContext>>(
             node)) {
    return "INDICATOR.DC";
  } else if(node_cast<SmaNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.SMA";
  } else if(node_cast<EmaNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.EMA";
  } else if(node_cast<WmaNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.WMA";
  } else if(node_cast<HmaNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.HMA";
  } else if(node_cast<RmaNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.RMA";
  } else if(node_cast<RsiNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.RSI";
  } else if(node_cast<RocNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.ROC";
  } else if(node_cast<RvolNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.RVOL";
  } else if(node_cast<HighestNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.HIGHEST";
  } else if(node_cast<LowestNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.LOWEST";
  } else if(node_cast<TrNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.TR";
  } else if(node_cast<MacdNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.MACD";
  } else if(node_cast<AtrNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.ATR";
  } else if(node_cast<StochNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.STOCH";
  } else if(node_cast<StochRsiNode<backtest::BacktestMethodContext>>(node)) {
    return "INDICATOR.STOCH_RSI";
  } else if(node_cast<AddNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.ADD";
  } else if(node_cast<SubtractNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.SUBTRACT";
  } else if(node_cast<MultiplyNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.MULTIPLY";
  } else if(node_cast<DivideNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.DIVIDE";
  } else if(node_cast<NegateNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.NEGATE";
  } else if(node_cast<SqrtNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.SQRT";
  } else if(node_cast<PercentageNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.PERCENTAGE";
  } else if(node_cast<RiskDistanceAmountNode<backtest::BacktestMethodContext>>(
             node)) {
    return "RISK_DISTANCE.AMOUNT";
  } else if(node_cast<RiskDistancePercentNode<backtest::BacktestMethodContext>>(
             node)) {
    return "RISK_DISTANCE.PERCENT";
  } else if(node_cast<RiskDistanceAtrNode<backtest::BacktestMethodContext>>(
             node)) {
    return "RISK_DISTANCE.ATR";
  } else if(node_cast<SlAmountNode<backtest::BacktestMethodContext>>(node)) {
    return "STOP_LOSS.AMOUNT";
  } else if(node_cast<TpAmountNode<backtest::BacktestMethodContext>>(node)) {
    return "TAKE_PROFIT.AMOUNT";
  } else if(node_cast<SlPercentNode<backtest::BacktestMethodContext>>(node)) {
    return "STOP_LOSS.PERCENT";
  } else if(node_cast<TpPercentNode<backtest::BacktestMethodContext>>(node)) {
    return "TAKE_PROFIT.PERCENT";
  } else if(node_cast<SlAtrNode<backtest::BacktestMethodContext>>(node)) {
    return "STOP_LOSS.ATR";
  } else if(node_cast<TpAtrNode<backtest::BacktestMethodContext>>(node)) {
    return "TAKE_PROFIT.ATR";
  } else if(node_cast<Sl1RNode>(node)) {
    return "STOP_LOSS.ONE_R";
  } else if(node_cast<SlRMultipleNode<backtest::BacktestMethodContext>>(node)) {
    return "STOP_LOSS.R_MULTIPLE";
  } else if(node_cast<TpRMultipleNode<backtest::BacktestMethodContext>>(node)) {
    return "TAKE_PROFIT.R_MULTIPLE";
  } else if(node_cast<InitialEntryPriceNode>(node)) {
    return "POSITION.INITIAL_ENTRY_PRICE";
  } else if(node_cast<LatestEntryPriceNode>(node)) {
    return "POSITION.LATEST_ENTRY_PRICE";
  } else if(node_cast<AveragePriceNode>(node)) {
    return "POSITION.AVERAGE_PRICE";
  } else if(node_cast<StopTargetRefPriceNode>(node)) {
    return "POSITION.STOP_TARGET_REF_PRICE";
  } else if(node_cast<PositionDirectionNode>(node)) {
    return "POSITION.DIRECTION";
  } else if(node_cast<PyramidingLayerNode>(node)) {
    return "POSITION.PYRAMIDING_LAYER";
  } else if(node_cast<PositionRMultipleNode<backtest::BacktestMethodContext>>(
             node)) {
    return "POSITION.R_MULTIPLE";
  } else if(node_cast<AbsDiffNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.ABS_DIFF";
  } else if(node_cast<AbsNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.ABS";
  } else if(node_cast<MaxNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.MAX";
  } else if(node_cast<MinNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.MIN";
  } else if(node_cast<PositivePartNode<backtest::BacktestMethodContext>>(
             node)) {
    return "OPERATOR.POSITIVE_PART";
  } else if(node_cast<NegativePartNode<backtest::BacktestMethodContext>>(
             node)) {
    return "OPERATOR.NEGATIVE_PART";
  } else if(node_cast<LookbackNode<backtest::BacktestMethodContext>>(node)) {
    return "OPERATOR.LOOKBACK";
  } else if(node_cast<NumericInputNode>(node)) {
    return "INPUT.NUMERIC";
  } else if(node_cast<AllOfNode<backtest::BacktestMethodContext>>(node)) {
    return "LOGIC.ALL_OF";
  } else if(node_cast<AnyOfNode<backtest::BacktestMethodContext>>(node)) {
    return "LOGIC.ANY_OF";
  } else if(node_cast<TrueNode>(node)) {
    return "LOGIC.ALWAYS";
  } else if(node_cast<FalseNode>(node)) {
    return "LOGIC.NEVER";
  } else if(node_cast<LessThanNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.LESS_THAN";
  } else if(node_cast<GreaterThanNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.GREATER_THAN";
  } else if(node_cast<LessEqualNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.LESS_EQUAL";
  } else if(node_cast<GreaterEqualNode<backtest::BacktestMethodContext>>(
             node)) {
    return "COMPARISON.GREATER_EQUAL";
  } else if(node_cast<EqualNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.EQUAL";
  } else if(node_cast<NotEqualNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.NOT_EQUAL";
  } else if(node_cast<CrossoverNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.CROSSOVER";
  } else if(node_cast<CrossunderNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.CROSSUNDER";
  } else if(node_cast<LogicalNotNode<backtest::BacktestMethodContext>>(node)) {
    return "LOGIC.NOT";
  } else if(node_cast<LogicalAndNode<backtest::BacktestMethodContext>>(node)) {
    return "LOGIC.AND";
  } else if(node_cast<LogicalOrNode<backtest::BacktestMethodContext>>(node)) {
    return "LOGIC.OR";
  } else if(node_cast<LogicalXorNode<backtest::BacktestMethodContext>>(node)) {
    return "LOGIC.XOR";
  }

  return "UNKNOWN";
}

auto get_series_node_title(const std::string& series_id) -> std::string
{
  if(series_id == "REQUESTED_ORDER.PRICE") {
    return "Requested Order Price";
  } else if(series_id == "REQUESTED_ORDER.DIRECTION") {
    return "Requested Order Direction";
  } else if(series_id == "REQUESTED_ORDER.IS_PYRAMIDING") {
    return "Is Pyramiding Order";
  } else if(series_id == "REQUESTED_ORDER.RAW_QUANTITY") {
    return "Raw Requested Quantity";
  } else if(series_id == "REQUESTED_ORDER.RAW_QUANTITY_LIMIT") {
    return "Raw Requested Quantity Limit";
  } else if(series_id == "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY") {
    return "Drawdown-Adjusted Quantity";
  } else if(series_id == "REQUESTED_ORDER.DRAWDOWN_ADJUSTED_QUANTITY_LIMIT") {
    return "Drawdown-Adjusted Quantity Limit";
  } else if(series_id == "REQUESTED_ORDER.QUANTITY") {
    return "Requested Quantity";
  } else if(series_id == "REQUESTED_ORDER.NOTIONAL") {
    return "Requested Notional";
  } else if(series_id == "REQUESTED_ORDER.COST") {
    return "Requested Cost";
  } else if(series_id == "REQUESTED_ORDER.ESTIMATED_ENTRY_FEE") {
    return "Estimated Entry Fee";
  } else if(series_id == "REQUESTED_ORDER.ESTIMATED_1R_EXIT_FEE") {
    return "Estimated 1R Exit Fee";
  } else if(series_id == "REQUESTED_ORDER.RISK_DISTANCE") {
    return "Risk Distance";
  } else if(series_id == "REQUESTED_ORDER.PRICE_RISK") {
    return "Requested Price Risk";
  } else if(series_id == "REQUESTED_ORDER.RISK_WITH_FEES") {
    return "Requested Risk With Fees";
  } else if(series_id == "REQUESTED_ORDER.FROZEN_UNIT_QUANTITY") {
    return "Frozen Unit Quantity";
  } else if(series_id == "OPERATOR.ABS") {
    return "Absolute Value";
  } else if(series_id == "OPERATOR.MAX") {
    return "Maximum";
  } else if(series_id == "OPERATOR.MIN") {
    return "Minimum";
  } else if(series_id == "OPERATOR.POSITIVE_PART") {
    return "Positive Part";
  } else if(series_id == "OPERATOR.NEGATIVE_PART") {
    return "Negative Part";
  } else if(series_id == "MARKET_DATA.OPEN") {
    return "Open Price";
  } else if(series_id == "MARKET_DATA.CLOSE") {
    return "Close Price";
  } else if(series_id == "MARKET_DATA.HIGH") {
    return "High Price";
  } else if(series_id == "MARKET_DATA.LOW") {
    return "Low Price";
  } else if(series_id == "MARKET_DATA.VOLUME") {
    return "Volume";
  } else if(series_id == "OPERATOR.CHANGE") {
    return "Change";
  } else if(series_id == "MARKET_DATA.FIELD") {
    return "Data";
  } else if(series_id == "PORTFOLIO.EQUITY") {
    return "Equity";
  } else if(series_id == "PORTFOLIO.EQUITY_PERCENT") {
    return "Equity (%)";
  } else if(series_id == "PORTFOLIO.DRAWDOWN") {
    return "Drawdown";
  } else if(series_id == "INDICATOR.SMA") {
    return "Simple Moving Average (SMA)";
  } else if(series_id == "INDICATOR.EMA") {
    return "Exponential Moving Average (EMA)";
  } else if(series_id == "INDICATOR.WMA") {
    return "Weighted Moving Average (WMA)";
  } else if(series_id == "INDICATOR.HMA") {
    return "Hull Moving Average (HMA)";
  } else if(series_id == "INDICATOR.RMA") {
    return "Running Moving Average (RMA)";
  } else if(series_id == "INDICATOR.RSI") {
    return "Relative Strength Index (RSI)";
  } else if(series_id == "INDICATOR.ROC") {
    return "Rate of Change (ROC)";
  } else if(series_id == "INDICATOR.RVOL") {
    return "Relative Volume (RVOL)";
  } else if(series_id == "INDICATOR.HIGHEST") {
    return "Highest";
  } else if(series_id == "INDICATOR.LOWEST") {
    return "Lowest";
  } else if(series_id == "INDICATOR.TR") {
    return "True Range (TR)";
  } else if(series_id == "INDICATOR.MACD") {
    return "Moving Average Convergence Divergence (MACD)";
  } else if(series_id == "INDICATOR.ATR") {
    return "Average True Range (ATR)";
  } else if(series_id == "INDICATOR.STDDEV") {
    return "Standard Deviation (STDDEV)";
  } else if(series_id == "INDICATOR.BB") {
    return "Bollinger Bands";
  } else if(series_id == "INDICATOR.KC") {
    return "Keltner Channel (KC)";
  } else if(series_id == "INDICATOR.DC") {
    return "Donchian Channels (DC)";
  } else if(series_id == "INDICATOR.STOCH") {
    return "Stochastic Oscillator";
  } else if(series_id == "INDICATOR.STOCH_RSI") {
    return "Stochastic RSI";
  } else if(series_id == "SERIES.REFERENCE") {
    return "Series";
  } else if(series_id == "VALUE.CONSTANT") {
    return "Value";
  } else if(series_id == "OPERATOR.LOOKBACK") {
    return "Lookback";
  } else if(series_id == "LOGIC.ALL_OF") {
    return "All Of";
  } else if(series_id == "LOGIC.ANY_OF") {
    return "Any Of";
  } else if(series_id == "LOGIC.ALWAYS") {
    return "Always";
  } else if(series_id == "LOGIC.NEVER") {
    return "Never";
  } else if(series_id == "COMPARISON.LESS_THAN") {
    return "Less Than";
  } else if(series_id == "COMPARISON.GREATER_THAN") {
    return "Greater Than";
  } else if(series_id == "COMPARISON.LESS_EQUAL") {
    return "Less Equal";
  } else if(series_id == "COMPARISON.GREATER_EQUAL") {
    return "Greater Equal";
  } else if(series_id == "COMPARISON.EQUAL") {
    return "Equal";
  } else if(series_id == "COMPARISON.NOT_EQUAL") {
    return "Not Equal";
  } else if(series_id == "COMPARISON.CROSSOVER") {
    return "Crossover";
  } else if(series_id == "COMPARISON.CROSSUNDER") {
    return "Crossunder";
  } else if(series_id == "LOGIC.NOT") {
    return "Not";
  } else if(series_id == "LOGIC.AND") {
    return "And";
  } else if(series_id == "LOGIC.OR") {
    return "Or";
  } else if(series_id == "LOGIC.XOR") {
    return "Xor";
  } else if(series_id == "INPUT.NUMERIC") {
    return "From Input";
  } else if(series_id == "OPERATOR.ADD") {
    return "Addition";
  } else if(series_id == "OPERATOR.SUBTRACT") {
    return "Subtraction";
  } else if(series_id == "OPERATOR.MULTIPLY") {
    return "Multiplication";
  } else if(series_id == "OPERATOR.DIVIDE") {
    return "Division";
  } else if(series_id == "OPERATOR.NEGATE") {
    return "Negation";
  } else if(series_id == "OPERATOR.SQRT") {
    return "Square Root";
  } else if(series_id == "OPERATOR.PERCENTAGE") {
    return "Percentage";
  } else if(series_id == "RISK_DISTANCE.AMOUNT") {
    return "1R Amount Distance";
  } else if(series_id == "RISK_DISTANCE.PERCENT") {
    return "1R Percentage Distance";
  } else if(series_id == "RISK_DISTANCE.ATR") {
    return "1R ATR Distance";
  } else if(series_id == "STOP_LOSS.AMOUNT") {
    return "Stop Loss Amount";
  } else if(series_id == "TAKE_PROFIT.AMOUNT") {
    return "Take Profit Amount";
  } else if(series_id == "STOP_LOSS.PERCENT") {
    return "Stop Loss Percent";
  } else if(series_id == "TAKE_PROFIT.PERCENT") {
    return "Take Profit Percent";
  } else if(series_id == "STOP_LOSS.ATR") {
    return "Stop Loss ATR";
  } else if(series_id == "TAKE_PROFIT.ATR") {
    return "Take Profit ATR";
  } else if(series_id == "STOP_LOSS.ONE_R") {
    return "Stop Loss 1R";
  } else if(series_id == "STOP_LOSS.R_MULTIPLE") {
    return "Stop Loss R-Multiple";
  } else if(series_id == "TAKE_PROFIT.R_MULTIPLE") {
    return "Take Profit R-Multiple";
  } else if(series_id == "POSITION.INITIAL_ENTRY_PRICE") {
    return "Initial Entry Price";
  } else if(series_id == "POSITION.LATEST_ENTRY_PRICE") {
    return "Latest Entry Price";
  } else if(series_id == "POSITION.AVERAGE_PRICE") {
    return "Average Price";
  } else if(series_id == "POSITION.STOP_TARGET_REF_PRICE") {
    return "Stop/Target Reference Price";
  } else if(series_id == "POSITION.DIRECTION") {
    return "Position Direction";
  } else if(series_id == "POSITION.PYRAMIDING_LAYER") {
    return "Pyramiding Layer";
  } else if(series_id == "POSITION.R_MULTIPLE") {
    return "Position R-Multiple";
  } else if(series_id == "OPERATOR.ABS_DIFF") {
    return "Absolute Difference";
  } else if(series_id == "OPERATOR.SELECT_OUTPUT") {
    return "Select Output";
  }

  return "Unknown";
}

auto get_portfolio_comparator_node_combo_entries()
 -> const std::vector<ui::ComboEntry>&
{
  static const auto entries = [] {
    const auto ids = std::vector<std::string>{
     "REQUESTED_ORDER.PRICE",
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
     "REQUESTED_ORDER.FROZEN_UNIT_QUANTITY",
     "MARKET_DATA.OPEN",
     "MARKET_DATA.HIGH",
     "MARKET_DATA.LOW",
     "MARKET_DATA.CLOSE",
     "MARKET_DATA.VOLUME",
     "MARKET_DATA.FIELD",
     "OPERATOR.LOOKBACK",
     "VALUE.CONSTANT",
     "OPERATOR.ADD",
     "OPERATOR.SUBTRACT",
     "OPERATOR.MULTIPLY",
     "OPERATOR.DIVIDE",
     "OPERATOR.NEGATE",
     "OPERATOR.ABS",
     "OPERATOR.ABS_DIFF",
     "OPERATOR.SQRT",
     "OPERATOR.MAX",
     "OPERATOR.MIN",
     "OPERATOR.POSITIVE_PART",
     "OPERATOR.NEGATIVE_PART"};
    auto result = std::vector<ui::ComboEntry>{};
    result.reserve(ids.size());
    for(const auto& id : ids) {
      const auto category =
       id == "MARKET_DATA.OPEN" || id == "MARKET_DATA.HIGH" ||
         id == "MARKET_DATA.LOW" || id == "MARKET_DATA.CLOSE" ||
         id == "MARKET_DATA.VOLUME" || id == "MARKET_DATA.FIELD" ||
         id == "OPERATOR.LOOKBACK"
        ? "Asset Data"
       : id == "VALUE.CONSTANT" || id == "OPERATOR.ADD" ||
          id == "OPERATOR.SUBTRACT" || id == "OPERATOR.MULTIPLY" ||
          id == "OPERATOR.DIVIDE" || id == "OPERATOR.NEGATE" ||
          id == "OPERATOR.ABS" || id == "OPERATOR.ABS_DIFF" ||
          id == "OPERATOR.SQRT" || id == "OPERATOR.MAX" ||
          id == "OPERATOR.MIN" || id == "OPERATOR.POSITIVE_PART" ||
          id == "OPERATOR.NEGATIVE_PART"
         ? "Math"
         : "Requested Order";
      result.push_back(ui::ComboEntry{
       .id = id, .title = get_series_node_title(id), .category = category});
    }
    return result;
  }();
  return entries;
}

// Category taxonomy used to group the series node picker so the ~68 node
// types are browsable instead of one long flat list.
auto get_series_node_category(const std::string& series_id) -> std::string
{
  static const auto category_by_id =
   std::unordered_map<std::string, std::string>{
    {"MARKET_DATA.OPEN", "Price & Volume"},
    {"MARKET_DATA.CLOSE", "Price & Volume"},
    {"MARKET_DATA.HIGH", "Price & Volume"},
    {"MARKET_DATA.LOW", "Price & Volume"},
    {"MARKET_DATA.VOLUME", "Price & Volume"},
    {"OPERATOR.CHANGE", "Price & Volume"},
    {"INDICATOR.HIGHEST", "Price & Volume"},
    {"INDICATOR.LOWEST", "Price & Volume"},
    {"MARKET_DATA.FIELD", "Account & Equity"},
    {"PORTFOLIO.EQUITY", "Account & Equity"},
    {"PORTFOLIO.EQUITY_PERCENT", "Account & Equity"},
    {"PORTFOLIO.DRAWDOWN", "Account & Equity"},
    {"INDICATOR.SMA", "Trend & Moving Averages"},
    {"INDICATOR.EMA", "Trend & Moving Averages"},
    {"INDICATOR.WMA", "Trend & Moving Averages"},
    {"INDICATOR.HMA", "Trend & Moving Averages"},
    {"INDICATOR.RMA", "Trend & Moving Averages"},
    {"INDICATOR.RSI", "Momentum & Oscillators"},
    {"INDICATOR.ROC", "Momentum & Oscillators"},
    {"INDICATOR.RVOL", "Momentum & Oscillators"},
    {"INDICATOR.STOCH", "Momentum & Oscillators"},
    {"INDICATOR.STOCH_RSI", "Momentum & Oscillators"},
    {"INDICATOR.MACD", "Momentum & Oscillators"},
    {"INDICATOR.TR", "Volatility & Bands"},
    {"INDICATOR.ATR", "Volatility & Bands"},
    {"INDICATOR.STDDEV", "Volatility & Bands"},
    {"INDICATOR.BB", "Volatility & Bands"},
    {"INDICATOR.KC", "Volatility & Bands"},
    {"INDICATOR.DC", "Volatility & Bands"},
    {"OPERATOR.ADD", "Math & Arithmetic"},
    {"OPERATOR.SUBTRACT", "Math & Arithmetic"},
    {"OPERATOR.MULTIPLY", "Math & Arithmetic"},
    {"OPERATOR.DIVIDE", "Math & Arithmetic"},
    {"OPERATOR.NEGATE", "Math & Arithmetic"},
    {"OPERATOR.SQRT", "Math & Arithmetic"},
    {"OPERATOR.PERCENTAGE", "Math & Arithmetic"},
    {"OPERATOR.ABS_DIFF", "Math & Arithmetic"},
    {"STOP_LOSS.AMOUNT", "Position & Risk"},
    {"TAKE_PROFIT.AMOUNT", "Position & Risk"},
    {"STOP_LOSS.PERCENT", "Position & Risk"},
    {"TAKE_PROFIT.PERCENT", "Position & Risk"},
    {"STOP_LOSS.ATR", "Position & Risk"},
    {"TAKE_PROFIT.ATR", "Position & Risk"},
    {"STOP_LOSS.ONE_R", "Position & Risk"},
    {"STOP_LOSS.R_MULTIPLE", "Position & Risk"},
    {"TAKE_PROFIT.R_MULTIPLE", "Position & Risk"},
    {"POSITION.INITIAL_ENTRY_PRICE", "Position & Risk"},
    {"POSITION.LATEST_ENTRY_PRICE", "Position & Risk"},
    {"POSITION.AVERAGE_PRICE", "Position & Risk"},
    {"POSITION.STOP_TARGET_REF_PRICE", "Position & Risk"},
    {"POSITION.DIRECTION", "Position & Risk"},
    {"POSITION.PYRAMIDING_LAYER", "Position & Risk"},
    {"POSITION.R_MULTIPLE", "Position & Risk"},
    {"LOGIC.ALL_OF", "Logic & Comparison"},
    {"LOGIC.ANY_OF", "Logic & Comparison"},
    {"LOGIC.ALWAYS", "Logic & Comparison"},
    {"LOGIC.NEVER", "Logic & Comparison"},
    {"COMPARISON.LESS_THAN", "Logic & Comparison"},
    {"COMPARISON.GREATER_THAN", "Logic & Comparison"},
    {"COMPARISON.LESS_EQUAL", "Logic & Comparison"},
    {"COMPARISON.GREATER_EQUAL", "Logic & Comparison"},
    {"COMPARISON.EQUAL", "Logic & Comparison"},
    {"COMPARISON.NOT_EQUAL", "Logic & Comparison"},
    {"COMPARISON.CROSSOVER", "Logic & Comparison"},
    {"COMPARISON.CROSSUNDER", "Logic & Comparison"},
    {"LOGIC.NOT", "Logic & Comparison"},
    {"LOGIC.AND", "Logic & Comparison"},
    {"LOGIC.OR", "Logic & Comparison"},
    {"LOGIC.XOR", "Logic & Comparison"},
    {"OPERATOR.SELECT_OUTPUT", "Custom & Input"},
    {"SERIES.REFERENCE", "Custom & Input"},
    {"VALUE.CONSTANT", "Custom & Input"},
    {"OPERATOR.LOOKBACK", "Custom & Input"},
    {"INPUT.NUMERIC", "Custom & Input"},
   };

  const auto it = category_by_id.find(series_id);
  return it != category_by_id.end() ? it->second : "Other";
}

// Series node ids ordered by category so they render as grouped sections in
// the searchable combo (categories must be adjacent for grouping to work).
auto get_series_node_combo_entries() -> const std::vector<ui::ComboEntry>&
{
  static const auto entries = [] {
    static const auto ordered_ids = std::vector<std::string>{
     "MARKET_DATA.OPEN",
     "MARKET_DATA.CLOSE",
     "MARKET_DATA.HIGH",
     "MARKET_DATA.LOW",
     "MARKET_DATA.VOLUME",
     "OPERATOR.CHANGE",
     "INDICATOR.HIGHEST",
     "INDICATOR.LOWEST",
     "MARKET_DATA.FIELD",
     "PORTFOLIO.EQUITY",
     "PORTFOLIO.EQUITY_PERCENT",
     "PORTFOLIO.DRAWDOWN",
     "INDICATOR.SMA",
     "INDICATOR.EMA",
     "INDICATOR.WMA",
     "INDICATOR.HMA",
     "INDICATOR.RMA",
     "INDICATOR.RSI",
     "INDICATOR.ROC",
     "INDICATOR.RVOL",
     "INDICATOR.STOCH",
     "INDICATOR.STOCH_RSI",
     "INDICATOR.MACD",
     "INDICATOR.TR",
     "INDICATOR.ATR",
     "INDICATOR.STDDEV",
     "INDICATOR.BB",
     "INDICATOR.KC",
     "INDICATOR.DC",
     "OPERATOR.ADD",
     "OPERATOR.SUBTRACT",
     "OPERATOR.MULTIPLY",
     "OPERATOR.DIVIDE",
     "OPERATOR.NEGATE",
     "OPERATOR.SQRT",
     "OPERATOR.PERCENTAGE",
     "OPERATOR.ABS_DIFF",
     "STOP_LOSS.AMOUNT",
     "TAKE_PROFIT.AMOUNT",
     "STOP_LOSS.PERCENT",
     "TAKE_PROFIT.PERCENT",
     "STOP_LOSS.ATR",
     "TAKE_PROFIT.ATR",
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
     "LOGIC.ALL_OF",
     "LOGIC.ANY_OF",
     "LOGIC.ALWAYS",
     "LOGIC.NEVER",
     "COMPARISON.LESS_THAN",
     "COMPARISON.GREATER_THAN",
     "COMPARISON.LESS_EQUAL",
     "COMPARISON.GREATER_EQUAL",
     "COMPARISON.EQUAL",
     "COMPARISON.NOT_EQUAL",
     "COMPARISON.CROSSOVER",
     "COMPARISON.CROSSUNDER",
     "LOGIC.NOT",
     "LOGIC.AND",
     "LOGIC.OR",
     "LOGIC.XOR",
     "OPERATOR.SELECT_OUTPUT",
     "SERIES.REFERENCE",
     "VALUE.CONSTANT",
     "OPERATOR.LOOKBACK",
     "INPUT.NUMERIC",
    };

    auto result = std::vector<ui::ComboEntry>{};
    result.reserve(ordered_ids.size());
    for(const auto& id : ordered_ids) {
      result.push_back(
       ui::ComboEntry{.id = id,
                      .title = get_series_node_title(id),
                      .category = get_series_node_category(id)});
    }
    return result;
  }();

  return entries;
}

auto get_condition_node_id(
 const ErasedNode<backtest::BacktestMethodContext>& node) -> std::string
{
  if(node_cast<AllOfNode<backtest::BacktestMethodContext>>(node)) {
    return "LOGIC.ALL_OF";
  } else if(node_cast<AnyOfNode<backtest::BacktestMethodContext>>(node)) {
    return "LOGIC.ANY_OF";
  } else if(node_cast<TrueNode>(node)) {
    return "LOGIC.ALWAYS";
  } else if(node_cast<FalseNode>(node)) {
    return "LOGIC.NEVER";
  } else if(node_cast<LessThanNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.LESS_THAN";
  } else if(node_cast<GreaterThanNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.GREATER_THAN";
  } else if(node_cast<LessEqualNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.LESS_EQUAL";
  } else if(node_cast<GreaterEqualNode<backtest::BacktestMethodContext>>(
             node)) {
    return "COMPARISON.GREATER_EQUAL";
  } else if(node_cast<EqualNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.EQUAL";
  } else if(node_cast<NotEqualNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.NOT_EQUAL";
  } else if(node_cast<CrossoverNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.CROSSOVER";
  } else if(node_cast<CrossunderNode<backtest::BacktestMethodContext>>(node)) {
    return "COMPARISON.CROSSUNDER";
  } else if(node_cast<LogicalNotNode<backtest::BacktestMethodContext>>(node)) {
    return "LOGIC.NOT";
  } else if(node_cast<LogicalAndNode<backtest::BacktestMethodContext>>(node)) {
    return "LOGIC.AND";
  } else if(node_cast<LogicalOrNode<backtest::BacktestMethodContext>>(node)) {
    return "LOGIC.OR";
  } else if(node_cast<LogicalXorNode<backtest::BacktestMethodContext>>(node)) {
    return "LOGIC.XOR";
  }

  return "UNKNOWN";
}

auto get_condition_node_title(const std::string& condition_id) -> std::string
{
  if(condition_id == "LOGIC.ALL_OF") {
    return "All Of";
  } else if(condition_id == "LOGIC.ANY_OF") {
    return "Any Of";
  } else if(condition_id == "LOGIC.ALWAYS") {
    return "Always";
  } else if(condition_id == "LOGIC.NEVER") {
    return "Never";
  } else if(condition_id == "COMPARISON.LESS_THAN") {
    return "Less Than";
  } else if(condition_id == "COMPARISON.GREATER_THAN") {
    return "Greater Than";
  } else if(condition_id == "COMPARISON.LESS_EQUAL") {
    return "Less Equal";
  } else if(condition_id == "COMPARISON.GREATER_EQUAL") {
    return "Greater Equal";
  } else if(condition_id == "COMPARISON.EQUAL") {
    return "Equal";
  } else if(condition_id == "COMPARISON.NOT_EQUAL") {
    return "Not Equal";
  } else if(condition_id == "COMPARISON.CROSSOVER") {
    return "Crossover";
  } else if(condition_id == "COMPARISON.CROSSUNDER") {
    return "Crossunder";
  } else if(condition_id == "LOGIC.NOT") {
    return "Not";
  } else if(condition_id == "LOGIC.AND") {
    return "And";
  } else if(condition_id == "LOGIC.OR") {
    return "Or";
  } else if(condition_id == "LOGIC.XOR") {
    return "Xor";
  }

  return "Unknown";
}

auto get_condition_node_category(const std::string& condition_id) -> std::string
{
  static const auto category_by_id =
   std::unordered_map<std::string, std::string>{
    {"COMPARISON.EQUAL", "Comparison"},
    {"COMPARISON.NOT_EQUAL", "Comparison"},
    {"COMPARISON.GREATER_THAN", "Comparison"},
    {"COMPARISON.LESS_THAN", "Comparison"},
    {"COMPARISON.GREATER_EQUAL", "Comparison"},
    {"COMPARISON.LESS_EQUAL", "Comparison"},
    {"COMPARISON.CROSSOVER", "Crossover"},
    {"COMPARISON.CROSSUNDER", "Crossover"},
    {"LOGIC.ALL_OF", "Logic"},
    {"LOGIC.ANY_OF", "Logic"},
    {"LOGIC.NOT", "Logic"},
    {"LOGIC.AND", "Logic"},
    {"LOGIC.OR", "Logic"},
    {"LOGIC.XOR", "Logic"},
    {"LOGIC.ALWAYS", "Fixed"},
    {"LOGIC.NEVER", "Fixed"},
   };

  const auto it = category_by_id.find(condition_id);
  return it != category_by_id.end() ? it->second : "Other";
}

auto get_condition_node_combo_entries() -> const std::vector<ui::ComboEntry>&
{
  static const auto entries = [] {
    static const auto ordered_ids = std::vector<std::string>{
     "COMPARISON.EQUAL",
     "COMPARISON.NOT_EQUAL",
     "COMPARISON.GREATER_THAN",
     "COMPARISON.LESS_THAN",
     "COMPARISON.GREATER_EQUAL",
     "COMPARISON.LESS_EQUAL",
     "COMPARISON.CROSSOVER",
     "COMPARISON.CROSSUNDER",
     "LOGIC.ALL_OF",
     "LOGIC.ANY_OF",
     "LOGIC.NOT",
     "LOGIC.AND",
     "LOGIC.OR",
     "LOGIC.XOR",
     "LOGIC.ALWAYS",
     "LOGIC.NEVER",
    };

    auto result = std::vector<ui::ComboEntry>{};
    result.reserve(ordered_ids.size());
    for(const auto& id : ordered_ids) {
      result.push_back(
       ui::ComboEntry{.id = id,
                      .title = get_condition_node_title(id),
                      .category = get_condition_node_category(id)});
    }
    return result;
  }();

  return entries;
}

auto get_default_condition_node(const std::string& condition_id)
 -> ErasedNode<backtest::BacktestMethodContext>
{
  if(condition_id == "LOGIC.ALL_OF") {
    return AllOfNode<backtest::BacktestMethodContext>{};
  } else if(condition_id == "LOGIC.ANY_OF") {
    return AnyOfNode<backtest::BacktestMethodContext>{};
  } else if(condition_id == "LOGIC.ALWAYS") {
    return TrueNode{};
  } else if(condition_id == "LOGIC.NEVER") {
    return FalseNode{};
  } else if(condition_id == "COMPARISON.LESS_THAN") {
    return LessThanNode<backtest::BacktestMethodContext>{CloseNode{},
                                                         CloseNode{}};
  } else if(condition_id == "COMPARISON.GREATER_THAN") {
    return GreaterThanNode<backtest::BacktestMethodContext>{CloseNode{},
                                                            CloseNode{}};
  } else if(condition_id == "COMPARISON.LESS_EQUAL") {
    return LessEqualNode<backtest::BacktestMethodContext>{CloseNode{},
                                                          CloseNode{}};
  } else if(condition_id == "COMPARISON.GREATER_EQUAL") {
    return GreaterEqualNode<backtest::BacktestMethodContext>{CloseNode{},
                                                             CloseNode{}};
  } else if(condition_id == "COMPARISON.EQUAL") {
    return EqualNode<backtest::BacktestMethodContext>{CloseNode{}, CloseNode{}};
  } else if(condition_id == "COMPARISON.NOT_EQUAL") {
    return NotEqualNode<backtest::BacktestMethodContext>{CloseNode{},
                                                         CloseNode{}};
  } else if(condition_id == "COMPARISON.CROSSOVER") {
    return CrossoverNode<backtest::BacktestMethodContext>{CloseNode{},
                                                          CloseNode{}};
  } else if(condition_id == "COMPARISON.CROSSUNDER") {
    return CrossunderNode<backtest::BacktestMethodContext>{CloseNode{},
                                                           CloseNode{}};
  } else if(condition_id == "LOGIC.NOT") {
    return LogicalNotNode<backtest::BacktestMethodContext>{FalseNode{}};
  } else if(condition_id == "LOGIC.AND") {
    return LogicalAndNode<backtest::BacktestMethodContext>{FalseNode{},
                                                           FalseNode{}};
  } else if(condition_id == "LOGIC.OR") {
    return LogicalOrNode<backtest::BacktestMethodContext>{FalseNode{},
                                                          FalseNode{}};
  } else if(condition_id == "LOGIC.XOR") {
    return LogicalXorNode<backtest::BacktestMethodContext>{FalseNode{},
                                                           FalseNode{}};
  }

  throw std::invalid_argument{
   std::format("Unknown condition node id: {}", condition_id)};
}

auto get_plot_method_id(const ErasedPlotMethod<ErasedPlotMethodContext>& method)
 -> std::string
{
  if(plot_method_cast<HLinePlotMethod>(method)) {
    return "PLOT.HLINE";
  }

  if(plot_method_cast<LinePlotMethod>(method)) {
    return "PLOT.LINE";
  }

  if(plot_method_cast<HistogramPlotMethod>(method)) {
    return "PLOT.HISTOGRAM";
  }

  if(plot_method_cast<MomentumHistogramPlotMethod>(method)) {
    return "PLOT.MOMENTUM_HISTOGRAM";
  }

  return "UNKNOWN";
}

auto get_plot_method_title(const std::string& plot_id) -> std::string
{
  if(plot_id == "PLOT.LINE") {
    return "Line";
  } else if(plot_id == "PLOT.HLINE") {
    return "Horizontal Line";
  } else if(plot_id == "PLOT.HISTOGRAM") {
    return "Histogram";
  } else if(plot_id == "PLOT.MOMENTUM_HISTOGRAM") {
    return "Momentum Histogram";
  }

  return "Unknown";
}

auto get_default_plot_method(const std::string& plot_id)
 -> ErasedPlotMethod<ErasedPlotMethodContext>
{
  if(plot_id == "PLOT.LINE") {
    return LinePlotMethod{ConstantPlotSourceMethod{0.0}, 0xFFFFFFFF};
  }

  if(plot_id == "PLOT.HLINE") {
    return HLinePlotMethod{0.0, 0xFFFFFFFF};
  }

  if(plot_id == "PLOT.HISTOGRAM") {
    return HistogramPlotMethod{ConstantPlotSourceMethod{0.0}};
  }

  if(plot_id == "PLOT.MOMENTUM_HISTOGRAM") {
    return MomentumHistogramPlotMethod{ConstantPlotSourceMethod{0.0}};
  }

  throw std::invalid_argument{
   std::format("Unknown plot method id: {}", plot_id)};
}

auto get_plot_source_method_id(
 const ErasedPlotSourceMethod<ErasedPlotMethodContext>& method) -> std::string
{
  if(plot_source_method_cast<ConstantPlotSourceMethod>(method)) {
    return "PLOT_SOURCE.CONSTANT";
  } else if(plot_source_method_cast<SeriesPlotSourceMethod>(method)) {
    return "PLOT_SOURCE.SERIES";
  }

  return "UNKNOWN";
}

auto get_plot_source_method_title(const std::string& plot_source_id)
 -> std::string
{
  if(plot_source_id == "PLOT_SOURCE.CONSTANT") {
    return "Constant Value";
  }

  if(plot_source_id == "PLOT_SOURCE.SERIES") {
    return "Series";
  }

  return "Unknown";
}

auto get_default_plot_source_method(const std::string& plot_source_id)
 -> ErasedPlotSourceMethod<ErasedPlotMethodContext>
{
  if(plot_source_id == "PLOT_SOURCE.CONSTANT") {
    return ConstantPlotSourceMethod{0.0};
  }

  if(plot_source_id == "PLOT_SOURCE.SERIES") {
    return SeriesPlotSourceMethod{""};
  }

  throw std::invalid_argument{
   std::format("Unknown plot source method id: {}", plot_source_id)};
}

export class ModelsWindow {
public:
  ModelsWindow()
  {
  }

  void render(this auto& self, WindowContext& context)
  {
    ImGui::Begin("Models");

    switch(self.current_page_) {
    case Page::BuiltIn:
      self.render_built_in_strategies(context);
      break;
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

  void discard_draft(this ModelsWindow& self)
  {
    self.reset();
  }

  void render_numeric_expression(
   this ModelsWindow& self,
   ErasedNode<backtest::BacktestMethodContext>& expression,
   WindowContext& context,
   std::vector<std::string> available_series_names = {},
   bool allow_unlisted_series_names = false)
  {
    const auto previous_names = std::move(self.available_series_names_);
    const auto previous_allow_unlisted = self.allow_unlisted_series_names_;
    self.available_series_names_ = std::move(available_series_names);
    self.allow_unlisted_series_names_ = allow_unlisted_series_names;
    self.render_series_node(expression, context);
    self.available_series_names_ = std::move(previous_names);
    self.allow_unlisted_series_names_ = previous_allow_unlisted;
  }

  void
  render_portfolio_comparator_expression(this ModelsWindow& self,
                                         backtest::ComparatorNode& expression,
                                         WindowContext& context)
  {
    self.render_comparator_node(expression, context);
  }

private:
  void render_comparator_node(this ModelsWindow& self,
                              backtest::ComparatorNode& node,
                              WindowContext& context)
  {
    auto node_id = get_portfolio_comparator_node_id(node);
    const auto selected =
     ui::searchable_combo("##Series",
                          node_id,
                          get_series_node_title(node_id),
                          get_portfolio_comparator_node_combo_entries());
    if(selected) {
      node = get_default_portfolio_comparator_node(*selected);
    }

    ImGui::Indent();
    if(auto* value = node_cast<ValueNode>(node)) {
      self.render_series_node_params(*value, context);
    } else if(auto* data = node_cast<DataNode>(node)) {
      self.render_series_node_params(*data, context);
    } else if(auto* lookback =
               node_cast<LookbackNode<backtest::RequestedOrderMethodContext>>(
                node)) {
      auto periods = static_cast<int>(lookback->period());
      ui::field_label("Periods");
      if(ImGui::InputInt("##lookback_periods", &periods)) {
        lookback->period(static_cast<std::size_t>(std::max(periods, 1)));
      }
      ui::field_label("Source");
      auto source = lookback->source();
      ImGui::PushID("source");
      self.render_comparator_node(source, context);
      ImGui::PopID();
      lookback->source(std::move(source));
    } else if(!self.render_comparator_binary(node, context)) {
      self.render_comparator_unary(node, context);
    }
    ImGui::Unindent();
  }

  auto render_comparator_binary(this ModelsWindow& self,
                                backtest::ComparatorNode& node,
                                WindowContext& context) -> bool
  {
    return [&]<typename... TNodes>() {
      return ([&] {
        auto* binary = node_cast<TNodes>(node);
        if(!binary) {
          return false;
        }
        ui::field_label("Left");
        auto left = binary->left();
        ImGui::PushID("left");
        self.render_comparator_node(left, context);
        ImGui::PopID();
        binary->left(std::move(left));
        ui::field_label("Right");
        auto right = binary->right();
        ImGui::PushID("right");
        self.render_comparator_node(right, context);
        ImGui::PopID();
        binary->right(std::move(right));
        return true;
      }() || ...);
    }.template operator()<AddNode<backtest::RequestedOrderMethodContext>,
                          SubtractNode<backtest::RequestedOrderMethodContext>,
                          MultiplyNode<backtest::RequestedOrderMethodContext>,
                          DivideNode<backtest::RequestedOrderMethodContext>,
                          AbsDiffNode<backtest::RequestedOrderMethodContext>,
                          MaxNode<backtest::RequestedOrderMethodContext>,
                          MinNode<backtest::RequestedOrderMethodContext>>();
  }

  auto render_comparator_unary(this ModelsWindow& self,
                               backtest::ComparatorNode& node,
                               WindowContext& context) -> bool
  {
    return
     [&]<typename... TNodes>() {
       return ([&] {
         auto* unary = node_cast<TNodes>(node);
         if(!unary) {
           return false;
         }
         ui::field_label("Value");
         auto value = unary->operand();
         ImGui::PushID("value");
         self.render_comparator_node(value, context);
         ImGui::PopID();
         unary->operand(std::move(value));
         return true;
       }() || ...);
     }.template
     operator()<NegateNode<backtest::RequestedOrderMethodContext>,
                AbsNode<backtest::RequestedOrderMethodContext>,
                SqrtNode<backtest::RequestedOrderMethodContext>,
                PositivePartNode<backtest::RequestedOrderMethodContext>,
                NegativePartNode<backtest::RequestedOrderMethodContext>>();
  }

  enum class Page { List, BuiltIn, AddNew, Edit } current_page_{Page::List};

  std::optional<backtest::ModelStoreHandle> selected_model_handle_opt_;
  std::shared_ptr<backtest::Model> editing_strategy_ptr_;
  std::shared_ptr<backtest::Model> editor_baseline_ptr_;

  ImGuiTextFilter strategy_filter_;
  ImGuiTextFilter built_in_strategy_filter_;

  std::vector<std::string> available_series_names_;
  bool allow_unlisted_series_names_{};
  std::unordered_map<std::string, std::string> changed_series_names_;

  auto has_unsaved_changes(this const auto& self) -> bool
  {
    return self.editing_strategy_ptr_ && self.editor_baseline_ptr_ &&
           *self.editing_strategy_ptr_ != *self.editor_baseline_ptr_;
  }

  void begin_add_model(this auto& self)
  {
    self.current_page_ = Page::AddNew;
    self.selected_model_handle_opt_ = std::nullopt;
    self.editing_strategy_ptr_ = std::make_shared<backtest::Model>();
    self.editor_baseline_ptr_ =
     std::make_shared<backtest::Model>(*self.editing_strategy_ptr_);
  }

  void begin_add_model(this auto& self, backtest::Model model)
  {
    self.current_page_ = Page::AddNew;
    self.selected_model_handle_opt_ = std::nullopt;
    self.editing_strategy_ptr_ =
     std::make_shared<backtest::Model>(std::move(model));
    self.editor_baseline_ptr_ =
     std::make_shared<backtest::Model>(*self.editing_strategy_ptr_);
  }

  void begin_edit_model(this auto& self,
                        backtest::ModelStoreHandle model_handle,
                        const backtest::Model& model)
  {
    self.current_page_ = Page::Edit;
    if(self.selected_model_handle_opt_ != model_handle ||
       !self.editing_strategy_ptr_ || !self.editor_baseline_ptr_) {
      self.selected_model_handle_opt_ = model_handle;
      self.editing_strategy_ptr_ = std::make_shared<backtest::Model>(model);
      self.editor_baseline_ptr_ = std::make_shared<backtest::Model>(model);
    }
  }

  void request_leave_editor(this auto& self)
  {
    self.current_page_ = Page::List;
  }

  void export_model(this const auto&,
                    const backtest::Model& strategy,
                    WindowContext& context)
  {
    const auto serialized_strategy = stringify_model(strategy);
#ifdef __EMSCRIPTEN__
    const auto file_name = "pludux-model-" + strategy.name() + ".json";
    pludux_js_save_file(
     file_name.c_str(), serialized_strategy.c_str(), "application/json");
#else
    auto nfd_guard = NFD::Guard{};
    auto out_path = NFD::UniquePath{};
    const auto filter_item =
     std::array<nfdfilteritem_t, 1>{{"JSON Files", "json"}};
    const auto result =
     NFD::SaveDialog(out_path, filter_item.data(), filter_item.size());

    if(result == NFD_OKAY) {
      const auto saved_path = std::string(out_path.get());
      auto out_stream = std::ofstream{saved_path};
      if(!out_stream.is_open()) {
        throw std::runtime_error(
         std::format("Failed to open '{}' for writing.", saved_path));
      }
      out_stream << serialized_strategy;
    } else if(result == NFD_ERROR) {
      throw std::runtime_error(
       std::format("Error '{}': {}", "Export", NFD::GetError()));
    }
#endif
  }

  void import_strategies(this const auto&, WindowContext& context)
  {
#ifdef __EMSCRIPTEN__
    using JsOnOpenedFileContentReady =
     std::function<void(const std::string&, const std::string&, void*)>;

    static const auto callback =
     JsOnOpenedFileContentReady{[](const std::string& file_name,
                                   const std::string& file_data,
                                   void* user_data) {
       auto& callback_context = *reinterpret_cast<WindowContext*>(user_data);
       callback_context.push_edit("Import Model",
                                  LoadModelJsonAction{file_name, file_data});
     }};
    pludux_js_open_multiple_text_files(".json", &callback, &context);
#else
    auto nfd_guard = NFD::Guard{};
    auto in_paths = NFD::UniquePathSet{};
    const auto filter_item =
     std::array<nfdfilteritem_t, 1>{{"JSON Files", "json"}};
    auto result =
     NFD::OpenDialogMultiple(in_paths, filter_item.data(), filter_item.size());

    try {
      if(result == NFD_OKAY) {
        auto paths_count = nfdpathsetsize_t{};
        result = NFD::PathSet::Count(in_paths, paths_count);
        if(result == NFD_ERROR) {
          throw std::runtime_error(
           std::format("Error '{}': {}", "Import", NFD::GetError()));
        }

        for(nfdpathsetsize_t i = 0; i < paths_count; ++i) {
          auto in_path = NFD::UniquePathSetPath{};
          result = NFD::PathSet::GetPath(in_paths, i, in_path);
          if(result == NFD_ERROR) {
            throw std::runtime_error(
             std::format("Error '{}': {}", "Import", NFD::GetError()));
          }
          context.push_edit("Import Model",
                            LoadModelJsonAction{std::string{in_path.get()}});
        }
      } else if(result == NFD_ERROR) {
        throw std::runtime_error(
         std::format("Error '{}': {}", "Import", NFD::GetError()));
      }
    } catch(const std::exception& ex) {
      context.alert(ex.what());
    }
#endif
  }

  void render_list_strategies(this auto& self, WindowContext& context)
  {
    const auto& app_state = context.app_state();
    const auto& model_handles = app_state.get_model_handles();
    const auto system_ptr = app_state.selected_system_if_present();

    ImGui::BeginGroup();
    if(ImGui::Button(PLUDUX_ICON_ADD " New Model")) {
      self.begin_add_model();
    }
    ImGui::SameLine();
    if(ImGui::Button(PLUDUX_ICON_IMPORT " Import")) {
      self.import_strategies(context);
    }
    ImGui::SameLine();
    if(ImGui::Button("Built-in Models")) {
      self.current_page_ = Page::BuiltIn;
    }
    ImGui::Spacing();
    ui::search_filter(self.strategy_filter_, "##strategy_search");
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
      ImGui::SetTooltip("Filter models by name");
    }
    ImGui::Separator();

    const auto visible_strategy_count =
     std::ranges::count_if(model_handles, [&](const auto model_handle) {
       return self.strategy_filter_.PassFilter(
        app_state.get_model(model_handle).name().c_str());
     });

    ImGui::BeginChild("strategy_list", ImVec2(0, 0));
    if(model_handles.empty()) {
      ImGui::Spacing();
      ImGui::TextDisabled("No models yet.");
      ImGui::TextWrapped(
       "Create a model or import one from a Pludux model JSON file.");
    } else if(visible_strategy_count == 0) {
      ImGui::Spacing();
      ImGui::TextDisabled("No models match this search.");
    }

    for(std::size_t i = 0; i < model_handles.size(); ++i) {
      const auto model_handle = model_handles[i];
      const auto& model = app_state.get_model(model_handle);
      const auto& model_name = model.name();

      if(!self.strategy_filter_.PassFilter(model_name.c_str())) {
        continue;
      }

      ImGui::PushID(i);

      ImGui::SetNextItemAllowOverlap();
      const auto is_selected = system_ptr && [&] {
        for(auto strategy_index = std::size_t{};
            strategy_index < system_ptr->strategy_count();
            ++strategy_index) {
          const auto* strategy = app_state.get_strategy_if_present(
           system_ptr->strategy_handle(strategy_index));
          if(strategy && strategy->references_model(model_handle)) {
            return true;
          }
        }
        return false;
      }();
      const auto has_draft = self.selected_model_handle_opt_ == model_handle &&
                             self.has_unsaved_changes();
      const auto display_name =
       has_draft ? model_name + " (Unsaved)" : model_name;
      const auto row_start = ImGui::GetCursorScreenPos();
      const auto row_width = ImGui::GetContentRegionAvail().x;
      const auto row_height = ImGui::GetFrameHeight();

      const auto row_clicked =
       ImGui::Selectable("##strategy_row",
                         is_selected,
                         ImGuiSelectableFlags_AllowDoubleClick |
                          ImGuiSelectableFlags_AllowOverlap,
                         ImVec2(row_width, row_height));
      if(row_clicked && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        self.begin_edit_model(model_handle, model);
      }

      ImGui::SetCursorScreenPos(row_start);
      ImGui::AlignTextToFramePadding();
      ImGui::TextUnformatted(display_name.c_str());

      const auto& style = ImGui::GetStyle();
      const auto spacing = style.ItemSpacing.x;
      const auto edit_width =
       ImGui::CalcTextSize(PLUDUX_ICON_EDIT).x + (2.0f * style.FramePadding.x);
      const auto more_width =
       ImGui::CalcTextSize(PLUDUX_ICON_MORE).x + (2.0f * style.FramePadding.x);
      const auto buttons_width = edit_width + spacing + more_width;
      const auto buttons_start_x =
       std::max(row_start.x, row_start.x + row_width - buttons_width);

      ImGui::SetCursorScreenPos(ImVec2(buttons_start_x, row_start.y));

      if(ui::icon_button(PLUDUX_ICON_EDIT "##edit_strategy", "Edit model")) {
        self.begin_edit_model(model_handle, model);
      }
      ImGui::SameLine();
      if(ui::icon_button(PLUDUX_ICON_MORE "##strategy_menu", "More actions")) {
        ImGui::OpenPopup("strategy_menu_more");
      }

      if(ImGui::BeginPopup("strategy_menu_more")) {
        if(ImGui::MenuItem(PLUDUX_ICON_EDIT " Edit")) {
          self.begin_edit_model(model_handle, model);
        }
        if(ImGui::MenuItem(PLUDUX_ICON_COPY " Duplicate")) {
          context.push_edit(
           "Duplicate Model", [model_handle](ApplicationState& app_state) {
             const auto& model = app_state.get_model(model_handle);
             auto duplicate_model = model;
             duplicate_model.name(model.name() + " Copy");
             app_state.add_model(std::move(duplicate_model));
           });
        }
        if(ImGui::MenuItem(PLUDUX_ICON_EXPORT " Export")) {
          self.export_model(model, context);
        }
        ImGui::Separator();

        const auto move_up_disabled = i == 0;
        if(ImGui::MenuItem(PLUDUX_ICON_MOVE_UP " Move Up",
                           nullptr,
                           false,
                           !move_up_disabled)) {
          context.push_edit(
           "Move Model Up",
           [from_index = i, to_index = i - 1](ApplicationState& app_state) {
             app_state.reorder_list_model(from_index, to_index);
           });
        }

        const auto move_down_disabled = i == model_handles.size() - 1;
        if(ImGui::MenuItem(PLUDUX_ICON_MOVE_DOWN " Move Down",
                           nullptr,
                           false,
                           !move_down_disabled)) {
          context.push_edit(
           "Move Model Down",
           [from_index = i, to_index = i + 1](ApplicationState& app_state) {
             app_state.reorder_list_model(from_index, to_index);
           });
        }
        ImGui::Separator();
        if(ImGui::MenuItem(PLUDUX_ICON_DELETE " Delete")) {
          context.push_edit("Delete Model",
                            [model_handle](ApplicationState& app_state) {
                              app_state.remove_model(model_handle);
                            });
        }

        ImGui::EndPopup();
      }

      ImGui::PopID();
      ImGui::Separator();
    }

    ImGui::EndChild();
    ImGui::EndGroup();
  }

  void render_built_in_strategies(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    if(ui::icon_button(PLUDUX_ICON_BACK "##back_to_strategies",
                       "Back to models")) {
      self.current_page_ = Page::List;
    }
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Built-in Models");
    ImGui::Separator();

    ui::search_filter(self.built_in_strategy_filter_,
                      "##built_in_strategy_search");
    if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
      ImGui::SetTooltip("Filter built-in models by name or description");
    }
    ImGui::Separator();

    const auto strategies = built_in_strategies();
    const auto visible_strategy_count =
     std::ranges::count_if(strategies, [&](const auto& strategy) {
       const auto searchable_text =
        std::format("{}\n{}", strategy.name, strategy.description);
       return self.built_in_strategy_filter_.PassFilter(
        searchable_text.c_str());
     });

    ImGui::BeginChild("built_in_strategy_list", ImVec2(0, 0));
    if(visible_strategy_count == 0) {
      ImGui::Spacing();
      ImGui::TextDisabled("No built-in models match this search.");
    }

    for(const auto& strategy : strategies) {
      const auto searchable_text =
       std::format("{}\n{}", strategy.name, strategy.description);
      if(!self.built_in_strategy_filter_.PassFilter(searchable_text.c_str())) {
        continue;
      }

      ImGui::PushID(strategy.name.data());
      ImGui::TextUnformatted(strategy.name.data(),
                             strategy.name.data() + strategy.name.size());
      ImGui::TextDisabled("%.*s",
                          static_cast<int>(strategy.description.size()),
                          strategy.description.data());
      if(ImGui::Button("Use Model")) {
        try {
          self.begin_add_model(
           backtest::parse_model(strategy.name, std::string{strategy.json}));
        } catch(const std::exception& ex) {
          context.alert(std::format("Failed to load built-in model '{}':\n{}",
                                    strategy.name,
                                    ex.what()));
        }
      }
      ImGui::Separator();
      ImGui::PopID();
    }

    ImGui::EndChild();
    ImGui::EndGroup();
  }

  void render_add_new_strategy(this auto& self, WindowContext& context)
  {
    ImGui::BeginGroup();
    if(ui::icon_button(PLUDUX_ICON_BACK "##back_to_strategies",
                       "Back to models")) {
      self.reset();
    }
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("New Model");
    if(self.has_unsaved_changes()) {
      ImGui::SameLine();
      ImGui::TextDisabled("(Unsaved)");
    }
    ImGui::Separator();

    ImGui::BeginChild("add_strategy_content",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
    self.edit_strategy_form(context);
    ImGui::EndChild();

    if(ImGui::Button(PLUDUX_ICON_ADD " Create")) {
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
    if(ui::icon_button(PLUDUX_ICON_BACK "##back_to_strategies",
                       "Back to models")) {
      self.request_leave_editor();
    }
    ImGui::SameLine();
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Edit Model");
    if(self.has_unsaved_changes()) {
      ImGui::SameLine();
      ImGui::TextDisabled("(Unsaved)");
    }
    ImGui::Separator();

    ImGui::BeginChild("edit_strategy_content",
                      ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
    self.edit_strategy_form(context);
    ImGui::EndChild();

    const auto changed = self.has_unsaved_changes();
    if(ImGui::Button("OK")) {
      if(changed) {
        self.submit_strategy_changes(context);
      }
      self.reset();
    }

    ImGui::SameLine();
    if(ImGui::Button("Cancel")) {
      self.request_leave_editor();
    }

    ImGui::SameLine();
    const auto draft_action = ui::apply_reset_button(changed);
    if(draft_action == ui::DraftAction::Apply) {
      self.submit_strategy_changes(context);
      self.editor_baseline_ptr_ =
       std::make_shared<backtest::Model>(*self.editing_strategy_ptr_);
    } else if(draft_action == ui::DraftAction::Reset) {
      const auto model_handle = self.selected_model_handle_opt_.value();
      const auto& model = context.app_state().get_model(model_handle);
      self.editing_strategy_ptr_ = std::make_shared<backtest::Model>(model);
      self.editor_baseline_ptr_ = std::make_shared<backtest::Model>(model);
    }

    ImGui::EndGroup();
  }

  void edit_strategy_form(this auto& self, WindowContext& context)
  {
    ui::form_section(
     "Model Details",
     "Build reusable trading rules from series, entry and exit conditions, "
     "and optional chart plots. A blank name is saved as 'Unnamed'.");
    {
      ui::field_label("Name");
      auto strategy_name = self.editing_strategy_ptr_->name();
      ImGui::InputTextWithHint("##strategy_name", "Unnamed", &strategy_name);
      self.editing_strategy_ptr_->name(strategy_name);
      ImGui::Spacing();
    }

    {
      if(ui::collapsible_section("Execution")) {
        ui::section_description(
         "Choose how ambiguous intrabar price movement is simulated when a "
         "single candle can touch multiple order levels.");
        self.editing_strategy_ptr_->intrabar_path(self.render_intrabar_path(
         self.editing_strategy_ptr_->intrabar_path()));
      }
    }

    {
      auto& series_nodes = self.editing_strategy_ptr_->series_nodes();
      const auto header = std::format(
       "Series Nodes ({})###series_nodes_section", series_nodes.size());

      self.changed_series_names_.clear();
      self.available_series_names_.clear();

      if(!ui::collapsible_section(header.c_str())) {
        // Collapsed: still populate available_series_names_ so other
        // sections (Plots, Positions) can reference existing series.
        for(auto& [series_name, series_node] : series_nodes) {
          self.available_series_names_.push_back(series_name);
        }
      } else {
        ui::section_description(
         "Create named calculations that can be reused by position rules and "
         "plots. Names must be unique within this model.");
        auto updated_series_nodes = series_nodes;
        for(auto id_counter = 0;
            auto& [series_name, series_node] : series_nodes) {
          ImGui::PushID(id_counter++);

          ui::field_label("Name");
          auto updated_series_name = series_name;
          ImGui::InputTextWithHint(
           "##series_name", "Unique series name", &updated_series_name);
          if(ImGui::IsItemDeactivatedAfterEdit()) {
            if(updated_series_name != series_name) {
              self.changed_series_names_[series_name] = updated_series_name;
            }
          }

          ui::field_label("Node");
          ImGui::PushID("series_node");
          self.render_series_node(series_node, context);
          ImGui::PopID();
          updated_series_nodes.set(series_name, series_node);

          if(ui::right_aligned_button(PLUDUX_ICON_DELETE " Delete")) {
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

        if(ImGui::Button(PLUDUX_ICON_ADD " Add Series")) {
          auto new_series_name =
           std::format("new_var_{}", series_nodes.size() + 1);
          auto new_series_node = get_default_series_node("MARKET_DATA.CLOSE");
          series_nodes.set(new_series_name, new_series_node);
        }
      }

      ImGui::Text("");
    }

    if(ui::collapsible_section("Positions")) {
      ui::section_description(
       "Configure long and short entry rules independently. Exit, stop-loss, "
       "take-profit, pyramiding, and risk controls are optional.");
      if(ui::collapsible_section("Long Position###long_position_section",
                                 false)) {
        ImGui::PushID("long_position");
        ImGui::Indent();

        auto long_position = self.editing_strategy_ptr_->long_position();
        self.render_position_form(long_position, context);
        self.editing_strategy_ptr_->long_position(std::move(long_position));

        ImGui::Unindent();
        ImGui::PopID();
      }

      if(ui::collapsible_section("Short Position###short_position_section",
                                 false)) {
        ImGui::PushID("short_position");
        ImGui::Indent();

        auto short_position = self.editing_strategy_ptr_->short_position();
        self.render_position_form(short_position, context);
        self.editing_strategy_ptr_->short_position(std::move(short_position));

        ImGui::Unindent();
        ImGui::PopID();
      }
    }

    ImGui::Text("");

    {
      auto plot_groups = self.editing_strategy_ptr_->plots();
      const auto header =
       std::format("Plots ({})###plots_section", plot_groups.size());

      if(ui::collapsible_section(header.c_str())) {
        ui::section_description(
         "Add reference lines or calculated series to the chart. Overlay "
         "plots share the price panel; other plots use a separate panel.");
        for(auto i = 0; i < plot_groups.size(); ++i) {
          auto& plot_group = plot_groups[i];

          ImGui::PushID(i);

          ui::field_label("Plot Name");
          auto plot_name = plot_group.name();
          ImGui::InputTextWithHint(
           "##plot_name", "Plot group name", &plot_name);
          plot_group.name(plot_name);

          ui::field_label("Overlays");
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

              if(ui::right_aligned_button(PLUDUX_ICON_DELETE " Remove Item")) {
                plot_items.erase(plot_items.begin() + j);
                --j; // Adjust index after removal
              }

              ImGui::Separator();
              ImGui::PopID();
            }

            if(ImGui::Button(PLUDUX_ICON_ADD " Add Item")) {
              plot_items.emplace_back(get_default_plot_method("PLOT.HLINE"));
            }

            plot_group.items(plot_items);
          }

          if(ui::right_aligned_button(PLUDUX_ICON_DELETE " Remove Plot")) {
            plot_groups.erase(plot_groups.begin() + i);
            --i; // Adjust index after removal
          }

          ImGui::Separator();
          ImGui::PopID();
        }

        if(ImGui::Button(PLUDUX_ICON_ADD " Add Plot")) {
          plot_groups.emplace_back("New Plot");
        }
      }

      self.editing_strategy_ptr_->plots(std::move(plot_groups));
    }
  }

  auto render_intrabar_path(this const auto&, backtest::IntrabarPath path)
   -> backtest::IntrabarPath
  {
    ui::field_label(
     "Intrabar Path",
     "Determines which candle extreme is assumed to occur first when the "
     "actual tick path is unavailable.");
    auto preview = "Candle Direction";
    if(path == backtest::IntrabarPath::LowFirst) {
      preview = "Low First";
    } else if(path == backtest::IntrabarPath::HighFirst) {
      preview = "High First";
    }
    if(ImGui::BeginCombo("##intrabar_path", preview)) {
      for(const auto [value, title] :
          {std::pair{backtest::IntrabarPath::LowFirst, "Low First"},
           std::pair{backtest::IntrabarPath::HighFirst, "High First"},
           std::pair{backtest::IntrabarPath::CandleDirection,
                     "Candle Direction"}}) {
        const auto selected = path == value;
        if(ImGui::Selectable(title, selected)) {
          path = value;
        }
        if(selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    return path;
  }

  auto render_signal_timing(this const auto&,
                            const char* label,
                            const char* id,
                            backtest::SignalTiming timing)
   -> backtest::SignalTiming
  {
    ui::field_label(
     label,
     "Current Close executes at the signal candle close; Next Open waits for "
     "the following candle.");
    const auto preview = timing == backtest::SignalTiming::CurrentClose
                          ? "Current Close"
                          : "Next Open";
    if(ImGui::BeginCombo(id, preview)) {
      for(const auto [value, title] :
          {std::pair{backtest::SignalTiming::CurrentClose, "Current Close"},
           std::pair{backtest::SignalTiming::NextOpen, "Next Open"}}) {
        const auto selected = timing == value;
        if(ImGui::Selectable(title, selected)) {
          timing = value;
        }
        if(selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    return timing;
  }

  auto render_exit_activation(this const auto&,
                              const char* id,
                              backtest::ExitActivation activation)
   -> backtest::ExitActivation
  {
    ui::field_label(
     "Activation",
     "Simultaneous evaluates every rule together. After Previous activates "
     "each rule only after the prior one has completed.");
    const auto preview = activation == backtest::ExitActivation::Simultaneous
                          ? "Simultaneous"
                          : "After Previous";
    if(ImGui::BeginCombo(id, preview)) {
      for(const auto [value, title] :
          {std::pair{backtest::ExitActivation::Simultaneous, "Simultaneous"},
           std::pair{backtest::ExitActivation::AfterPrevious,
                     "After Previous"}}) {
        const auto selected = activation == value;
        if(ImGui::Selectable(title, selected)) {
          activation = value;
        }
        if(selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    return activation;
  }

  auto render_pyramiding_retrigger(this const auto&,
                                   backtest::PyramidingRetrigger retrigger)
   -> backtest::PyramidingRetrigger
  {
    ui::field_label(
     "Retrigger",
     "Every Evaluation can add another layer while the signal remains true. "
     "After False requires the signal to become false before it can trigger "
     "again.");
    const auto preview =
     retrigger == backtest::PyramidingRetrigger::EveryEvaluation
      ? "Every Evaluation"
      : "After False";
    if(ImGui::BeginCombo("##retrigger", preview)) {
      for(const auto [value, title] :
          {std::pair{backtest::PyramidingRetrigger::EveryEvaluation,
                     "Every Evaluation"},
           std::pair{backtest::PyramidingRetrigger::AfterFalse,
                     "After False"}}) {
        const auto selected = retrigger == value;
        if(ImGui::Selectable(title, selected)) {
          retrigger = value;
        }
        if(selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    return retrigger;
  }

  void render_position_form(this auto& self,
                            backtest::Model::Position& position,
                            WindowContext& context)
  {
    if(ui::collapsible_section("Entry")) {
      ui::section_description(
       "The entry signal must evaluate true before a new position can open.");
      ImGui::PushID("entry");
      auto entry = position.entry();
      auto changed_node = self.render_condition_node(entry.signal(), context);
      entry.signal(std::move(changed_node));
      entry.timing(
       self.render_signal_timing("Timing", "##timing", entry.timing()));
      position.entry(std::move(entry));
      ImGui::PopID();
    }
    {
      const auto exits_header = std::format(
       "Signal Exits ({})###signal_exits_section", position.exits().size());
      if(ui::collapsible_section(exits_header.c_str())) {
        ui::section_description(
         "Close or reduce a position when a condition becomes true. Rules can "
         "run together or activate in sequence.");
        ImGui::PushID("signal_exits");
        auto exits = position.exits();
        position.exits_activation(self.render_exit_activation(
         "##activation", position.exits_activation()));
        if(ImGui::Button(PLUDUX_ICON_ADD " Add Signal Exit")) {
          exits.emplace_back();
        }

        for(auto index = std::size_t{0}; index < exits.size(); ++index) {
          ImGui::PushID(static_cast<int>(index));
          ImGui::Separator();
          ImGui::Text("Signal Exit %zu", index + 1);

          if(ImGui::Button(PLUDUX_ICON_MOVE_UP " Move Up") && index > 0) {
            std::swap(exits[index], exits[index - 1]);
            ImGui::PopID();
            break;
          }
          ImGui::SameLine();
          if(ImGui::Button(PLUDUX_ICON_MOVE_DOWN " Move Down") &&
             index + 1 < exits.size()) {
            std::swap(exits[index], exits[index + 1]);
            ImGui::PopID();
            break;
          }
          ImGui::SameLine();
          if(ImGui::Button(PLUDUX_ICON_DELETE " Remove")) {
            exits.erase(exits.begin() + static_cast<std::ptrdiff_t>(index));
            ImGui::PopID();
            break;
          }

          auto& exit = exits[index];
          auto enabled = exit.enabled();
          ui::field_label("Enabled");
          ImGui::Checkbox("##enabled", &enabled);
          exit.enabled(enabled);

          ImGui::BeginDisabled(!enabled);
          auto changed_node =
           self.render_condition_node(exit.signal(), context);
          exit.signal(std::move(changed_node));
          exit.timing(
           self.render_signal_timing("Timing", "##timing", exit.timing()));

          ImGui::PushID("reduce");
          exit.reduce(self.render_position_reduction(exit.reduce()));
          ImGui::PopID();
          ImGui::EndDisabled();
          ImGui::PopID();
        }

        position.exits(std::move(exits));
        ImGui::PopID();
      }
    }
    if(ui::collapsible_section("Pyramiding", /* default_open= */ false)) {
      ui::section_description(
       "Allow additional entries while a position is already open, up to the "
       "configured number of layers.");
      ImGui::PushID("pyramiding");

      ui::field_label("Signal");
      auto pyramiding = position.pyramiding();

      auto changed_node =
       self.render_condition_node(pyramiding.signal(), context);
      pyramiding.signal(std::move(changed_node));

      pyramiding.timing(
       self.render_signal_timing("Timing", "##timing", pyramiding.timing()));
      pyramiding.retrigger(
       self.render_pyramiding_retrigger(pyramiding.retrigger()));

      auto pyramiding_cooldown = static_cast<int>(pyramiding.cooldown());
      ui::field_label(
       "Cooldown Bars",
       "Pause all pyramiding processing for this many complete bars after "
       "each accepted entry.");
      if(ImGui::InputInt("##cooldown", &pyramiding_cooldown)) {
        if(pyramiding_cooldown < 0) {
          pyramiding_cooldown = 0;
        }
        pyramiding.cooldown(static_cast<std::size_t>(pyramiding_cooldown));
      }

      auto pyramiding_max_layers = static_cast<int>(pyramiding.max_layers());
      ui::field_label(
       "Max Layers",
       "Maximum number of concurrently accumulated entry layers.");
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
         ui::field_label(label);

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
       render_reference_combo("Favorable SL/TP reference",
                              "##favorable_stop_target_reference",
                              pyramiding.favorable_stop_target_reference()));
      pyramiding.unfavorable_stop_target_reference(
       render_reference_combo("Unfavorable SL/TP reference",
                              "##unfavorable_stop_target_reference",
                              pyramiding.unfavorable_stop_target_reference()));

      position.pyramiding(std::move(pyramiding));
      ImGui::PopID();
    }
    if(ui::collapsible_section("1R Risk Distance", /* default_open= */ false)) {
      ui::section_description(
       "Define the price distance represented by one unit of risk (1R). It is "
       "used by risk-based sizing and performance metrics.");
      ImGui::PushID("risk_distance");
      auto risk_distance = position.risk_distance();
      auto risk_distance_id = get_series_node_id(risk_distance);
      const auto risk_distance_ids = std::array{
       "RISK_DISTANCE.AMOUNT", "RISK_DISTANCE.PERCENT", "RISK_DISTANCE.ATR"};
      ui::field_label("Method");
      if(ImGui::BeginCombo("##risk_distance_method",
                           get_series_node_title(risk_distance_id).c_str())) {
        for(const auto* id : risk_distance_ids) {
          const auto is_selected = risk_distance_id == id;
          if(ImGui::Selectable(get_series_node_title(id).c_str(),
                               is_selected)) {
            risk_distance = get_default_series_node(id);
            risk_distance_id = id;
          }
          if(is_selected) {
            ImGui::SetItemDefaultFocus();
          }
        }
        ImGui::EndCombo();
      }
      self.render_series_node_params(risk_distance, context);
      position.risk_distance(std::move(risk_distance));
      ImGui::PopID();
    }
    {
      const auto stop_losses_header = std::format(
       "Stop Losses ({})###stop_losses_section", position.stop_losses().size());
      if(ui::collapsible_section(stop_losses_header.c_str())) {
        ui::section_description(
         "Stop-loss rules cap downside at a calculated or explicit price. "
         "Trailing stops update as price moves favorably.");
        ImGui::PushID("stop_losses");

        auto stop_losses = position.stop_losses();
        position.stop_losses_activation(self.render_exit_activation(
         "##activation", position.stop_losses_activation()));
        if(ImGui::Button(PLUDUX_ICON_ADD " Add Stop Loss")) {
          stop_losses.emplace_back();
        }

        for(auto index = std::size_t{0}; index < stop_losses.size(); ++index) {
          ImGui::PushID(static_cast<int>(index));
          ImGui::Separator();
          ImGui::Text("Stop Loss %zu", index + 1);

          if(ImGui::Button(PLUDUX_ICON_MOVE_UP " Move Up") && index > 0) {
            std::swap(stop_losses[index], stop_losses[index - 1]);
            ImGui::PopID();
            break;
          }
          ImGui::SameLine();
          if(ImGui::Button(PLUDUX_ICON_MOVE_DOWN " Move Down") &&
             index + 1 < stop_losses.size()) {
            std::swap(stop_losses[index], stop_losses[index + 1]);
            ImGui::PopID();
            break;
          }
          ImGui::SameLine();
          if(ImGui::Button(PLUDUX_ICON_DELETE " Remove")) {
            stop_losses.erase(stop_losses.begin() +
                              static_cast<std::ptrdiff_t>(index));
            ImGui::PopID();
            break;
          }

          auto& stop_loss = stop_losses[index];
          auto enabled = stop_loss.enabled();
          auto trailing = stop_loss.trailing();
          ui::field_label("Enabled");
          ImGui::Checkbox("##enabled", &enabled);
          ui::field_label("Trailing");
          ImGui::Checkbox("##trailing", &trailing);
          stop_loss.enabled(enabled);
          stop_loss.trailing(trailing);

          ImGui::BeginDisabled(!enabled);
          ImGui::PushID("reduce");
          stop_loss.reduce(self.render_position_reduction(stop_loss.reduce()));
          ImGui::PopID();
          ImGui::EndDisabled();

          ui::field_label("Stop Price");
          auto stop_price = stop_loss.stop_price();
          ImGui::PushID("stop_price");
          self.render_series_node(stop_price, context);
          ImGui::PopID();
          stop_loss.stop_price(std::move(stop_price));
          ImGui::PopID();
        }

        position.stop_losses(std::move(stop_losses));
        ImGui::PopID();
      }
    }
    {
      const auto take_profits_header =
       std::format("Take Profits ({})###take_profits_section",
                   position.take_profits().size());
      if(ui::collapsible_section(take_profits_header.c_str())) {
        ui::section_description(
         "Take-profit rules close or reduce a position when a favorable price "
         "target is reached.");
        ImGui::PushID("take_profits");

        auto take_profits = position.take_profits();
        position.take_profits_activation(self.render_exit_activation(
         "##activation", position.take_profits_activation()));
        if(ImGui::Button(PLUDUX_ICON_ADD " Add Take Profit")) {
          take_profits.emplace_back();
        }

        for(auto index = std::size_t{0}; index < take_profits.size(); ++index) {
          ImGui::PushID(static_cast<int>(index));
          ImGui::Separator();
          ImGui::Text("Take Profit %zu", index + 1);

          if(ImGui::Button(PLUDUX_ICON_MOVE_UP " Move Up") && index > 0) {
            std::swap(take_profits[index], take_profits[index - 1]);
            ImGui::PopID();
            break;
          }
          ImGui::SameLine();
          if(ImGui::Button(PLUDUX_ICON_MOVE_DOWN " Move Down") &&
             index + 1 < take_profits.size()) {
            std::swap(take_profits[index], take_profits[index + 1]);
            ImGui::PopID();
            break;
          }
          ImGui::SameLine();
          if(ImGui::Button(PLUDUX_ICON_DELETE " Remove")) {
            take_profits.erase(take_profits.begin() +
                               static_cast<std::ptrdiff_t>(index));
            ImGui::PopID();
            break;
          }

          auto& take_profit = take_profits[index];
          auto enabled = take_profit.enabled();
          ui::field_label("Enabled");
          ImGui::Checkbox("##enabled", &enabled);
          take_profit.enabled(enabled);

          ImGui::BeginDisabled(!enabled);
          ImGui::PushID("reduce");
          take_profit.reduce(
           self.render_position_reduction(take_profit.reduce()));
          ImGui::PopID();
          ImGui::EndDisabled();

          ui::field_label("Target Price");
          auto target_price = take_profit.target_price();
          ImGui::PushID("target_price");
          self.render_series_node(target_price, context);
          ImGui::PopID();
          take_profit.target_price(std::move(target_price));
          ImGui::PopID();
        }

        position.take_profits(std::move(take_profits));
        ImGui::PopID();
      }
    }
  }

  auto render_position_reduction(this auto&, double reduce) -> double
  {
    constexpr auto minimum_percent = 0.01;
    constexpr auto maximum_percent = 100.0;
    auto reduce_percent =
     std::clamp(reduce * 100.0, minimum_percent, maximum_percent);

    ui::field_label("Position Reduction (%)");
    ImGui::SliderScalar("##position_reduction",
                        ImGuiDataType_Double,
                        &reduce_percent,
                        &minimum_percent,
                        &maximum_percent,
                        "%.2f%%",
                        ImGuiSliderFlags_AlwaysClamp);

    return reduce_percent / 100.0;
  }

  void
  render_series_node(this auto& self,
                     ErasedNode<backtest::BacktestMethodContext>& series_node,
                     WindowContext& context)
  {
    auto series_node_id = get_series_node_id(series_node);
    {
      const auto combo_preview_value = get_series_node_title(series_node_id);
      const auto selected =
       ui::searchable_combo("##Series",
                            series_node_id,
                            combo_preview_value,
                            get_series_node_combo_entries());
      if(selected) {
        if(*selected == "SERIES.REFERENCE" &&
           self.available_series_names_.empty() &&
           !self.allow_unlisted_series_names_) {
          const auto error_message = std::format(
           "Cannot select '{}' when there are no available series other "
           "than the current one.",
           get_series_node_title(*selected));
          context.alert(error_message);
        } else {
          series_node = get_default_series_node(*selected);
        }
      }
    }
    {
      ImGui::Indent();
      self.render_series_node_params(series_node, context);
      ImGui::Unindent();
    }
  }

  void
  render_series_node_params(this auto& self,
                            ErasedNode<backtest::BacktestMethodContext>& node,
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
    }.template
     operator()<SelectOutputNode<backtest::BacktestMethodContext>,
                SeriesNode,
                DataNode,
                LookbackNode<backtest::BacktestMethodContext>,
                NumericInputNode,

                AllOfNode<backtest::BacktestMethodContext>,
                AnyOfNode<backtest::BacktestMethodContext>,
                TrueNode,
                FalseNode,
                LessThanNode<backtest::BacktestMethodContext>,
                GreaterThanNode<backtest::BacktestMethodContext>,
                LessEqualNode<backtest::BacktestMethodContext>,
                GreaterEqualNode<backtest::BacktestMethodContext>,
                EqualNode<backtest::BacktestMethodContext>,
                NotEqualNode<backtest::BacktestMethodContext>,
                CrossoverNode<backtest::BacktestMethodContext>,
                CrossunderNode<backtest::BacktestMethodContext>,
                LogicalNotNode<backtest::BacktestMethodContext>,
                LogicalAndNode<backtest::BacktestMethodContext>,
                LogicalOrNode<backtest::BacktestMethodContext>,
                LogicalXorNode<backtest::BacktestMethodContext>,

                BbNode<backtest::BacktestMethodContext>,
                KcNode<backtest::BacktestMethodContext>,
                DonchianChannelNode<backtest::BacktestMethodContext>,
                StochNode<backtest::BacktestMethodContext>,
                StochRsiNode<backtest::BacktestMethodContext>,
                HighestNode<backtest::BacktestMethodContext>,
                LowestNode<backtest::BacktestMethodContext>,
                TrNode<backtest::BacktestMethodContext>,
                SmaNode<backtest::BacktestMethodContext>,
                EmaNode<backtest::BacktestMethodContext>,
                RmaNode<backtest::BacktestMethodContext>,
                WmaNode<backtest::BacktestMethodContext>,
                HmaNode<backtest::BacktestMethodContext>,
                RsiNode<backtest::BacktestMethodContext>,
                RocNode<backtest::BacktestMethodContext>,
                RvolNode<backtest::BacktestMethodContext>,

                MacdNode<backtest::BacktestMethodContext>,
                AtrNode<backtest::BacktestMethodContext>,
                ValueNode,
                ChangeNode<backtest::BacktestMethodContext>,
                AddNode<backtest::BacktestMethodContext>,
                SubtractNode<backtest::BacktestMethodContext>,
                MultiplyNode<backtest::BacktestMethodContext>,
                DivideNode<backtest::BacktestMethodContext>,
                PercentageNode<backtest::BacktestMethodContext>,
                RiskDistanceAmountNode<backtest::BacktestMethodContext>,
                RiskDistancePercentNode<backtest::BacktestMethodContext>,
                RiskDistanceAtrNode<backtest::BacktestMethodContext>,
                SlAmountNode<backtest::BacktestMethodContext>,
                TpAmountNode<backtest::BacktestMethodContext>,
                SlPercentNode<backtest::BacktestMethodContext>,
                TpPercentNode<backtest::BacktestMethodContext>,
                SlAtrNode<backtest::BacktestMethodContext>,
                TpAtrNode<backtest::BacktestMethodContext>,
                Sl1RNode,
                SlRMultipleNode<backtest::BacktestMethodContext>,
                TpRMultipleNode<backtest::BacktestMethodContext>,
                InitialEntryPriceNode,
                LatestEntryPriceNode,
                AveragePriceNode,
                StopTargetRefPriceNode,
                PositionDirectionNode,
                PyramidingLayerNode,
                RequestedOrderPriceNode,
                RequestedOrderDirectionNode,
                IsPyramidingOrderNode,
                RawRequestedQuantityNode,
                RawRequestedQuantityLimitNode,
                DrawdownAdjustedQuantityNode,
                DrawdownAdjustedQuantityLimitNode,
                RequestedQuantityNode,
                RequestedNotionalNode,
                RequestedCostNode,
                EstimatedEntryFeeNode,
                EstimatedOneRExitFeeNode,
                RequestedOrderRiskDistanceNode,
                RequestedPriceRiskNode,
                RequestedRiskWithFeesNode,
                FrozenUnitQuantityNode,
                AbsDiffNode<backtest::BacktestMethodContext>,
                AbsNode<backtest::BacktestMethodContext>,
                MaxNode<backtest::BacktestMethodContext>,
                MinNode<backtest::BacktestMethodContext>,
                PositivePartNode<backtest::BacktestMethodContext>,
                NegativePartNode<backtest::BacktestMethodContext>,
                NegateNode<backtest::BacktestMethodContext>,
                SqrtNode<backtest::BacktestMethodContext>,
                StddevNode<backtest::BacktestMethodContext>>());
  }

  void render_series_node_params(
   this auto& self,
   SelectOutputNode<backtest::BacktestMethodContext>& node,
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

      ui::field_label("Output");
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
      ui::field_label("Source");
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
    if(self.allow_unlisted_series_names_) {
      ui::field_label("Name");
      auto name = node.name();
      if(ImGui::InputTextWithHint(
          "##named_series", "Model series name", &name)) {
        node.name(std::move(name));
      }
      return;
    }
    if(self.changed_series_names_.contains(node.name())) {
      const auto new_name = self.changed_series_names_.at(node.name());
      node.name(new_name);
    } else if(std::ranges::find(self.available_series_names_, node.name()) ==
              self.available_series_names_.end()) {
      node.name("");
    }

    ui::field_label("Name");

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
                                 BbNode<backtest::BacktestMethodContext>& node,
                                 WindowContext& context)
  {
    ui::field_label("MA Type");
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

    ui::field_label("Period");
    auto period = node.period();
    ImGui::PushID("period");
    self.render_series_node(period, context);
    ImGui::PopID();
    node.period(std::move(period));

    ui::field_label("StdDev");
    auto stddev = node.stddev();
    ImGui::PushID("stddev");
    self.render_series_node(stddev, context);
    ImGui::PopID();
    node.stddev(std::move(stddev));

    ui::field_label("Source");
    auto source = node.source();
    ImGui::PushID("source");
    self.render_series_node(source, context);
    ImGui::PopID();
    node.source(std::move(source));
  }

  void render_series_node_params(this auto& self,
                                 KcNode<backtest::BacktestMethodContext>& node,
                                 WindowContext& context)
  {
    {
      ui::field_label("Length");
      auto period = node.period();
      ImGui::PushID("period");
      self.render_series_node(period, context);
      ImGui::PopID();
      node.period(std::move(period));
    }
    {
      ui::field_label("MA Type");
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
      ui::field_label("Source");
      auto source = node.source();
      ImGui::PushID("source");
      self.render_series_node(source, context);
      ImGui::PopID();
      node.source(std::move(source));
    }
    {
      ui::field_label("Band Type");
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
      ui::field_label("ATR Length");
      auto band_atr_period = node.band_atr_period();
      ImGui::PushID("band_atr_period");
      self.render_series_node(band_atr_period, context);
      ImGui::PopID();
      node.band_atr_period(std::move(band_atr_period));
    }
    {
      ui::field_label("Multiplier");
      auto multiplier = node.multiplier();
      ImGui::PushID("multiplier");
      self.render_series_node(multiplier, context);
      ImGui::PopID();
      node.multiplier(std::move(multiplier));
    }
  }

  void render_series_node_params(
   this auto& self,
   DonchianChannelNode<backtest::BacktestMethodContext>& node,
   WindowContext& context)
  {
    {
      ui::field_label("Length");
      auto period = node.period();
      ImGui::PushID("period");
      self.render_series_node(period, context);
      ImGui::PopID();
      node.period(std::move(period));
    }
  }

  void
  render_series_node_params(this auto& self,
                            StochNode<backtest::BacktestMethodContext>& node,
                            WindowContext& context)
  {
    {
      ui::field_label("D Period");
      auto d_period = node.d_period();
      ImGui::PushID("d_period");
      self.render_series_node(d_period, context);
      ImGui::PopID();
      node.d_period(std::move(d_period));
    }
    {
      ui::field_label("K Period");
      auto k_period = node.k_period();
      ImGui::PushID("k_period");
      self.render_series_node(k_period, context);
      ImGui::PopID();
      node.k_period(std::move(k_period));
    }
    {
      ui::field_label("K Smooth");
      auto k_smooth = node.k_smooth();
      ImGui::PushID("k_smooth");
      self.render_series_node(k_smooth, context);
      ImGui::PopID();
      node.k_smooth(std::move(k_smooth));
    }
  }

  void
  render_series_node_params(this auto& self,
                            StochRsiNode<backtest::BacktestMethodContext>& node,
                            WindowContext& context)
  {
    {
      ui::field_label("D Period");
      auto d_period = node.d_period();
      ImGui::PushID("d_period");
      self.render_series_node(d_period, context);
      ImGui::PopID();
      node.d_period(std::move(d_period));
    }
    {
      ui::field_label("K Period");
      auto k_period = node.k_period();
      ImGui::PushID("k_period");
      self.render_series_node(k_period, context);
      ImGui::PopID();
      node.k_period(std::move(k_period));
    }
    {
      ui::field_label("K Smooth");
      auto k_smooth = node.k_smooth();
      ImGui::PushID("k_smooth");
      self.render_series_node(k_smooth, context);
      ImGui::PopID();
      node.k_smooth(std::move(k_smooth));
    }
    {
      ui::field_label("RSI Period");
      auto rsi_period = node.rsi_period();
      ImGui::PushID("rsi_period");
      self.render_series_node(rsi_period, context);
      ImGui::PopID();
      node.rsi_period(std::move(rsi_period));
    }
    {
      ui::field_label("RSI Source");
      auto rsi_source = node.rsi_source();
      ImGui::PushID("rsi_source");
      self.render_series_node(rsi_source, context);
      ImGui::PopID();
      node.rsi_source(std::move(rsi_source));
    }
  }

  void
  render_series_node_params(this auto& self,
                            RvolNode<backtest::BacktestMethodContext>& node,
                            WindowContext& context)
  {
    ui::field_label("Period");
    auto period = node.period();
    ImGui::PushID("period");
    self.render_series_node(period, context);
    ImGui::PopID();
    node.period(std::move(period));
  }

  template<typename TConditionNode>
    requires std::same_as<TConditionNode,
                          AllOfNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode,
                          AnyOfNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode, TrueNode> ||
             std::same_as<TConditionNode, FalseNode> ||
             std::same_as<TConditionNode,
                          LessThanNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode,
                          GreaterThanNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode,
                          LessEqualNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode,
                          GreaterEqualNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode,
                          EqualNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode,
                          NotEqualNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode,
                          CrossoverNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode,
                          CrossunderNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode,
                          LogicalNotNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode,
                          LogicalAndNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode,
                          LogicalOrNode<backtest::BacktestMethodContext>> ||
             std::same_as<TConditionNode,
                          LogicalXorNode<backtest::BacktestMethodContext>>
  void render_series_node_params(this auto& self,
                                 TConditionNode& node,
                                 WindowContext& context)
  {
    auto changed_node = self.render_condition_node(
     ErasedNode<backtest::BacktestMethodContext>{node}, context);
    if(const auto* updated = node_cast<TConditionNode>(changed_node)) {
      node = *updated;
    }
  }

  void render_series_node_params(this auto& self,
                                 TrNode<backtest::BacktestMethodContext>& node,
                                 WindowContext& context)
  {
    ImGui::TextUnformatted("No parameters.");
  }

  template<typename TNodeWithPeriod>
    requires std::same_as<TNodeWithPeriod,
                          SmaNode<backtest::BacktestMethodContext>> ||
             std::same_as<TNodeWithPeriod,
                          EmaNode<backtest::BacktestMethodContext>> ||
             std::same_as<TNodeWithPeriod,
                          RmaNode<backtest::BacktestMethodContext>> ||
             std::same_as<TNodeWithPeriod,
                          WmaNode<backtest::BacktestMethodContext>> ||
             std::same_as<TNodeWithPeriod,
                          HmaNode<backtest::BacktestMethodContext>> ||
             std::same_as<TNodeWithPeriod,
                          RsiNode<backtest::BacktestMethodContext>> ||
             std::same_as<TNodeWithPeriod,
                          RocNode<backtest::BacktestMethodContext>> ||
             std::same_as<TNodeWithPeriod,
                          HighestNode<backtest::BacktestMethodContext>> ||
             std::same_as<TNodeWithPeriod,
                          LowestNode<backtest::BacktestMethodContext>> ||
             std::same_as<TNodeWithPeriod,
                          StddevNode<backtest::BacktestMethodContext>>
  void render_series_node_params(this auto& self,
                                 TNodeWithPeriod& node,
                                 WindowContext& context)
  {
    ui::field_label("Period");
    auto period = node.period();
    ImGui::PushID("period");
    self.render_series_node(period, context);
    ImGui::PopID();
    node.period(std::move(period));

    ui::field_label("Source");
    auto source = node.source();
    ImGui::PushID("source");
    self.render_series_node(source, context);
    ImGui::PopID();
    node.source(std::move(source));
  }

  template<typename TBinaryOpNode>
    requires std::same_as<TBinaryOpNode,
                          AddNode<backtest::BacktestMethodContext>> ||
             std::same_as<TBinaryOpNode,
                          SubtractNode<backtest::BacktestMethodContext>> ||
             std::same_as<TBinaryOpNode,
                          MultiplyNode<backtest::BacktestMethodContext>> ||
             std::same_as<TBinaryOpNode,
                          DivideNode<backtest::BacktestMethodContext>> ||
             std::same_as<TBinaryOpNode,
                          AbsDiffNode<backtest::BacktestMethodContext>> ||
             std::same_as<TBinaryOpNode,
                          MaxNode<backtest::BacktestMethodContext>> ||
             std::same_as<TBinaryOpNode,
                          MinNode<backtest::BacktestMethodContext>>
  void render_series_node_params(this auto& self,
                                 TBinaryOpNode& node,
                                 WindowContext& context)
  {
    {
      ui::field_label("Left");
      auto left = node.left();
      ImGui::PushID("left");
      self.render_series_node(left, context);
      ImGui::PopID();
      node.left(std::move(left));
    }

    {
      ui::field_label("Right");
      auto right = node.right();
      ImGui::PushID("right");
      self.render_series_node(right, context);
      ImGui::PopID();
      node.right(std::move(right));
    }
  }

  template<typename TUnaryOpNode>
    requires std::same_as<TUnaryOpNode,
                          NegateNode<backtest::BacktestMethodContext>> ||
             std::same_as<TUnaryOpNode,
                          AbsNode<backtest::BacktestMethodContext>> ||
             std::same_as<TUnaryOpNode,
                          SqrtNode<backtest::BacktestMethodContext>> ||
             std::same_as<TUnaryOpNode,
                          PositivePartNode<backtest::BacktestMethodContext>> ||
             std::same_as<TUnaryOpNode,
                          NegativePartNode<backtest::BacktestMethodContext>>
  void render_series_node_params(this auto& self,
                                 TUnaryOpNode& node,
                                 WindowContext& context)
  {
    ui::field_label("Value");
    auto value = node.operand();
    ImGui::PushID("value");
    self.render_series_node(value, context);
    ImGui::PopID();
    node.operand(std::move(value));
  }

  void render_series_node_params(
   this auto& self,
   PercentageNode<backtest::BacktestMethodContext>& node,
   WindowContext& context)
  {
    {
      ui::field_label("Percent");
      auto percent = node.percent();
      ImGui::PushID("percent");
      if(ImGui::InputDouble("##percent", &percent, 0.1, 1.0, "%.2f")) {
        node.percent(std::move(percent));
      }
      ImGui::PopID();
    }

    {
      ui::field_label("Base");
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
    ui::field_label(label);
    auto value = node.value();
    ImGui::PushID(label);
    self.render_series_node(value, context);
    ImGui::PopID();
    node.value(std::move(value));
  }

  void render_series_node_params(
   this auto& self,
   RiskDistanceAmountNode<backtest::BacktestMethodContext>& node,
   WindowContext& context)
  {
    self.render_stop_target_value_node_params(node, context, "Amount");
  }

  void render_series_node_params(
   this auto& self,
   RiskDistancePercentNode<backtest::BacktestMethodContext>& node,
   WindowContext& context)
  {
    self.render_stop_target_value_node_params(node, context, "Percentage");
  }

  void
  render_series_node_params(this auto& self,
                            SlAmountNode<backtest::BacktestMethodContext>& node,
                            WindowContext& context)
  {
    self.render_stop_target_value_node_params(node, context, "Amount");
  }

  void
  render_series_node_params(this auto& self,
                            TpAmountNode<backtest::BacktestMethodContext>& node,
                            WindowContext& context)
  {
    self.render_stop_target_value_node_params(node, context, "Amount");
  }

  void render_series_node_params(
   this auto& self,
   SlPercentNode<backtest::BacktestMethodContext>& node,
   WindowContext& context)
  {
    self.render_stop_target_value_node_params(node, context, "Percent");
  }

  void render_series_node_params(
   this auto& self,
   TpPercentNode<backtest::BacktestMethodContext>& node,
   WindowContext& context)
  {
    self.render_stop_target_value_node_params(node, context, "Percent");
  }

  void render_series_node_params(this auto&, Sl1RNode&, WindowContext&)
  {
  }

  void render_series_node_params(
   this auto& self,
   SlRMultipleNode<backtest::BacktestMethodContext>& node,
   WindowContext& context)
  {
    self.render_stop_target_value_node_params(node, context, "Multiple");
  }

  void render_series_node_params(
   this auto& self,
   TpRMultipleNode<backtest::BacktestMethodContext>& node,
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

  void
  render_series_node_params(this auto&, PyramidingLayerNode&, WindowContext&)
  {
  }

  template<typename TNode>
    requires std::same_as<TNode, RequestedOrderPriceNode> ||
             std::same_as<TNode, RequestedOrderDirectionNode> ||
             std::same_as<TNode, IsPyramidingOrderNode> ||
             std::same_as<TNode, RawRequestedQuantityNode> ||
             std::same_as<TNode, RawRequestedQuantityLimitNode> ||
             std::same_as<TNode, DrawdownAdjustedQuantityNode> ||
             std::same_as<TNode, DrawdownAdjustedQuantityLimitNode> ||
             std::same_as<TNode, RequestedQuantityNode> ||
             std::same_as<TNode, RequestedNotionalNode> ||
             std::same_as<TNode, RequestedCostNode> ||
             std::same_as<TNode, EstimatedEntryFeeNode> ||
             std::same_as<TNode, EstimatedOneRExitFeeNode> ||
             std::same_as<TNode, RequestedOrderRiskDistanceNode> ||
             std::same_as<TNode, RequestedPriceRiskNode> ||
             std::same_as<TNode, RequestedRiskWithFeesNode> ||
             std::same_as<TNode, FrozenUnitQuantityNode>
  void render_series_node_params(this auto&, TNode&, WindowContext&)
  {
  }

  void render_series_node_params(
   this auto& self,
   PositionRMultipleNode<backtest::BacktestMethodContext>& node,
   WindowContext& context)
  {
    ui::field_label("Source");
    auto source = node.source();
    ImGui::PushID("source");
    self.render_series_node(source, context);
    ImGui::PopID();
    node.source(std::move(source));
  }

  void
  render_series_node_params(this auto& self,
                            ChangeNode<backtest::BacktestMethodContext>& node,
                            WindowContext& context)
  {
    ui::field_label("Source");
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
    ui::field_label("Field");
    auto field = node.field();
    if(ImGui::InputText("##field", &field)) {
      node.field(field);
    }
  }

  void
  render_series_node_params(this auto& self,
                            MacdNode<backtest::BacktestMethodContext>& node,
                            WindowContext& context)
  {
    {
      ui::field_label("Fast Period");
      auto fast_period = node.fast_period();
      ImGui::PushID("fast_period");
      self.render_series_node(fast_period, context);
      ImGui::PopID();
      node.fast_period(std::move(fast_period));
    }
    {
      ui::field_label("Slow Period");
      auto slow_period = node.slow_period();
      ImGui::PushID("slow_period");
      self.render_series_node(slow_period, context);
      ImGui::PopID();
      node.slow_period(std::move(slow_period));
    }
    {
      ui::field_label("Signal Period");
      auto signal_period = node.signal_period();
      ImGui::PushID("signal_period");
      self.render_series_node(signal_period, context);
      ImGui::PopID();
      node.signal_period(std::move(signal_period));
    }
    {
      ui::field_label("Source");
      auto source = node.source();
      ImGui::PushID("source");
      self.render_series_node(source, context);
      ImGui::PopID();
      node.source(std::move(source));
    }
  }

  void render_series_node_params(this auto& self,
                                 AtrNode<backtest::BacktestMethodContext>& node,
                                 WindowContext& context)
  {
    ui::field_label("Period");
    auto period = node.period();
    ImGui::PushID("period");
    self.render_series_node(period, context);
    ImGui::PopID();
    node.period(std::move(period));

    ui::field_label("Smoothing");

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
    ui::field_label("Period");
    auto period = node.period();
    ImGui::PushID("period");
    self.render_series_node(period, context);
    ImGui::PopID();
    node.period(std::move(period));

    ui::field_label("Multiplier");
    auto multiplier = node.multiplier();
    ImGui::PushID("multiplier");
    self.render_series_node(multiplier, context);
    ImGui::PopID();
    node.multiplier(std::move(multiplier));

    ui::field_label("Smoothing");

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

  void render_series_node_params(
   this auto& self,
   RiskDistanceAtrNode<backtest::BacktestMethodContext>& node,
   WindowContext& context)
  {
    self.render_stop_target_atr_node_params(node, context);
  }

  void
  render_series_node_params(this auto& self,
                            SlAtrNode<backtest::BacktestMethodContext>& node,
                            WindowContext& context)
  {
    self.render_stop_target_atr_node_params(node, context);
  }

  void
  render_series_node_params(this auto& self,
                            TpAtrNode<backtest::BacktestMethodContext>& node,
                            WindowContext& context)
  {
    self.render_stop_target_atr_node_params(node, context);
  }

  void render_series_node_params(this auto& self,
                                 ValueNode& node,
                                 WindowContext& context)
  {
    ui::field_label("Value");
    auto value = node.value();
    if(ImGui::InputDouble("##value", &value, 0.1, 1.0, "%.2f")) {
      node.value(value);
    }
  }

  void
  render_series_node_params(this auto& self,
                            LookbackNode<backtest::BacktestMethodContext>& node,
                            WindowContext& context)
  {
    ui::field_label("Periods");
    auto periods = static_cast<int>(node.period());
    if(ImGui::InputInt("##lookback_periods", &periods)) {
      if(periods < 1) {
        periods = 1;
      }
      node.period(static_cast<std::size_t>(periods));
    }
    {
      ui::field_label("Source");
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
    ui::field_label(
     "Input label",
     "This label identifies the adjustable value in each strategy that uses "
     "the model.");

    auto input_label = node.label();
    if(ImGui::InputTextWithHint(
        "##input_label", "Shown in the Strategy editor", &input_label)) {
      node.label(std::move(input_label));
    }

    auto input_representation = node.representation();
    auto formatted_value = node.value();
    auto value_changed = false;

    constexpr std::array<const char*, 3> input_types{
     "Decimal", "Signed Integer", "Unsigned Integer"};
    auto selected_type_index = static_cast<int>(input_representation);

    ui::field_label(
     "Number format",
     "Choose Decimal for fractional values, or an integer format for whole "
     "numbers only.");
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

    ui::field_label("Value");
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

  auto draw_condition_node_combo(
   this auto& self,
   const ErasedNode<backtest::BacktestMethodContext>& condition) -> std::string
  {
    auto result_condition_id = get_condition_node_id(condition);

    ImGui::PushID(result_condition_id.c_str());
    {
      const auto combo_preview_value =
       get_condition_node_title(result_condition_id);
      if(const auto selected =
          ui::searchable_combo("##Conditions",
                               result_condition_id,
                               combo_preview_value,
                               get_condition_node_combo_entries())) {
        result_condition_id = *selected;
      }
    }

    ImGui::PopID();

    return result_condition_id;
  }

  auto make_condition_node_from_other(this auto& self,
                                      const std::string& condition_id,
                                      auto other_condition)
   -> ErasedNode<backtest::BacktestMethodContext>
  {
    const auto get_condition_series_params =
     []<typename TNode>(const TNode& node)
     -> std::pair<ErasedNode<backtest::BacktestMethodContext>,
                  ErasedNode<backtest::BacktestMethodContext>> {
      if constexpr(requires {
                     {
                       node.source()
                     } -> std::convertible_to<
                      ErasedNode<backtest::BacktestMethodContext>>;
                     {
                       node.reference()
                     } -> std::convertible_to<
                      ErasedNode<backtest::BacktestMethodContext>>;
                   }) {
        return {node.source(), node.reference()};
      } else if constexpr(requires {
                            {
                              node.target()
                            } -> std::convertible_to<
                             ErasedNode<backtest::BacktestMethodContext>>;
                            {
                              node.threshold()
                            } -> std::convertible_to<
                             ErasedNode<backtest::BacktestMethodContext>>;
                          }) {
        return {node.target(), node.threshold()};
      } else {
        return {CloseNode{}, CloseNode{}};
      }
    };

    if(condition_id == "COMPARISON.EQUAL") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return EqualNode<backtest::BacktestMethodContext>{
       std::move(lhs_series_param), std::move(rhs_series_param)};
    }

    if(condition_id == "COMPARISON.NOT_EQUAL") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return NotEqualNode<backtest::BacktestMethodContext>{
       std::move(lhs_series_param), std::move(rhs_series_param)};
    }

    if(condition_id == "COMPARISON.GREATER_THAN") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return GreaterThanNode<backtest::BacktestMethodContext>{
       std::move(lhs_series_param), std::move(rhs_series_param)};
    }

    if(condition_id == "COMPARISON.LESS_THAN") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return LessThanNode<backtest::BacktestMethodContext>{
       std::move(lhs_series_param), std::move(rhs_series_param)};
    }

    if(condition_id == "COMPARISON.GREATER_EQUAL") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return GreaterEqualNode<backtest::BacktestMethodContext>{
       std::move(lhs_series_param), std::move(rhs_series_param)};
    }

    if(condition_id == "COMPARISON.LESS_EQUAL") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return LessEqualNode<backtest::BacktestMethodContext>{
       std::move(lhs_series_param), std::move(rhs_series_param)};
    }

    if(condition_id == "COMPARISON.CROSSOVER") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return CrossoverNode<backtest::BacktestMethodContext>{
       std::move(lhs_series_param), std::move(rhs_series_param)};
    }

    if(condition_id == "COMPARISON.CROSSUNDER") {
      auto [lhs_series_param, rhs_series_param] =
       get_condition_series_params(other_condition);
      return CrossunderNode<backtest::BacktestMethodContext>{
       std::move(lhs_series_param), std::move(rhs_series_param)};
    }

    const auto get_conditions_param = []<typename TNode>(const TNode& node)
     -> std::vector<ErasedNode<backtest::BacktestMethodContext>> {
      if constexpr(requires {
                     {
                       node.conditions()
                     } -> std::convertible_to<
                      std::vector<ErasedNode<backtest::BacktestMethodContext>>>;
                   }) {
        return node.conditions();
      } else if constexpr(requires {
                            {
                              node.other_condition()
                            } -> std::convertible_to<
                             ErasedNode<backtest::BacktestMethodContext>>;
                          }) {
        return {node.other_condition()};
      } else if constexpr(requires {
                            {
                              node.first_condition()
                            } -> std::convertible_to<
                             ErasedNode<backtest::BacktestMethodContext>>;
                            {
                              node.second_condition()
                            } -> std::convertible_to<
                             ErasedNode<backtest::BacktestMethodContext>>;
                          }) {
        return {node.first_condition(), node.second_condition()};
      } else {
        return {node};
      }
    };

    if(condition_id == "LOGIC.ALL_OF") {
      const auto conditions = get_conditions_param(other_condition);
      return AllOfNode<backtest::BacktestMethodContext>{conditions};
    }

    if(condition_id == "LOGIC.ANY_OF") {
      const auto conditions = get_conditions_param(other_condition);
      return AnyOfNode<backtest::BacktestMethodContext>{conditions};
    }

    const auto get_first_condition_param =
     []<typename TNode>(
      const TNode& node) -> ErasedNode<backtest::BacktestMethodContext> {
      if constexpr(requires {
                     {
                       node.condition()
                     } -> std::convertible_to<
                      ErasedNode<backtest::BacktestMethodContext>>;
                   }) {
        return node.condition();
      } else if constexpr(requires {
                            {
                              node.first_condition()
                            } -> std::convertible_to<
                             ErasedNode<backtest::BacktestMethodContext>>;
                          }) {
        return node.first_condition();
      } else if constexpr(requires {
                            {
                              node.conditions()
                            } -> std::convertible_to<std::vector<
                             ErasedNode<backtest::BacktestMethodContext>>>;
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
                            } -> std::convertible_to<
                             ErasedNode<backtest::BacktestMethodContext>>;
                          }) {
        return node.other_condition();
      } else {
        return node;
      }
    };

    const auto get_second_condition_param =
     []<typename TNode>(
      const TNode& node) -> ErasedNode<backtest::BacktestMethodContext> {
      if constexpr(requires {
                     {
                       node.other_condition()
                     } -> std::convertible_to<
                      ErasedNode<backtest::BacktestMethodContext>>;
                   }) {
        return node.other_condition();
      } else if constexpr(requires {
                            {
                              node.second_condition()
                            } -> std::convertible_to<
                             ErasedNode<backtest::BacktestMethodContext>>;
                          }) {
        return node.second_condition();
      } else {
        return node;
      }
    };

    if(condition_id == "LOGIC.NOT") {
      return LogicalNotNode<backtest::BacktestMethodContext>{
       get_first_condition_param(other_condition)};
    }

    if(condition_id == "LOGIC.AND") {
      return LogicalAndNode<backtest::BacktestMethodContext>{
       get_first_condition_param(other_condition),
       get_second_condition_param(other_condition)};
    }

    if(condition_id == "LOGIC.OR") {
      return LogicalOrNode<backtest::BacktestMethodContext>{
       get_first_condition_param(other_condition),
       get_second_condition_param(other_condition)};
    }

    if(condition_id == "LOGIC.XOR") {
      return LogicalXorNode<backtest::BacktestMethodContext>{
       get_first_condition_param(other_condition),
       get_second_condition_param(other_condition)};
    }

    if(condition_id == "LOGIC.ALWAYS") {
      return TrueNode{};
    }

    if(condition_id == "LOGIC.NEVER") {
      return FalseNode{};
    }

    return FalseNode{};
  }

  auto render_condition_node(
   this auto& self,
   const ErasedNode<backtest::BacktestMethodContext>& any_condition,
   WindowContext& context) -> ErasedNode<backtest::BacktestMethodContext>
  {
    if(auto* condition_ptr =
        node_cast<AllOfNode<backtest::BacktestMethodContext>>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<AnyOfNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<TrueNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr = node_cast<FalseNode>(any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<LessThanNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<GreaterThanNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<LessEqualNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<GreaterEqualNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<EqualNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<NotEqualNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<CrossoverNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<CrossunderNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<LogicalNotNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<LogicalAndNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<LogicalOrNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else if(auto* condition_ptr =
               node_cast<LogicalXorNode<backtest::BacktestMethodContext>>(
                any_condition)) {
      return self.render_condition_node(*condition_ptr, context);
    } else {
      return any_condition;
    }
  }

  template<typename TCondition>
    requires std::same_as<TCondition,
                          EqualNode<backtest::BacktestMethodContext>> ||
             std::same_as<TCondition,
                          NotEqualNode<backtest::BacktestMethodContext>> ||
             std::same_as<TCondition,
                          GreaterThanNode<backtest::BacktestMethodContext>> ||
             std::same_as<TCondition,
                          LessThanNode<backtest::BacktestMethodContext>> ||
             std::same_as<TCondition,
                          GreaterEqualNode<backtest::BacktestMethodContext>> ||
             std::same_as<TCondition,
                          LessEqualNode<backtest::BacktestMethodContext>>
  auto render_condition_node(this auto& self,
                             const TCondition& condition,
                             WindowContext& context)
   -> ErasedNode<backtest::BacktestMethodContext>
  {
    auto new_condition = condition;

    auto target = new_condition.target();
    ui::field_label("Left value");
    ImGui::PushID("left_param");
    self.render_series_node(target, context);
    ImGui::PopID();

    ui::field_label("Comparison");
    const auto updated_condition_id =
     self.draw_condition_node_combo(new_condition);

    auto threshold = new_condition.threshold();
    ui::field_label("Right value");
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
    requires std::same_as<TCondition,
                          CrossoverNode<backtest::BacktestMethodContext>> ||
             std::same_as<TCondition,
                          CrossunderNode<backtest::BacktestMethodContext>>
  auto render_condition_node(this auto& self,
                             const TCondition& condition,
                             WindowContext& context)
   -> ErasedNode<backtest::BacktestMethodContext>
  {
    auto new_condition = condition;

    auto source = new_condition.source();
    ui::field_label("Source");
    ImGui::PushID("left_param");
    self.render_series_node(source, context);
    ImGui::PopID();

    ui::field_label("Crossing rule");
    const auto updated_condition_id =
     self.draw_condition_node_combo(new_condition);

    auto reference = new_condition.reference();
    ui::field_label("Reference");
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
                             WindowContext& context)
   -> ErasedNode<backtest::BacktestMethodContext>
  {
    auto new_condition = condition;
    ui::field_label("Condition");
    const auto updated_condition_id =
     self.draw_condition_node_combo(new_condition);

    auto changed_node = self.make_condition_node_from_other(
     updated_condition_id, std::move(new_condition));

    return changed_node;
  }

  template<typename TCondition>
    requires std::same_as<TCondition,
                          AllOfNode<backtest::BacktestMethodContext>> ||
             std::same_as<TCondition,
                          AnyOfNode<backtest::BacktestMethodContext>>
  auto render_condition_node(this auto& self,
                             const TCondition& condition,
                             WindowContext& context)
   -> ErasedNode<backtest::BacktestMethodContext>
  {
    auto new_condition = condition;
    ui::field_label("Condition group");
    const auto updated_condition_id =
     self.draw_condition_node_combo(new_condition);
    ImGui::Indent();

    auto conditions = new_condition.conditions();
    for(auto i = 0; i < conditions.size(); ++i) {
      ImGui::PushID(i);
      auto& sub_condition = conditions[i];
      const auto condition_heading = std::format("Condition {}", i + 1);
      ImGui::SeparatorText(condition_heading.c_str());
      sub_condition = self.render_condition_node(sub_condition, context);

      if(ui::right_aligned_button(PLUDUX_ICON_DELETE " Remove Condition")) {
        conditions.erase(conditions.begin() + i);
        --i;
      }

      ImGui::Separator();
      ImGui::PopID();
    }

    if(ImGui::Button(PLUDUX_ICON_ADD " Add Condition")) {
      conditions.emplace_back(FalseNode{});
    }

    new_condition = TCondition(std::move(conditions));

    ImGui::Unindent();

    auto changed_node = self.make_condition_node_from_other(
     updated_condition_id, std::move(new_condition));

    return changed_node;
  }

  template<typename TCondition>
    requires std::same_as<TCondition,
                          LogicalNotNode<backtest::BacktestMethodContext>>
  auto render_condition_node(this auto& self,
                             const TCondition& condition,
                             WindowContext& context)
   -> ErasedNode<backtest::BacktestMethodContext>
  {
    auto new_condition = condition;
    ui::field_label("Condition");
    const auto updated_condition_id =
     self.draw_condition_node_combo(new_condition);
    ImGui::Indent();
    ImGui::SeparatorText("Nested condition");
    auto sub_condition = new_condition.other_condition();
    sub_condition = self.render_condition_node(sub_condition, context);
    new_condition.other_condition(std::move(sub_condition));
    ImGui::Unindent();
    auto changed_node = self.make_condition_node_from_other(
     updated_condition_id, std::move(new_condition));
    return changed_node;
  }

  template<typename TCondition>
    requires std::same_as<TCondition,
                          LogicalAndNode<backtest::BacktestMethodContext>> ||
             std::same_as<TCondition,
                          LogicalOrNode<backtest::BacktestMethodContext>> ||
             std::same_as<TCondition,
                          LogicalXorNode<backtest::BacktestMethodContext>>
  auto render_condition_node(this auto& self,
                             const TCondition& condition,
                             WindowContext& context)
   -> ErasedNode<backtest::BacktestMethodContext>
  {
    auto new_condition = condition;
    auto first_condition = new_condition.first_condition();
    auto second_condition = new_condition.second_condition();

    {
      ImGui::PushID("first_condition");
      ImGui::SeparatorText("First condition");
      first_condition = self.render_condition_node(first_condition, context);
      ImGui::PopID();
    }

    ui::field_label("Logical operator");
    const auto updated_condition_id =
     self.draw_condition_node_combo(new_condition);

    {
      ImGui::PushID("second_condition");
      ImGui::SeparatorText("Second condition");
      second_condition = self.render_condition_node(second_condition, context);
      ImGui::PopID();
    }

    new_condition.first_condition(std::move(first_condition));
    new_condition.second_condition(std::move(second_condition));

    auto changed_node = self.make_condition_node_from_other(
     updated_condition_id, std::move(new_condition));
    return changed_node;
  }

  void
  render_plot_method(this auto& self,
                     ErasedPlotMethod<ErasedPlotMethodContext>& plot_method,
                     WindowContext& context)
  {
    static const auto entries = [] {
      auto result = std::vector<ui::ComboEntry>{};
      for(const auto& id : {"PLOT.LINE",
                            "PLOT.HLINE",
                            "PLOT.HISTOGRAM",
                            "PLOT.MOMENTUM_HISTOGRAM"}) {
        result.push_back(ui::ComboEntry{
         .id = id, .title = get_plot_method_title(id), .category = ""});
      }
      return result;
    }();

    const auto plot_method_id = get_plot_method_id(plot_method);
    const auto combo_preview_value = get_plot_method_title(plot_method_id);
    if(const auto selected = ui::searchable_combo(
        "##plot_method", plot_method_id, combo_preview_value, entries)) {
      plot_method = get_default_plot_method(*selected);
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
      } else if(auto* method_ptr =
                 plot_method_cast<MomentumHistogramPlotMethod>(plot_method)) {
        self.render_plot_method(*method_ptr, context);
      }
      ImGui::Unindent();
    }
  }

  void render_plot_method(this auto& self,
                          HLinePlotMethod& plot_method,
                          WindowContext& context)
  {
    ui::field_label("Level");
    auto level = plot_method.level();
    if(ImGui::InputDouble("##hline_level", &level, 0.1, 1.0, "%.2f")) {
      plot_method.level(level);
    }

    ui::field_label("Color");
    const auto color_vec = ImGui::ColorConvertU32ToFloat4(plot_method.color());
    auto color = std::array{color_vec.x, color_vec.y, color_vec.z, color_vec.w};
    if(ImGui::ColorEdit4("##hline_color", color.data())) {
      plot_method.color(ImGui::ColorConvertFloat4ToU32(
       {color[0], color[1], color[2], color[3]}));
    }
  }

  template<typename TPlotMethod>
    requires std::same_as<TPlotMethod, LinePlotMethod> ||
             std::same_as<TPlotMethod, HistogramPlotMethod> ||
             std::same_as<TPlotMethod, MomentumHistogramPlotMethod>
  void render_plot_method(this auto& self,
                          TPlotMethod& plot_method,
                          WindowContext& context)
  {
    static const auto source_entries = [] {
      auto result = std::vector<ui::ComboEntry>{};
      for(const auto& id : {"PLOT_SOURCE.SERIES", "PLOT_SOURCE.CONSTANT"}) {
        result.push_back(ui::ComboEntry{
         .id = id, .title = get_plot_source_method_title(id), .category = ""});
      }
      return result;
    }();

    ui::field_label("Source");
    {
      const auto plot_source_id =
       get_plot_source_method_id(plot_method.source());
      const auto combo_preview_value =
       get_plot_source_method_title(plot_source_id);
      if(const auto selected = ui::searchable_combo(
          "##Sources", plot_source_id, combo_preview_value, source_entries)) {
        if(*selected == "PLOT_SOURCE.SERIES" &&
           self.available_series_names_.empty()) {
          const auto error_message = std::format(
           "Cannot select '{}' when there are no available series other "
           "than the current one.",
           get_plot_source_method_title(*selected));
          context.alert(error_message);
        } else {
          auto source = get_default_plot_source_method(*selected);

          if(*selected == "PLOT_SOURCE.SERIES") {
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

    ImGui::Indent();
    auto source = plot_method.source();
    self.render_plot_source_method(source, context);
    plot_method.source(std::move(source));
    ImGui::Unindent();

    if constexpr(!std::same_as<TPlotMethod, MomentumHistogramPlotMethod>) {
      ui::field_label("Color");
      const auto color_vec =
       ImGui::ColorConvertU32ToFloat4(plot_method.color());
      auto color =
       std::array{color_vec.x, color_vec.y, color_vec.z, color_vec.w};
      if(ImGui::ColorEdit4("##line_color", color.data())) {
        plot_method.color(ImGui::ColorConvertFloat4ToU32(
         {color[0], color[1], color[2], color[3]}));
      }
    } else {
      const auto render_color = [&](const char* label,
                                    const char* id,
                                    std::uint32_t current_color,
                                    auto set_color) {
        ui::field_label(label);
        const auto color_vec = ImGui::ColorConvertU32ToFloat4(current_color);
        auto color =
         std::array{color_vec.x, color_vec.y, color_vec.z, color_vec.w};
        if(ImGui::ColorEdit4(id, color.data())) {
          set_color(ImGui::ColorConvertFloat4ToU32(
           {color[0], color[1], color[2], color[3]}));
        }
      };

      render_color(
       "Positive rising",
       "##momentum_histogram_positive_rising_color",
       plot_method.positive_rising_color(),
       [&](std::uint32_t color) { plot_method.positive_rising_color(color); });
      render_color(
       "Positive falling",
       "##momentum_histogram_positive_falling_color",
       plot_method.positive_falling_color(),
       [&](std::uint32_t color) { plot_method.positive_falling_color(color); });
      render_color(
       "Negative falling",
       "##momentum_histogram_negative_falling_color",
       plot_method.negative_falling_color(),
       [&](std::uint32_t color) { plot_method.negative_falling_color(color); });
      render_color(
       "Negative rising",
       "##momentum_histogram_negative_rising_color",
       plot_method.negative_rising_color(),
       [&](std::uint32_t color) { plot_method.negative_rising_color(color); });
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

    ui::field_label("Series");
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
    ui::field_label("Value");
    auto value = plot_source_method.value();
    if(ImGui::InputDouble("##constant_value", &value, 0.1, 1.0, "%.2f")) {
      plot_source_method.value(value);
    }
  }

  void render_plot_source_method(
   this auto& self,
   ErasedPlotSourceMethod<ErasedPlotMethodContext>& plot_source_method,
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
    context.push_edit(
     self.selected_model_handle_opt_ ? "Edit Model" : "Add Model",
     [model_handle_opt = self.selected_model_handle_opt_,
      edit_strategy_ptr =
       self.editing_strategy_ptr_](ApplicationState& app_state) {
       if(edit_strategy_ptr->name().empty()) {
         edit_strategy_ptr->name("Unnamed");
       }

       if(!model_handle_opt) {
         app_state.add_model(std::move(*edit_strategy_ptr));
         return;
       }

       app_state.update_model(model_handle_opt.value(), *edit_strategy_ptr);
     });
  }

  void reset(this auto& self)
  {
    self.current_page_ = Page::List;
    self.selected_model_handle_opt_ = std::nullopt;
    self.editing_strategy_ptr_ = nullptr;
    self.editor_baseline_ptr_ = nullptr;
    self.available_series_names_.clear();
    self.changed_series_names_.clear();
  }
};

} // namespace pludux::apps
