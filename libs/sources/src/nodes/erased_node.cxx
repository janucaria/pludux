module;

#include <any>
#include <concepts>
#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

export module pludux:nodes.erased_node;

import :nodes.ohlcv_node;
import :nodes.value_node;

export namespace pludux {

class ErasedNode {
public:
  ErasedNode()
  : ErasedNode{CloseNode{}}
  {
  }

  ErasedNode(std::size_t value)
  : ErasedNode{static_cast<double>(value)}
  {
  }

  ErasedNode(double value)
  : ErasedNode{ValueNode{value}}
  {
  }

  template<typename UNode>
    requires(!std::same_as<std::remove_cvref_t<UNode>, ErasedNode>) &&
             (!std::same_as<std::remove_cvref_t<UNode>,
                            std::vector<ErasedNode>>) &&
             requires(UNode node) {
               { node == node } -> std::convertible_to<bool>;
               { node != node } -> std::convertible_to<bool>;
             }
  ErasedNode(UNode impl = UNode{})
  : impl_{std::make_any<UNode>(std::move(impl))}
  , equals_{[](const std::any& impl, const ErasedNode& other) static -> bool {
    if(auto other_node = std::any_cast<UNode>(&other.impl_)) {
      const auto& node = *std::any_cast<UNode>(&impl);
      return node == *other_node;
    }
    return false;
  }}
  , not_equals_{
     [](const std::any& impl, const ErasedNode& other) static -> bool {
       if(auto other_node = std::any_cast<UNode>(&other.impl_)) {
         const auto& node = *std::any_cast<UNode>(&impl);
         return node != *other_node;
       }
       return true;
     }}
  {
  }

  auto operator==(this const ErasedNode& self, const ErasedNode& other) noexcept
   -> bool
  {
    return self.equals_(self.impl_, other);
  }

  auto operator!=(this const ErasedNode& self, const ErasedNode& other) noexcept
   -> bool
  {
    return self.not_equals_(self.impl_, other);
  }

  template<typename UNode>
  friend auto node_cast(const ErasedNode& node) noexcept -> const UNode*
  {
    return std::any_cast<const UNode>(&node.impl_);
  }

  template<typename UNode>
  friend auto node_cast(ErasedNode& node) noexcept -> UNode*
  {
    return std::any_cast<UNode>(&node.impl_);
  }

private:
  std::any impl_;

  std::function<auto(const std::any&, const ErasedNode&)->bool> equals_;

  std::function<auto(const std::any&, const ErasedNode&)->bool> not_equals_;
};

} // namespace pludux
