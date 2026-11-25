module;

#include <cmath>
#include <cstddef>
#include <functional>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

export module pludux.backtest:call_node;

import pludux;
import :backtest_method_context;

export namespace pludux::backtest {

class CallNode {
public:
  using Representation = NumericInputNode::ValueRepresentation;

  explicit CallNode(std::string field = {},
                    Representation representation = Representation::Decimal)
  : field_{std::move(field)}
  , representation_{representation}
  {
  }

  auto operator==(const CallNode&) const -> bool = default;
  auto field() const noexcept -> const std::string&
  {
    return field_;
  }
  void field(std::string value)
  {
    field_ = std::move(value);
  }
  auto representation() const noexcept -> Representation
  {
    return representation_;
  }
  void representation(Representation value) noexcept
  {
    representation_ = value;
  }

private:
  std::string field_;
  Representation representation_;
};

inline auto hash_series_method(const CallNode& node) noexcept -> std::size_t
{
  return std::hash<std::string_view>{}("pludux.backtest.CallNode") ^
         std::hash<std::string>{}(node.field()) ^
         static_cast<std::size_t>(node.representation());
}

inline auto pludux_tag_invoke(EvaluateSeriesMethod,
                              const CallNode& node,
                              AssetSnapshot snapshot,
                              BacktestMethodContext context) noexcept -> double
{
  const auto raw = context.call_value(node.field());
  return std::isfinite(raw)
          ? resolve_numeric_input_value(raw, node.representation())
          : raw;
}

inline auto pludux_tag_invoke(NodeToErasedMethod<BacktestMethodContext>,
                              const CallNode& node,
                              NodeToErasedMethodContext&) -> StrategyMethod
{
  return StrategyMethod{node};
}

} // namespace pludux::backtest
