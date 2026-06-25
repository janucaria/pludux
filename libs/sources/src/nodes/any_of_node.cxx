module;

#include <initializer_list>
#include <utility>
#include <vector>

export module pludux:nodes.any_of_node;

import :nodes.erased_node;

export namespace pludux {

class AnyOfNode {
public:
  AnyOfNode(std::initializer_list<ErasedNode> conditions)
  : conditions_{conditions}
  {
  }

  explicit AnyOfNode(std::vector<ErasedNode> conditions)
  : conditions_{std::move(conditions)}
  {
  }

  auto operator==(const AnyOfNode& other) const noexcept -> bool = default;


  auto conditions(this const AnyOfNode& self) noexcept
   -> const std::vector<ErasedNode>&
  {
    return self.conditions_;
  }

  void conditions(this AnyOfNode& self,
                  std::vector<ErasedNode> conditions) noexcept
  {
    self.conditions_ = std::move(conditions);
  }

private:
  std::vector<ErasedNode> conditions_;
};

} // namespace pludux