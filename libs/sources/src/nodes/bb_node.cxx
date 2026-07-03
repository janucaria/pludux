module;

#include <cstddef>
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

export module pludux:nodes.bb_node;

import :nodes.erased_node;
import :ma_node_type;

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
  : BbNode{
     std::move(source), ErasedNode{period}, ErasedNode{stddev}, ma_node_type}
  {
  }

  BbNode(ErasedNode source,
         ErasedNode period,
         ErasedNode stddev,
         MaNodeType ma_node_type = MaNodeType::Sma)
  : source_{std::move(source)}
  , period_{std::move(period)}
  , stddev_{std::move(stddev)}
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

  auto period(this const BbNode& self) noexcept -> const ErasedNode&
  {
    return self.period_;
  }

  void period(this BbNode& self, std::size_t new_period) noexcept
  {
    self.period_ = ErasedNode{new_period};
  }

  void period(this BbNode& self, ErasedNode period) noexcept
  {
    self.period_ = std::move(period);
  }

  auto stddev(this const BbNode& self) noexcept -> const ErasedNode&
  {
    return self.stddev_;
  }

  void stddev(this BbNode& self, double new_stddev) noexcept
  {
    self.stddev_ = ErasedNode{new_stddev};
  }

  void stddev(this BbNode& self, ErasedNode stddev) noexcept
  {
    self.stddev_ = std::move(stddev);
  }

private:
  ErasedNode source_;
  ErasedNode period_;
  ErasedNode stddev_;
  MaNodeType ma_node_type_;
};

} // namespace pludux
