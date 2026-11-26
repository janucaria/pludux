module;

#include <cstddef>
#include <functional>
#include <string>
#include <utility>

export module pludux.backtest:extern_field_node;

import pludux;
import :backtest_method_context;

export namespace pludux::backtest {

class ExternFieldNode {
public:
  ExternFieldNode(std::string reference, std::string field)
  : reference_{std::move(reference)}
  , field_{std::move(field)}
  {
  }

  auto operator==(const ExternFieldNode&) const -> bool = default;

  auto reference(this auto& self) noexcept -> auto&
  {
    return self.reference_;
  }

  auto field(this auto& self) noexcept -> auto&
  {
    return self.field_;
  }

private:
  std::string reference_;
  std::string field_;
};

inline auto hash_series_method(const ExternFieldNode& node) noexcept
 -> std::size_t
{
  const auto reference = std::hash<std::string>{}(node.reference());
  const auto field = std::hash<std::string>{}(node.field());
  return reference ^
         (field + 0x9e3779b9U + (reference << 6) + (reference >> 2));
}

inline auto pludux_tag_invoke(EvaluateSeriesMethod,
                              const ExternFieldNode& node,
                              AssetSnapshot snapshot,
                              BacktestMethodContext context) noexcept -> double
{
  return context.external_value(node.reference(), node.field(), snapshot);
}

inline auto pludux_tag_invoke(NodeToErasedMethod<BacktestMethodContext>,
                              const ExternFieldNode& node,
                              NodeToErasedMethodContext&) -> StrategyMethod
{
  return StrategyMethod{node};
}

} // namespace pludux::backtest
