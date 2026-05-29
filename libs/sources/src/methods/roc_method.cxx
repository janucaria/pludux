module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:methods.roc_method;

import :methods.ohlcv_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod>
class RocMethod {
public:
  RocMethod()
  : RocMethod{14}
  {
  }

  RocMethod(std::size_t period)
  : RocMethod{TSourceMethod{}, period}
  {
  }

  RocMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const RocMethod& other) const noexcept -> bool = default;

  auto source(this const RocMethod& self) noexcept -> TSourceMethod
  {
    return self.source_;
  }

  void source(this RocMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const RocMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this RocMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

} // namespace pludux
