module;

#include <concepts>
#include <cstddef>
#include <span>
#include <type_traits>

export module pludux:node_to_erased_method;

export import :methods.erased_series_method;

export namespace pludux {

class NodeToErasedMethodContext {
public:
  NodeToErasedMethodContext() = default;

  explicit NodeToErasedMethodContext(std::span<const double> inputs)
  : inputs_{inputs}
  {
  }

  auto consume(this NodeToErasedMethodContext& self, double fallback) noexcept
   -> double
  {
    if(self.input_index_ < self.inputs_.size()) {
      return self.inputs_[self.input_index_++];
    }
    ++self.input_index_;
    return fallback;
  }

  auto input_index(this const NodeToErasedMethodContext& self) noexcept
   -> std::size_t
  {
    return self.input_index_;
  }

private:
  std::span<const double> inputs_{};
  std::size_t input_index_{0};
};

void pludux_tag_invoke();

template<typename TContext, typename TNode>
consteval auto node_context_admissible() -> bool
{
  if constexpr(requires {
                 {
                   TContext::template node_admissible<
                    std::remove_cvref_t<TNode>>()
                 } -> std::same_as<bool>;
               }) {
    return TContext::template node_admissible<std::remove_cvref_t<TNode>>();
  }
  return true;
}

template<typename TContext>
struct NodeToErasedMethod {
  template<typename TNode>
    requires(node_context_admissible<TContext, TNode>()) &&
            requires(NodeToErasedMethod self,
                     const TNode& node,
                     NodeToErasedMethodContext& context) {
              {
                pludux_tag_invoke(self, node, context)
              } -> std::same_as<ErasedSeriesMethod<TContext>>;
            }
  auto operator()(
   this NodeToErasedMethod self,
   const TNode& node,
   NodeToErasedMethodContext& context)
   -> ErasedSeriesMethod<TContext>
  {
    return pludux_tag_invoke(self, node, context);
  }
};

template<typename TContext>
inline constexpr auto node_to_erased_method = NodeToErasedMethod<TContext>{};

} // namespace pludux
