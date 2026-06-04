module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:methods.lookback_method;

export namespace pludux {

template<typename TSourceMethod>
class LookbackMethod {
public:
  explicit LookbackMethod(std::size_t period = 1)
  : LookbackMethod{TSourceMethod{}, period}
  {
  }

  LookbackMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  template<typename UMethod>
  LookbackMethod(const LookbackMethod<UMethod>& other,
                 std::size_t additional_period)
  : LookbackMethod{other.source(), other.period() + additional_period}
  {
  }

  auto operator==(const LookbackMethod& other) const noexcept -> bool = default;

  auto source(this const LookbackMethod& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this LookbackMethod& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const LookbackMethod& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this LookbackMethod& self, std::size_t period) noexcept
  {
    self.period_ = period;
  }

private:
  TSourceMethod source_;
  std::size_t period_;
};

// Deduction guide to deduce TSourceMethod from UMethod in the copy constructor
template<typename UMethod>
LookbackMethod(const LookbackMethod<UMethod>&, std::size_t)
 -> LookbackMethod<UMethod>;

} // namespace pludux