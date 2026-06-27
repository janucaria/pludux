module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:nodes.atr_node;

import :nodes.adaptive_ma_node;
import :nodes.erased_node;

export namespace pludux {

class AtrNode {
public:
  AtrNode()
  : AtrNode{14}
  {
  }

  explicit AtrNode(std::size_t period,
                   MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : AtrNode{ErasedNode{period}, ma_smoothing_type}
  {
  }

  explicit AtrNode(ErasedNode period,
                   MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : period_{std::move(period)}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const AtrNode& other) const noexcept -> bool = default;

  auto period(this const AtrNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this AtrNode& self, std::size_t new_period) noexcept
  {
    self.period_ = ErasedNode{new_period};
  }

  void period(this AtrNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  auto ma_smoothing_type(this const AtrNode& self) noexcept -> MaNodeType
  {
    return self.ma_smoothing_type_;
  }

  void ma_smoothing_type(this AtrNode& self, MaNodeType new_type) noexcept
  {
    self.ma_smoothing_type_ = new_type;
  }

private:
  ErasedNode period_;
  MaNodeType ma_smoothing_type_;
};

} // namespace pludux
