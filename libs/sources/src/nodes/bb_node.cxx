module;

#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

export module pludux:nodes.bb_node;

import :methods.bb_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;
import :ma_node_type;

export namespace pludux {

class BbNode {
public:
  BbNode()
  : BbNode{20, 1.5}
  {
  }

  BbNode(std::size_t period, double stddev)
  : BbNode{CloseNode{}, period, stddev}
  {
  }

  BbNode(ErasedNode<ErasedSeriesMethodContext> source,
         std::size_t period,
         double stddev,
         MaNodeType ma_node_type = MaNodeType::Sma)
  : BbNode{std::move(source),
           ValueNode{static_cast<double>(period)},
           ValueNode{stddev},
           ma_node_type}
  {
  }

  BbNode(ErasedNode<ErasedSeriesMethodContext> source,
         ErasedNode<ErasedSeriesMethodContext> period,
         ErasedNode<ErasedSeriesMethodContext> stddev,
         MaNodeType ma_node_type = MaNodeType::Sma)
  : source_{std::move(source)}
  , period_{std::move(period)}
  , stddev_{std::move(stddev)}
  , ma_node_type_{ma_node_type}
  {
  }

  auto operator==(const BbNode& other) const noexcept -> bool = default;

  auto source(this const BbNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.source_;
  }

  void source(this BbNode& self,
              ErasedNode<ErasedSeriesMethodContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto ma_node_type(this const BbNode& self) noexcept -> MaNodeType
  {
    return self.ma_node_type_;
  }

  auto ma_node_type(this BbNode& self, MaNodeType ma_node_type) noexcept
  {
    self.ma_node_type_ = ma_node_type;
  }

  auto period(this const BbNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.period_;
  }

  void period(this BbNode& self, std::size_t new_period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(new_period)};
  }

  void period(this BbNode& self,
              ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  auto stddev(this const BbNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.stddev_;
  }

  void stddev(this BbNode& self, double new_stddev) noexcept
  {
    self.stddev_ = ValueNode{new_stddev};
  }

  void stddev(this BbNode& self,
              ErasedNode<ErasedSeriesMethodContext> stddev) noexcept
  {
    self.stddev_ = std::move(stddev);
  }

private:
  ErasedNode<ErasedSeriesMethodContext> source_;
  ErasedNode<ErasedSeriesMethodContext> period_;
  ErasedNode<ErasedSeriesMethodContext> stddev_;
  MaNodeType ma_node_type_;
};

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const BbNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);
  const auto stddev = node_to_erased_method<TContext>(node.stddev(), context);

  return BbMethod{source_method,
                  period,
                  stddev,
                  static_cast<MaMethodType>(node.ma_node_type())};
}

} // namespace pludux
