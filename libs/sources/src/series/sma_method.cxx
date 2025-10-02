module;

#include <cstddef>
#include <limits>
#include <utility>

export module pludux:series.sma_method;

import :asset_snapshot;
import :series.method_call_context;
import :series.method_output;

import :series.ohlcv_method;

export namespace pludux::series {

template<typename TSourceMethod = CloseMethod>
class SmaMethod {
public:
  using ResultType = typename TSourceMethod::ResultType;

  SmaMethod()
  : SmaMethod{20}
  {
  }

  explicit SmaMethod(std::size_t period)
  : SmaMethod{TSourceMethod{}, period}
  {
  }

  explicit SmaMethod(TSourceMethod source, std::size_t period)
  : source_{std::move(source)}
  , period_{period}
  {
  }

  auto operator==(const SmaMethod& other) const noexcept -> bool = default;

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
  {
    const auto asset_size = asset_snapshot.size();
    if(asset_size < self.period_) {
      return std::numeric_limits<ResultType>::quiet_NaN();
    }

    auto sum = ResultType{0};
    for(auto i = 0uz; i < self.period_; ++i) {
      sum += self.source_(asset_snapshot[i], context);
    }

    return sum / self.period_;
  }

  auto operator()(this const auto& self,
                  AssetSnapshot asset_snapshot,
                  MethodOutput output,
                  MethodCallContext<ResultType> auto context) noexcept
   -> ResultType
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

} // namespace pludux::series
