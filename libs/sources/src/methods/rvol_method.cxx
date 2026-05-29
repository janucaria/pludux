module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.rvol_method;

export namespace pludux {

class RvolMethod {
public:
  explicit RvolMethod(std::size_t period = 14)
  : period_{period}
  {
  }

  auto operator==(const RvolMethod& other) const noexcept -> bool = default;

  auto period(this const RvolMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this RvolMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  std::size_t period_;
};

} // namespace pludux
