module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:methods.hma_method;

import :methods.ohlcv_method;
import :methods.value_method;
import :methods.wma_method;
import :methods.operators_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
class HmaMethod {
public:
  HmaMethod()
  : HmaMethod{20}
  {
  }

  explicit HmaMethod(std::size_t period)
  : HmaMethod{TSourceMethod{}, period}
  {
  }

  explicit HmaMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const HmaMethod& other) const noexcept -> bool = default;

  auto source(this const HmaMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this HmaMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const HmaMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this HmaMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux
