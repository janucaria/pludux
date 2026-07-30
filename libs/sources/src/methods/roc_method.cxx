module;

#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>

export module pludux:methods.roc_method;

import :methods.ohlcv_method;
import :methods.value_method;

export namespace pludux {

template<typename TSourceMethod = CloseMethod,
         typename TPeriodMethod = ValueMethod>
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
  : RocMethod{std::move(source), ValueMethod{static_cast<double>(period)}}
  {
  }

  RocMethod(TSourceMethod source, TPeriodMethod period)
    requires(!std::is_arithmetic_v<TPeriodMethod>)
  : source_{std::move(source)}
  , period_{std::move(period)}
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

  auto period(this const RocMethod& self) noexcept -> const TPeriodMethod&
  {
    return self.period_;
  }

  void period(this RocMethod& self, TPeriodMethod period) noexcept
  {
    self.period_ = std::move(period);
  }

private:
  TSourceMethod source_;
  TPeriodMethod period_;
};

} // namespace pludux
