module;

#include <cassert>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:nodes.atr_node;

import :nodes.adaptive_ma_node;

export namespace pludux {

class AtrNode {
public:
  AtrNode()
  : AtrNode{14}
  {
  }

  explicit AtrNode(std::size_t period,
                     MaNodeType ma_smoothing_type = MaNodeType::Rma)
  : period_{period}
  , ma_smoothing_type_{ma_smoothing_type}
  {
  }

  auto operator==(const AtrNode& other) const noexcept -> bool = default;


  auto period(this AtrNode self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this AtrNode& self, std::size_t new_period) noexcept
  {
    self.period_ = new_period;
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
  std::size_t period_;
  MaNodeType ma_smoothing_type_;
};

} // namespace pludux