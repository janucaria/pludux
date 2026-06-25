module;

#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

export module pludux:nodes.bb_node;

import :nodes.erased_node;
import :nodes.adaptive_ma_node;

export namespace pludux {

class BbNode {
public:
  BbNode()
  : BbNode{20, 1.5}
  {
  }

  BbNode(std::size_t period, double stddev)
  : BbNode{ErasedNode{}, period, stddev}
  {
  }

  BbNode(ErasedNode source,
           std::size_t period,
           double stddev,
           MaNodeType ma_node_type = MaNodeType::Sma)
  : source_{std::move(source)}
  , period_{period}
  , stddev_{stddev}
  , ma_node_type_{ma_node_type}
  {
  }

  auto operator==(const BbNode& other) const noexcept -> bool = default;


  auto source(this const BbNode& self) noexcept -> const ErasedNode&
  {
    return self.source_;
  }

  void source(this BbNode& self, ErasedNode source) noexcept
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

  auto period(this const BbNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this BbNode& self, std::size_t new_period) noexcept
  {
    self.period_ = new_period;
  }

  auto stddev(this const BbNode& self) noexcept -> double
  {
    return self.stddev_;
  }

  void stddev(this BbNode& self, double new_stddev) noexcept
  {
    self.stddev_ = new_stddev;
  }

private:
  ErasedNode source_;
  std::size_t period_;
  double stddev_;
  MaNodeType ma_node_type_;
};

} // namespace pludux