module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.lookback_method;

import :asset_snapshot;
import :method_contextable;
import :series_output;

export namespace pludux {

template<typename TSourceMethod>
  requires requires { typename TSourceMethod::ResultType; }
class LookbackMethod {
public:
  using ResultType = TSourceMethod::ResultType;

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
    requires requires { typename UMethod::ResultType; }
  LookbackMethod(const LookbackMethod<UMethod>& other,
                 std::size_t additional_period)
  : LookbackMethod{other.source(), other.period() + additional_period}
  {
  }

  auto operator==(const LookbackMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return self.source_(asset_snapshot[self.period_], context);
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  SeriesOutput output,
                  MethodContextable auto context) noexcept -> ResultType
  {
    return std::numeric_limits<ResultType>::quiet_NaN();
  }

  auto source(this const auto& self) noexcept -> const TSourceMethod&
  {
    return self.source_;
  }

  void source(this auto& self, TSourceMethod source) noexcept
  {
    self.source_ = std::move(source);
  }

  auto period(this const auto& self) noexcept -> std::size_t
  {
    return self.period_;
  }

  void period(this auto& self, std::size_t period) noexcept
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