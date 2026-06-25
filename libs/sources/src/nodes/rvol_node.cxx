module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:nodes.rvol_node;


export namespace pludux {

class RvolNode {
public:
  explicit RvolNode(std::size_t period = 14)
  : period_{period}
  {
  }

  auto operator==(const RvolNode& other) const noexcept -> bool = default;


  auto period(this const RvolNode& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this RvolNode& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  std::size_t period_;
};

} // namespace pludux