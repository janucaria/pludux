module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:methods.rma_method;

import :methods.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
class RmaMethod {
public:
  RmaMethod()
  : RmaMethod{20}
  {
  }

  explicit RmaMethod(std::size_t period)
  : RmaMethod{TSourceMethod{}, period}
  {
  }

  explicit RmaMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const RmaMethod& other) const noexcept -> bool = default;

  auto source(this const RmaMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this RmaMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RmaMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this RmaMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux
