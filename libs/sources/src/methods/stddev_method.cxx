module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:methods.stddev_method;

import :methods.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
class StddevMethod {
public:
  StddevMethod()
  : StddevMethod{20}
  {
  }

  explicit StddevMethod(std::size_t period)
  : StddevMethod{TSourceMethod{}, period}
  {
  }

  explicit StddevMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const StddevMethod& other) const noexcept -> bool = default;

  auto source(this const StddevMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this StddevMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const StddevMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this StddevMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux
