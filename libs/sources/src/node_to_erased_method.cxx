module;

#include <cstddef>
#include <span>

export module pludux:node_to_erased_method;

export import :methods.any_series_method;

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

struct NodeToErasedMethod {
  template<typename TNode>
  auto operator()(
   this NodeToErasedMethod self,
   const TNode& node,
   NodeToErasedMethodContext&
    context) noexcept(noexcept(pludux_tag_invoke(self, node, context)))
   -> AnySeriesMethod
  {
    return pludux_tag_invoke(self, node, context);
  }
};

inline constexpr auto node_to_erased_method = NodeToErasedMethod{};

} // namespace pludux
