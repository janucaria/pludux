module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.macd_node;

import :methods.macd_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

class MacdNode {
public:
  MacdNode()
  : MacdNode{12, 26, 9}
  {
  }

  MacdNode(std::size_t short_period,
           std::size_t long_period,
           std::size_t signal_period)
  : MacdNode{CloseNode{}, short_period, long_period, signal_period}
  {
  }

  MacdNode(ErasedNode<ErasedSeriesMethodContext> source,
           std::size_t short_period,
           std::size_t long_period,
           std::size_t signal_period)
  : MacdNode{std::move(source),
             ValueNode{static_cast<double>(short_period)},
             ValueNode{static_cast<double>(long_period)},
             ValueNode{static_cast<double>(signal_period)}}
  {
  }

  MacdNode(ErasedNode<ErasedSeriesMethodContext> source,
           ErasedNode<ErasedSeriesMethodContext> fast_period,
           ErasedNode<ErasedSeriesMethodContext> slow_period,
           ErasedNode<ErasedSeriesMethodContext> signal_period)
  : source_{std::move(source)}
  , fast_period_{std::move(fast_period)}
  , slow_period_{std::move(slow_period)}
  , signal_period_{std::move(signal_period)}
  {
  }

  auto operator==(const MacdNode& other) const noexcept -> bool = default;

  auto source(this const MacdNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.source_;
  }

  void source(this MacdNode& self,
              ErasedNode<ErasedSeriesMethodContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto short_period(this const MacdNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.fast_period_;
  }

  void short_period(this MacdNode& self, std::size_t period) noexcept
  {
    self.fast_period_ = ValueNode{static_cast<double>(period)};
  }

  void short_period(this MacdNode& self,
                    ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.fast_period_ = std::move(period);
  }

  auto fast_period(this const MacdNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.short_period();
  }

  void fast_period(this MacdNode& self, std::size_t period) noexcept
  {
    self.short_period(period);
  }

  void fast_period(this MacdNode& self,
                   ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.short_period(std::move(period));
  }

  auto long_period(this const MacdNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.slow_period_;
  }

  void long_period(this MacdNode& self, std::size_t period) noexcept
  {
    self.slow_period_ = ValueNode{static_cast<double>(period)};
  }

  void long_period(this MacdNode& self,
                   ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.slow_period_ = std::move(period);
  }

  auto slow_period(this const MacdNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.long_period();
  }

  void slow_period(this MacdNode& self, std::size_t period) noexcept
  {
    self.long_period(period);
  }

  void slow_period(this MacdNode& self,
                   ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.long_period(std::move(period));
  }

  auto signal_period(this const MacdNode& self) noexcept
   -> const ErasedNode<ErasedSeriesMethodContext>&
  {
    return self.signal_period_;
  }

  void signal_period(this MacdNode& self, std::size_t period) noexcept
  {
    self.signal_period_ = ValueNode{static_cast<double>(period)};
  }

  void signal_period(this MacdNode& self,
                     ErasedNode<ErasedSeriesMethodContext> period) noexcept
  {
    self.signal_period_ = std::move(period);
  }

private:
  ErasedNode<ErasedSeriesMethodContext> source_;
  ErasedNode<ErasedSeriesMethodContext> fast_period_;
  ErasedNode<ErasedSeriesMethodContext> slow_period_;
  ErasedNode<ErasedSeriesMethodContext> signal_period_;
};

template<MethodContextable TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const MacdNode& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto fast_period =
   node_to_erased_method<TContext>(node.fast_period(), context);
  const auto slow_period =
   node_to_erased_method<TContext>(node.slow_period(), context);
  const auto signal_period =
   node_to_erased_method<TContext>(node.signal_period(), context);

  return MacdMethod{source_method, fast_period, slow_period, signal_period};
}

} // namespace pludux
