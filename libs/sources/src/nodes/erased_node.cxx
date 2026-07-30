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

import :node_to_erased_method;

export namespace pludux {

class ErasedNode {
public:
  template<typename UNode>
    requires(!std::same_as<std::remove_cvref_t<UNode>, ErasedNode>) &&
             (!std::same_as<std::remove_cvref_t<UNode>,
                            std::vector<ErasedNode>>) &&
             (!std::convertible_to<UNode, double>) &&
             std::equality_comparable<UNode>
  ErasedNode(UNode impl)
  : impl_{std::make_any<UNode>(std::move(impl))}
  , convert_to_erased_method_{[](const std::any& impl,
                                 NodeToErasedMethodContext& context)
                               -> AnySeriesMethod {
    const auto& node = *std::any_cast<UNode>(&impl);
    return node_to_erased_method(node, context);
  }}
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

  friend auto pludux_tag_invoke(NodeToErasedMethod,
                                const ErasedNode& node,
                                NodeToErasedMethodContext& context)
   -> AnySeriesMethod;

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

  std::function<
   auto(const std::any&, NodeToErasedMethodContext&)->AnySeriesMethod>
   convert_to_erased_method_;

  std::function<auto(const std::any&, const ErasedNode&)->bool> equals_;

  std::function<auto(const std::any&, const ErasedNode&)->bool> not_equals_;
};

auto pludux_tag_invoke(NodeToErasedMethod,
                       const ErasedNode& node,
                       NodeToErasedMethodContext& context) -> AnySeriesMethod
{
  return node.convert_to_erased_method_(node.impl_, context);
}

} // namespace pludux
