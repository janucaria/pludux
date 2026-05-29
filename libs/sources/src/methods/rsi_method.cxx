module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.rsi_method;

import :methods.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
class RsiMethod {
public:
  RsiMethod()
  : RsiMethod{14}
  {
  }

  explicit RsiMethod(std::size_t period)
  : RsiMethod{TSourceMethod{}, period}
  {
  }

  explicit RsiMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const RsiMethod& other) const noexcept -> bool = default;

  auto source(this const RsiMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this RsiMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RsiMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this RsiMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

RsiMethod() -> RsiMethod<CloseMethod>;
RsiMethod(std::size_t) -> RsiMethod<CloseMethod>;

} // namespace pludux
