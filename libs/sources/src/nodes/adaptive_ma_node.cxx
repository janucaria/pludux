module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:nodes.adaptive_ma_node;

import :nodes.erased_node;

export namespace pludux {

enum class MaNodeType { Sma, Ema, Wma, Rma, Hma };

class AdaptiveMaNode {
public:
  AdaptiveMaNode()
  : AdaptiveMaNode{20}
  {
  }

  explicit AdaptiveMaNode(std::size_t period)
  : AdaptiveMaNode{ErasedNode{}, period}
  {
  }

  AdaptiveMaNode(ErasedNode source, std::size_t period)
  : AdaptiveMaNode{std::move(source), MaNodeType::Sma, period}
  {
  }

  AdaptiveMaNode(ErasedNode source,
                   MaNodeType ma_type,
                   std::size_t period)
  : source_{std::move(source)}
  , ma_type_{ma_type}
  , period_{period}
  {
  }

  auto operator==(const AdaptiveMaNode& other) const noexcept
   -> bool = default;


  auto ma_type(this const AdaptiveMaNode& self) noexcept -> MaNodeType
  {
    return self.ma_type_;
  }

  void ma_type(this AdaptiveMaNode& self, MaNodeType ma_type) noexcept
  {
    self.ma_type_ = ma_type;
  }

  auto source(this const AdaptiveMaNode& self) noexcept
   -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this AdaptiveMaNode& self, ErasedNode source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const AdaptiveMaNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this AdaptiveMaNode& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  ErasedNode source_;
  MaNodeType ma_type_;
  std::size_t period_;
};

} // namespace pludux