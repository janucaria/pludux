module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

export module pludux:nodes.kc_node;

import :methods.kc_method;
import :node_to_erased_method;
import :nodes.erased_node;
import :nodes.ohlcv_node;
import :nodes.value_node;
import :ma_node_type;

export namespace pludux {

enum class KcBandNodeType { Atr, Tr, RangeHighLow };

template<typename TContext = std::monostate>
class KcNode {
public:
  KcNode()
  : KcNode{10, 1.5, 14}
  {
  }

  KcNode(std::size_t period, double multiplier, std::size_t band_atr_period)
  : KcNode{CloseNode{}, period, multiplier, band_atr_period}
  {
  }

  KcNode(std::size_t period,
         double multiplier,
         std::size_t band_atr_period,
         KcBandNodeType band_node_type,
         MaNodeType ma_node_type)
  : KcNode{CloseNode{},
           period,
           multiplier,
           band_atr_period,
           band_node_type,
           ma_node_type}
  {
  }

   KcNode(ErasedNode<TContext> source,
         std::size_t period,
         double multiplier,
         std::size_t band_atr_period,
         KcBandNodeType band_node_type = KcBandNodeType::Atr,
         MaNodeType ma_node_type = MaNodeType::Ema)
  : KcNode{std::move(source),
           ValueNode{static_cast<double>(period)},
           ValueNode{multiplier},
           ValueNode{static_cast<double>(band_atr_period)},
           band_node_type,
           ma_node_type}
  {
  }

   KcNode(ErasedNode<TContext> source,
          ErasedNode<TContext> period,
          ErasedNode<TContext> multiplier,
          ErasedNode<TContext> band_atr_period,
         KcBandNodeType band_node_type = KcBandNodeType::Atr,
         MaNodeType ma_node_type = MaNodeType::Ema)
  : source_{std::move(source)}
  , period_{std::move(period)}
  , multiplier_{std::move(multiplier)}
  , band_atr_period_{std::move(band_atr_period)}
  , band_node_type_{band_node_type}
  , ma_node_type_{ma_node_type}
  {
  }

  auto operator==(const KcNode& other) const noexcept -> bool = default;

   auto source(this const KcNode& self) noexcept
    -> const ErasedNode<TContext>&
  {
    return self.source_;
  }

   void source(this KcNode& self, ErasedNode<TContext> source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto ma_node_type(this const KcNode& self) noexcept -> MaNodeType
  {
    return self.ma_node_type_;
  }

  void ma_node_type(this KcNode& self, MaNodeType ma_node_type) noexcept
  {
    self.ma_node_type_ = ma_node_type;
  }

   auto period(this const KcNode& self) noexcept
    -> const ErasedNode<TContext>&
  {
    return self.period_;
  }

  void period(this KcNode& self, std::size_t period) noexcept
  {
    self.period_ = ValueNode{static_cast<double>(period)};
  }

   void period(this KcNode& self, ErasedNode<TContext> period) noexcept
  {
    self.period_ = std::move(period);
  }

  auto band_node_type(this const KcNode& self) noexcept -> KcBandNodeType
  {
    return self.band_node_type_;
  }

  void band_node_type(this KcNode& self, KcBandNodeType band_node_type) noexcept
  {
    self.band_node_type_ = band_node_type;
  }

   auto band_atr_period(this const KcNode& self) noexcept
    -> const ErasedNode<TContext>&
  {
    return self.band_atr_period_;
  }

  void band_atr_period(this KcNode& self, std::size_t band_atr_period) noexcept
  {
    self.band_atr_period_ = ValueNode{static_cast<double>(band_atr_period)};
  }

  void band_atr_period(
   this KcNode& self,
    ErasedNode<TContext> band_atr_period) noexcept
  {
    self.band_atr_period_ = std::move(band_atr_period);
  }

   auto multiplier(this const KcNode& self) noexcept
    -> const ErasedNode<TContext>&
  {
    return self.multiplier_;
  }

  void multiplier(this KcNode& self, double multiplier) noexcept
  {
    self.multiplier_ = ValueNode{multiplier};
  }

   void multiplier(this KcNode& self, ErasedNode<TContext> multiplier) noexcept
  {
    self.multiplier_ = std::move(multiplier);
  }

private:
   ErasedNode<TContext> source_;
   ErasedNode<TContext> period_;
   ErasedNode<TContext> multiplier_;
   ErasedNode<TContext> band_atr_period_;
  KcBandNodeType band_node_type_;
  MaNodeType ma_node_type_;
};

template<typename TContext>
auto pludux_tag_invoke(NodeToErasedMethod<TContext>,
                        const KcNode<TContext>& node,
                       NodeToErasedMethodContext& context)
 -> ErasedSeriesMethod<TContext>
{
  const auto source_method =
   node_to_erased_method<TContext>(node.source(), context);
  const auto period = node_to_erased_method<TContext>(node.period(), context);
  const auto multiplier =
   node_to_erased_method<TContext>(node.multiplier(), context);
  const auto band_atr_period =
   node_to_erased_method<TContext>(node.band_atr_period(), context);

  return KcMethod{source_method,
                  period,
                  multiplier,
                  band_atr_period,
                  static_cast<KcBandMethodType>(node.band_node_type()),
                  static_cast<MaMethodType>(node.ma_node_type())};
}

} // namespace pludux
