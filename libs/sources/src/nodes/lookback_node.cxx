module;

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>

export module pludux:nodes.lookback_node;

import :methods.lookback_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;

export namespace pludux {

template<typename TContext>
class LookbackNode {
public:
  explicit LookbackNode(std::uint32_t period = 1)
  : LookbackNode{CloseNode{}, period}
  {
  }

  LookbackNode(ErasedNode<TContext> source, std::uint32_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  LookbackNode(const LookbackNode& other, std::uint32_t additional_period)
  : LookbackNode{other.source(), other.period()}
  {
    if(additional_period >
       std::numeric_limits<std::uint32_t>::max() - period_) {
      throw std::out_of_range{"Lookback period is out of range"};
    }
    period_ += additional_period;
  }

  auto operator==(const LookbackNode& other) const noexcept -> bool =
   default;

  auto source(this const LookbackNode& self) noexcept
   -> const ErasedNode<TContext>&
  {
    return self.source_;
  }

  void source(this LookbackNode& self, ErasedNode<TContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const LookbackNode& self) noexcept -> std::uint32_t
  {
    return self.period_;
  }

  void period(this LookbackNode& self, std::uint32_t period) noexcept
  {
    self.period_ = period;
  }

private:
  ErasedNode<TContext> source_;
  std::uint32_t period_;
};


template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                       const LookbackNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  return ErasedSeriesMethod<TContext>{LookbackMethod{
   node_to_erased_method<TContext>(node.source(), context), node.period()}};
}

} // namespace pludux
