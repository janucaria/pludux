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
  : KcNode{std::move(source),
           ErasedNode{period},
           ErasedNode{multiplier},
           ErasedNode{band_atr_period},
           band_node_type,
           ma_node_type}
  {
  }

  KcNode(ErasedNode source,
         ErasedNode period,
         ErasedNode multiplier,
         ErasedNode band_atr_period,
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

  auto period(this const KcNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this KcNode& self, std::size_t period) noexcept
  {
    self.period_ = ErasedNode{period};
  }

  void period(this KcNode& self, ErasedNode period) noexcept
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

  auto band_atr_period(this const KcNode& self) noexcept -> const ErasedNode&
  {
    return self.band_atr_period_;
  }

  void band_atr_period(this KcNode& self, std::size_t band_atr_period) noexcept
  {
    self.band_atr_period_ = ErasedNode{band_atr_period};
  }

  void band_atr_period(this KcNode& self, ErasedNode band_atr_period) noexcept
  {
    self.band_atr_period_ = std::move(band_atr_period);
  }

  auto multiplier(this const KcNode& self) noexcept -> const ErasedNode&
  {
    return self.multiplier_;
  }

  void multiplier(this KcNode& self, double multiplier) noexcept
  {
    self.multiplier_ = ErasedNode{multiplier};
  }

  void multiplier(this KcNode& self, ErasedNode multiplier) noexcept
  {
    self.multiplier_ = std::move(multiplier);
  }

private:
  ErasedNode source_;
  ErasedNode period_;
  ErasedNode multiplier_;
  ErasedNode band_atr_period_;
  KcBandNodeType band_node_type_;
  MaNodeType ma_node_type_;
};

} // namespace pludux
