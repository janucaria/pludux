module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

export module pludux:nodes.kc_node;

import :nodes.erased_node;
import :nodes.adaptive_ma_node;

export namespace pludux {

enum class KcBandNodeType { Atr, Tr, RangeHighLow };

class KcNode {
public:
  KcNode()
  : KcNode{10, 1.5, 14}
  {
  }

  KcNode(std::size_t period, double multiplier, std::size_t band_atr_period)
  : KcNode{ErasedNode{}, period, multiplier, band_atr_period}
  {
  }

  KcNode(std::size_t period,
           double multiplier,
           std::size_t band_atr_period,
           KcBandNodeType band_node_type,
           MaNodeType ma_node_type)
  : KcNode{ErasedNode{},
             period,
             multiplier,
             band_atr_period,
             band_node_type,
             ma_node_type}
  {
  }

  KcNode(ErasedNode source,
           std::size_t period,
           double multiplier,
           std::size_t band_atr_period,
           KcBandNodeType band_node_type = KcBandNodeType::Atr,
           MaNodeType ma_node_type = MaNodeType::Ema)
  : source_{std::move(source)}
  , period_{period}
  , multiplier_{multiplier}
  , band_atr_period_{band_atr_period}
  , band_node_type_{band_node_type}
  , ma_node_type_{ma_node_type}
  {
  }

  auto operator==(const KcNode& other) const noexcept -> bool = default;


  auto source(this const KcNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this KcNode& self, ErasedNode source) noexcept
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

  auto period(this const KcNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this KcNode& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

  auto band_node_type(this const KcNode& self) noexcept -> KcBandNodeType
  {
    return self.band_node_type_;
  }

  void band_node_type(this KcNode& self,
                        KcBandNodeType band_node_type) noexcept
  {
    self.band_node_type_ = band_node_type;
  }

  auto band_atr_period(this const KcNode& self) noexcept -> std::size_t
  {
    return self.band_atr_period_;
  }

  void band_atr_period(this KcNode& self,
                       std::size_t band_atr_period) noexcept
  {
    self.band_atr_period_ = band_atr_period;
  }

  auto multiplier(this const KcNode& self) noexcept -> double
  {
    return self.multiplier_;
  }

  void multiplier(this KcNode& self, double multiplier) noexcept
  {
    self.multiplier_ = multiplier;
  }

private:
  ErasedNode source_;
  std::size_t period_;
  double multiplier_;

  std::size_t band_atr_period_;
  KcBandNodeType band_node_type_;
  MaNodeType ma_node_type_;
};

} // namespace pludux