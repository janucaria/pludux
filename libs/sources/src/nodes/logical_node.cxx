module;

#include <functional>
#include <utility>

export module pludux:nodes.logical_node;

import :methods.logical_method;
import :nodes.erased_node;

export namespace pludux {

template<typename TOperator>
class BinaryLogicalNode {
public:
  BinaryLogicalNode(ErasedNode first_condition,
                    ErasedNode second_condition)
  : first_condition_{std::move(first_condition)}
  , second_condition_{std::move(second_condition)}
  {
  }

  auto operator==(const BinaryLogicalNode& other) const noexcept
   -> bool = default;


  auto first_condition(this const BinaryLogicalNode& self)
   -> const ErasedNode&
  {
    return self.first_condition_;
  }

  void first_condition(this BinaryLogicalNode& self, ErasedNode condition)
  {
    self.first_condition_ = std::move(condition);
  }

  auto second_condition(this const BinaryLogicalNode& self)
   -> const ErasedNode&
  {
    return self.second_condition_;
  }

  void second_condition(this BinaryLogicalNode& self,
                        ErasedNode condition)
  {
    self.second_condition_ = std::move(condition);
  }

private:
  ErasedNode first_condition_;
  ErasedNode second_condition_;
};

template<typename TOperator>
class UnaryLogicalNode {
public:
  explicit UnaryLogicalNode(ErasedNode condition)
  : other_condition_{std::move(condition)}
  {
  }

  auto operator==(const UnaryLogicalNode& other) const noexcept
   -> bool = default;


  auto other_condition(this const UnaryLogicalNode& self)
   -> const ErasedNode&
  {
    return self.other_condition_;
  }

  void other_condition(this UnaryLogicalNode& self, ErasedNode condition)
  {
    self.other_condition_ = std::move(condition);
  }

private:
  ErasedNode other_condition_;
};

using LogicalAndNode =
 BinaryLogicalNode<std::logical_and<>>;

using LogicalOrNode =
 BinaryLogicalNode<std::logical_or<>>;

using LogicalNotNode = UnaryLogicalNode<std::logical_not<>>;

using LogicalXorNode =
 BinaryLogicalNode<LogicalXor<>>;

} // namespace pludux
